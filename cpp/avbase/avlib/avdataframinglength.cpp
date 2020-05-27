///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT4-Equivalent:
// QT4-Approach: port
// QT4-Progress: none
// QT4-Tested: no
// QT4-Problems:
// QT4-Comment:
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Andreas Landgraf, a.landgraf@avibit.com
    \brief   Data framing with length parsing
 */


#include "avmisc.h"
#include "avdataframinglength.h"


///////////////////////////////////////////////////////////////////////////////

AVDataFramingLength::AVDataFramingLength(int headerLength, int offsetLengthField,
                                         LengthFieldSize sizeLengthField, int buffer_size,
                                         QDataStream::ByteOrder byte_order) :
    m_buffer                (buffer_size, '\0'),
    m_max_buffer_size       (buffer_size*2), // take 2x maxLength as due the index adjustment, we loose some memory
    m_write_index           (0),
    m_read_index            (0),
    m_current_data_length   (0),
    m_current_header_valid  (false),
    m_header_length         (headerLength),
    m_offset_length_field   (offsetLengthField),
    m_size_length_field     (sizeLengthField),
    m_max_data_length       (buffer_size),
    m_byte_order            (byte_order),
    m_reset_request         (false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVDataFramingLength::~AVDataFramingLength()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingLength::slotGotData(const QByteArray& data)
{
    if(m_reset_request)
        performReset();

    if(numBytesInBuffer() == 0)
    {
        AVLogDebug2 << "adjusting read and write indices, buffer is empty";
        m_write_index = 0;
        m_read_index = 0;
    }
    else if(m_write_index > m_buffer.count()/2 && m_read_index != 0)
    {
        AVLogDebug2 << "adjusting read and write indices, buffer is NOT empty";
        // copy data to the beginning of the byte array
        int n_bytes_to_write = numBytesInBuffer();
        memmove(m_buffer.data(), m_buffer.data() + m_read_index, n_bytes_to_write);
        m_write_index = n_bytes_to_write;
        m_read_index = 0;
        AVASSERT(numBytesInBuffer() == n_bytes_to_write);
    }

    // reasonable sanity check
    if (data.count() > m_max_buffer_size/2)
    {
        AVLogger->Write(LOG_WARNING, "AVDataFramingLength: received data length (%d bytes) is bigger maxbuf/2 (%d bytes)",
                        data.count(), m_max_buffer_size/2);
    }

    // check buffer size
    int needed_buffer_size = m_write_index + data.count();
    bool buffer_big_enough = m_buffer.count() >= needed_buffer_size;
    if(buffer_big_enough == false)
    {
        if(needed_buffer_size <= m_max_buffer_size)
        {
            // enlarge buffer until m_max_buffer_size
            AVLogDebug2 << "enlarging buffer to " << needed_buffer_size << " bytes";
            m_buffer.resize(needed_buffer_size);
            buffer_big_enough = true;
        }
        else
        {
            AVLogger->Write(LOG_ERROR, "AVDataFramingLength: Buffer overrun. Resetting buffer!");
            reset();
        }
    }

    if(buffer_big_enough)
    {
        memcpy(m_buffer.data() + m_write_index, data.data(), data.count());
        m_write_index += data.count();
    }

    bool data_available = true;
    while(data_available)
    {
        data_available = parseMessage();

        if(m_reset_request)
        {
            // request happended while parsing
            performReset();
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingLength::reset()
{
    m_reset_request = true;
}

///////////////////////////////////////////////////////////////////////////////

void AVDataFramingLength::performReset()
{
    m_current_data_length = 0;
    m_current_header_valid = false;
    m_write_index = 0;
    m_read_index = 0;
    m_reset_request = false;
}
///////////////////////////////////////////////////////////////////////////////

bool AVDataFramingLength::parseMessage()
{
    if (numBytesInBuffer() < m_header_length)
        return false;

    const char* data_to_read = m_buffer.data() + m_read_index;
    if (!m_current_header_valid)
    {
        QByteArray networkValue;
        networkValue.setRawData(data_to_read + m_offset_length_field, m_size_length_field);
        QDataStream stream(&networkValue, QIODevice::ReadOnly);
        // This function sets the byte order of incoming data; network format is BigEndian,
        // while Intel format is LittleEndian
        stream.setByteOrder(m_byte_order);
        switch (m_size_length_field)
        {
            case AVDataFramingLength::SHORT:
                {
                    quint16 value = 0;
                    stream >> value;
                    m_current_data_length = value;
                }
                break;
            case AVDataFramingLength::INT:
                {
                    quint32 value = 0;
                    stream >> value;
                    m_current_data_length = value;
                }
                break;
            default:
                m_current_data_length = 0;
                break;
        }
        networkValue.clear();
        if (m_current_data_length > m_max_data_length)
        {
            emit signalMaxLengthExceeded(m_current_data_length);
            return false;
        }
        m_current_header_valid = true;
    }
    if (m_current_header_valid && (numBytesInBuffer() >= (m_header_length + m_current_data_length)))
    {
        QByteArray header;
        QByteArray data;
        header.setRawData(data_to_read, m_header_length);
        data.setRawData(data_to_read + m_header_length, m_current_data_length);
        emit signalGotMessage(header, data);
        data.clear();
        header.clear();

        QByteArray full;
        full.setRawData(data_to_read, m_header_length+m_current_data_length);
        emit signalGotFullMessage(full);
        full.clear();

        // advance the read index
        m_read_index += m_header_length + m_current_data_length;

        m_current_data_length = 0;
        m_current_header_valid = false;

        uint remainingLength = numBytesInBuffer();
        if(remainingLength > 0)
            return true; // we have unprocessed data still
    }

    return false;
}

// end of file
