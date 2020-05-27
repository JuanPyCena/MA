///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Christian Muschick, <c.muschick@avibit.com>
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Data framing with arbitrary delimiter strings
*/



#include "avmisc.h"

#include "avdataframingdelimiters.h"

///////////////////////////////////////////////////////////////////////////////

AVDataFramingDelimiters::AVDataFramingDelimiters(const QByteArray& start_delimiter,
                                                 const QByteArray& end_delimiter) :
    m_receive_ba         (), // construct before buffer
    m_receive_buffer     (&m_receive_ba),
    m_buffer_region_start(0),
    m_start_delimiter    (start_delimiter)
{
    m_end_delimiter.push_back(end_delimiter);
    AVASSERT(!end_delimiter.isEmpty());
    AVASSERT(m_receive_buffer.open(QIODevice::WriteOnly));
}

///////////////////////////////////////////////////////////////////////////////

AVDataFramingDelimiters::~AVDataFramingDelimiters()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingDelimiters::reset()
{
    m_receive_buffer.seek(0);
    m_buffer_region_start = 0;
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingDelimiters::setDelimiters(
        const QByteArray& start_delimiter, const QByteArray& end_delimiter)
{
    m_start_delimiter = start_delimiter;
    m_end_delimiter.clear();
    m_end_delimiter.push_back(end_delimiter);
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingDelimiters::addOptionalEndDelimiter(const QByteArray& end_delimiter)
{
    m_end_delimiter.push_back(end_delimiter);
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingDelimiters::slotGotData(const QByteArray& data)
{
    AVASSERT(m_receive_buffer.write(data.data(), data.size()) != -1);
//    AVLogger->Write(LOG_INFO, "AVDataFramingDelimiters::slotGotData with length %d", data.size());
//    AVLogger->Write(LOG_INFO, "%s", AVHexDump((const uchar*)data.data(),
//                                              data.size()).toLocal8Bit().constData());
//    AVLogger->Write(LOG_INFO, "region start %i, at %i", m_buffer_region_start,
//                    (uint)m_receive_buffer.at());

    // Loop until no more complete messages can be found in valid buffer region
    uint region_size;
    int end_index;
    int delimiter_end_index;
    do
    {
        skipJunk(end_index, delimiter_end_index);
//        AVLogger->Write(LOG_INFO, "AVDataFramingDelimiters::slotGotData: "
//                "end index is %d", end_index);

        // Handle complete message if end delimiter was found
        if (end_index != -1)
        {
            AVASSERT(memcmp(m_receive_ba.data()+m_buffer_region_start,
                            m_start_delimiter.data(),
                            m_start_delimiter.size()) == 0);
            // skip start delimiter
            m_buffer_region_start += m_start_delimiter.size();
            const char* msg_data = m_receive_ba.data() + m_buffer_region_start;
            uint msg_length      = end_index - m_buffer_region_start;
//            AVLogger->Write(LOG_INFO, "AVDataFramingDelimiters::slotGotData: message \n%s",
//                            AVHexDump((const uchar*)msg_data, msg_length)
//                                .toLocal8Bit().constData());

            // Continue searching after the end delimiter.
            // Set this before emitting the signal, because that can call the reset() function!
            m_buffer_region_start = delimiter_end_index;

            // Emit our message
            QByteArray msg_array;
            msg_array.setRawData(msg_data, msg_length);
            emit signalGotMessage(msg_array);
            msg_array.clear();
        }
        // If there is no more data in the buffer, reset position to 0; else continue checking
        // for messages after this one
        region_size = m_receive_buffer.pos() - m_buffer_region_start;

//        AVLogger->Write(LOG_INFO, "region size is %i", region_size);
    } while (region_size != 0 && end_index != -1);

    // "Housekeep" the buffer
    if (region_size == 0)
    {
        // reset to position 0 if empty
        m_receive_buffer.seek(0);
        m_buffer_region_start = 0;
    } else if ( region_size <= m_buffer_region_start)
    {
        // Copy remaining (partially received) message to start of receive buffer if copy can be
        // performed without overlap -> keep buffer size low
        memcpy(m_receive_ba.data(), m_receive_ba.data() + m_buffer_region_start, region_size);
        // Set buffer write position and buffer region start to new values
        AVASSERT(m_receive_buffer.seek(region_size));
        m_buffer_region_start = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingDelimiters::skipJunk(int& message_end_index, int& delimiter_end_index)
{
    message_end_index = -1;
    delimiter_end_index = -1;
    while (true)
    {
        for (int d_pos=0; d_pos<m_end_delimiter.size(); ++d_pos)
        {
            int cur_end_index = findBefore(m_end_delimiter[d_pos], m_buffer_region_start,
                                           m_receive_buffer.pos(), m_receive_ba);
            if (cur_end_index != -1 &&
                (cur_end_index < message_end_index || message_end_index == -1))
            {
                message_end_index   = cur_end_index;
                delimiter_end_index = message_end_index + m_end_delimiter[d_pos].size();
            }
        }

        int start_index = -1;
        if (m_start_delimiter.isEmpty())
        {
            start_index = m_buffer_region_start;
        } else
        {
            // Find the last m_start_delimiter in the valid buffer region before the first
            // found m_end_delimiter (if any).
            // Discard everything preceding it.
            int next_start_index = m_buffer_region_start;
            do {
                next_start_index =
                        findBefore(m_start_delimiter, next_start_index,
                                   message_end_index == -1 ? m_receive_buffer.pos() :
                                                             message_end_index,
                                           m_receive_ba);
                if (next_start_index != -1) {
                    start_index = next_start_index;
                    ++next_start_index;
                }
            } while (next_start_index != -1);
        }
        if (start_index == -1)
        {
            // No m_start_delimiter character before this m_end_delimiter in valid buffer region -
            // discard buffer until this m_end_delimiter and search buffer after m_end_delimiter
            // if necessary.
            if (message_end_index != -1 && message_end_index+1 < m_receive_buffer.pos())
            {
                m_buffer_region_start = message_end_index+1;
                // Loop again
            } else
            {
                AVASSERT(m_receive_buffer.seek(0));
                m_buffer_region_start = 0;
                message_end_index = -1;
                return;
            }
        } else
        {
            // Skip until m_start_delimiter
            AVASSERT(start_index <= m_receive_buffer.pos());
            m_buffer_region_start = start_index;
            return;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////

int AVDataFramingDelimiters::findBefore(const QByteArray& str, uint start_index, uint before,
                                        const QByteArray& ba)
{
    before = AVmin(static_cast<uint>(ba.size()), before);
    // Detect if str doesn't even match into space between start_index and before
    if (start_index >= before) return -1;
    if (before - start_index < static_cast<uint>(str.size())) return -1;
    const char* cur_pos = ba.data()+start_index;
    // Last position with which str can possibly start.
    const char* last_possible_pos = ba.data()+before-str.size();
    AVASSERT(last_possible_pos >= cur_pos);
    // Not exactly the most efficient way to do it, but str will be one character most of the time.
    uint len = str.size();
    do
    {
        if (memcmp(str.data(), cur_pos, len) == 0)
        {
            int ret = cur_pos - ba.data();
            AVASSERT(ret <= static_cast<int>(before)-str.size());
            return ret;
        }
        ++cur_pos;
    } while (cur_pos != last_possible_pos+1);
    return -1;
}

// end of file
