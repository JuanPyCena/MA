///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-Port: Anton Skrabal, a.skrabal@avibit.com
    \brief
*/



#include <qbuffer.h>

#include "avlog.h"

#include "avdataencoderrawpacket.h"

#define BLOCKSIZE 4

#define DEBUG_LOGGING 0

/////////////////////////////////////////////////////////////////////////////

AVDataEncoderRawPacket::AVDataEncoderRawPacket(
    const QString& debug_name,
    const quint32& packet_delimiter_first_half,
    const quint32& packet_delimiter_second_half,
    const quint32& packet_padding) : AVDataEncoderBase(debug_name)
{
    m_packet_delimiter_first_half = packet_delimiter_first_half;
    m_packet_delimiter_second_half = packet_delimiter_second_half;
    m_packet_padding = packet_padding;

    AVASSERT(m_packet_padding != m_packet_delimiter_first_half);
    AVASSERT(m_packet_padding != m_packet_delimiter_second_half);

    AVASSERT(DATA_CODEC_RAW_PACKET_ID != m_packet_delimiter_first_half);
    AVASSERT(DATA_CODEC_RAW_PACKET_ID != m_packet_delimiter_second_half);
    AVASSERT(DATA_CODEC_RAW_PACKET_ID != m_packet_padding);
    AVASSERT(DATA_CODEC_RAW_PACKET_LENGTH_ID != m_packet_delimiter_first_half);
    AVASSERT(DATA_CODEC_RAW_PACKET_LENGTH_ID != m_packet_delimiter_second_half);
    AVASSERT(DATA_CODEC_RAW_PACKET_LENGTH_ID != m_packet_padding);

    for(quint32 possible_end_padding_count = 0;
        possible_end_padding_count < BLOCKSIZE;
        ++possible_end_padding_count)
    {
        if (m_packet_delimiter_first_half == possible_end_padding_count ||
            m_packet_delimiter_second_half == possible_end_padding_count ||
            m_packet_padding == possible_end_padding_count)
        {
            LOGGER_ROOT.Write(LOG_FATAL, "AVDataEncoderRawPacket: %s: "
                            "either the delimiter or padding matches a possible "
                            "end padding count (the padding range is 0 to %d)",
                            qPrintable(m_debug_name), BLOCKSIZE-1);
            AVASSERT(0);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool AVDataEncoderRawPacket::internal_encode(QIODevice* read_device,
                                             QIODevice* write_device_external,
                                             quint32 nr_bytes_to_read,
                                             quint32& real_nr_bytes_read,
                                             quint32& real_nr_bytes_written)
{
    // sanity checks

    AVASSERT(read_device);
    AVASSERT(write_device_external);
    AVASSERT(real_nr_bytes_read == 0);
    AVASSERT(real_nr_bytes_written == 0);
    AVASSERT(sizeof(quint32) == BLOCKSIZE);

    // write the data codec identifer
    QBuffer write_device;
    AVASSERT(write_device.open(QIODevice::WriteOnly));

    // encode

    quint32 block = 0;
    quint32 previous_block = 0;
    quint32 nr_end_padding_bytes_added = 0;

    quint32 missing_bytes_to_read = nr_bytes_to_read;
    unsigned int nr_intermediate_paddings_added = 0;
    bool first_block = true;

    while(!read_device->atEnd() && (nr_bytes_to_read == 0 || missing_bytes_to_read > 0))
    {
        // if the end padding was added, we should not read any further data!
        AVASSERT(nr_end_padding_bytes_added == 0);

        // determine how many bytes should be read

        quint32 bytes_to_read = BLOCKSIZE;
        if (nr_bytes_to_read > 0 && missing_bytes_to_read < BLOCKSIZE)
            bytes_to_read = missing_bytes_to_read;

        // read the bytes

        block = 0;
        // TODO: refactor the follwing line. reinterpret_cast should not be used, bytes_to_read must be 4 in that case!
        int read_block_result = read_device->read(reinterpret_cast<char*>(&block), bytes_to_read);
        real_nr_bytes_read += read_block_result;
        missing_bytes_to_read -= read_block_result;

        // if this is the last block, determine end padding bytes which will
        // be added by writing the whole block

        if (read_device->atEnd() || (nr_bytes_to_read > 0 && missing_bytes_to_read == 0))
            nr_end_padding_bytes_added = BLOCKSIZE - read_block_result;

        // the first block is always written without padding

        if (first_block)
        {
            first_block = false;
            // TODO: refactor
            AVASSERT(write_device.write(reinterpret_cast<char*>(&block), BLOCKSIZE) == BLOCKSIZE);
            previous_block = block;
            continue;
        }

        // add intermediate padding to ensure the delimiter does not occur in the encoded stream

        if ( (block == m_packet_delimiter_first_half &&
              previous_block == m_packet_delimiter_second_half) ||
             (block == m_packet_padding))
        {
            AVASSERT(write_device.write(reinterpret_cast<char*>(&m_packet_padding), BLOCKSIZE) == BLOCKSIZE);
            ++nr_intermediate_paddings_added;
        }

        // write the block

        // TODO: refactor
        AVASSERT(write_device.write(reinterpret_cast<char*>(&block), BLOCKSIZE) == BLOCKSIZE);
        previous_block = block;
    }

    // write number of padding bytes, this will be used by the decode method
    // Note: Some checks in the constructor take care, that any possible end
    // padding byte count will not collide with the used delimiter!

    // TODO: refactor
    AVASSERT(write_device.write(reinterpret_cast<char*>(&nr_end_padding_bytes_added), BLOCKSIZE) == BLOCKSIZE);

    // write the delimiter

    // TODO: refactor
    AVASSERT(write_device.write(reinterpret_cast<char*>(&m_packet_delimiter_second_half), BLOCKSIZE) == BLOCKSIZE);

    // TODO: refactor
    AVASSERT(write_device.write(reinterpret_cast<char*>(&m_packet_delimiter_first_half), BLOCKSIZE) == BLOCKSIZE);

    // debug output

#if DEBUG_LOGGING
    LOGGER_ROOT.Write(LOG_DEBUG1, "AVDataEncoderRawPacket:internal_encode: %s: "
                    "intermediate paddings: %d, end paddings: %d", qPrintable(m_debug_name),
                    nr_intermediate_paddings_added, nr_end_padding_bytes_added);
#endif

    if (nr_bytes_to_read > 0 &&
        (real_nr_bytes_read != nr_bytes_to_read || missing_bytes_to_read != 0))
    {
        LOGGER_ROOT.Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_encode: %s: "
                        "read bytes %d != wanted bytes %d (missing: %d)", qPrintable(m_debug_name),
                        real_nr_bytes_read, nr_bytes_to_read, missing_bytes_to_read);
        return false;
    }

    quint32 codec_identifier = DATA_CODEC_RAW_PACKET_LENGTH_ID;
    // TODO: refactor
    AVASSERT(write_device_external->write(reinterpret_cast<char*>(&codec_identifier), BLOCKSIZE) == BLOCKSIZE);
    quint32 size = write_device.buffer().size() + BLOCKSIZE + BLOCKSIZE;
    // TODO: refactor
    AVASSERT(write_device_external->write(reinterpret_cast<char*>(&size), BLOCKSIZE) == BLOCKSIZE);
    AVASSERT(write_device_external->write(write_device.buffer()) ==
             static_cast<int>(write_device.buffer().size()));
    real_nr_bytes_written = size;

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDataEncoderRawPacket::internal_decode(QIODevice* read_device,
                                             QIODevice* write_device,
                                             quint32 nr_max_bytes_to_read,
                                             quint32,
                                             quint32& real_nr_bytes_read,
                                             quint32& real_nr_bytes_written)
{
    // sanity checks

    AVASSERT(read_device);
    AVASSERT(write_device);
    AVASSERT(real_nr_bytes_read == 0);
    AVASSERT(real_nr_bytes_written == 0);
    AVASSERT(sizeof(quint32) == BLOCKSIZE);

    // check the data codec identifer

    quint32 codec_identifier;
    // TODO: refactor
    if (read_device->read(reinterpret_cast<char*>(&codec_identifier), BLOCKSIZE) <= 0)
    {
        AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                        "could not read data coded ID from device - aborting",
                        qPrintable(m_debug_name));
        return false;
    }

    real_nr_bytes_read += BLOCKSIZE;

    if (codec_identifier == DATA_CODEC_RAW_PACKET_ID)
    {
        quint32 dummy = 0;
        return internal_decode_payload(read_device,
                                       write_device,
                                       nr_max_bytes_to_read,
                                       dummy,
                                       real_nr_bytes_read,
                                       real_nr_bytes_written);
    }
    else if (codec_identifier == DATA_CODEC_RAW_PACKET_LENGTH_ID)
    {
        quint32 size;
        // TODO: refactor
        if (read_device->read(reinterpret_cast<char*>(&size), BLOCKSIZE) <= 0)
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                            "could not read data size from device - aborting",
                            qPrintable(m_debug_name));
            return false;
        }
        AVASSERT(size >= 2*BLOCKSIZE);
        size -= 2*BLOCKSIZE;

        QByteArray ba(size, '\0');
        int bytes_read = 0;
        while (bytes_read < static_cast<int>(size))
        {
            int bytes_read_now = read_device->read(ba.data() + bytes_read, size - bytes_read);
            if (bytes_read_now <= 0)
            {
                AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                                "could not read data from device - aborting",
                                qPrintable(m_debug_name));
                return false;
            }
            bytes_read += bytes_read_now;
        }
        AVASSERT(bytes_read == static_cast<int>(size));

        QBuffer read_buffer_internal(&ba);
        AVASSERT(read_buffer_internal.open(QIODevice::ReadOnly));

        quint32 dummy = 0;
        return internal_decode_payload(&read_buffer_internal,
                                       write_device,
                                       nr_max_bytes_to_read,
                                       dummy,
                                       real_nr_bytes_read,
                                       real_nr_bytes_written);
    }

    AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                    "read codec id (%d) != awaited codec id (%d or %d)",
                    qPrintable(m_debug_name), codec_identifier, DATA_CODEC_RAW_PACKET_ID,
                    DATA_CODEC_RAW_PACKET_LENGTH_ID);
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDataEncoderRawPacket::internal_decode_payload(QIODevice* read_device,
                                                     QIODevice* write_device,
                                                     quint32 nr_max_bytes_to_read,
                                                     quint32,
                                                     quint32& real_nr_bytes_read,
                                                     quint32& real_nr_bytes_written)
{
    // sanity checks

    AVASSERT(read_device);
    AVASSERT(write_device);
    AVASSERT(sizeof(quint32) == BLOCKSIZE);

    // decode

    bool previous_block_was_padding = false;
    quint32 nr_intermediate_paddings = 0;
    bool found_delimiter = false;

    // the decode buffer should hold the last 4 read blocks
    QVector<quint32> decode_buffer;
    const int max_decode_buffer_len = 4;
    AVASSERT(decode_buffer.count() == 0);

    while(!read_device->atEnd())
    {
        // read block

        quint32 block = 0;
        // TODO: refactor
        if (read_device->read(reinterpret_cast<char*>(&block), BLOCKSIZE) != BLOCKSIZE)
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                            "could not read block from input - aborting",
                            qPrintable(m_debug_name));
            return false;
        }

        real_nr_bytes_read += BLOCKSIZE;

        if (nr_max_bytes_to_read > 0 && real_nr_bytes_read > nr_max_bytes_to_read)
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                            "already read %d bytes, but got restriction of %d bytes - aborting",
                            qPrintable(m_debug_name), real_nr_bytes_read, nr_max_bytes_to_read);
            return false;
        }

        // put the block into the decode buffer

        decode_buffer.append(block);
        //printQVector(decode_buffer);
        if (decode_buffer.count() == 1) continue;

        int decode_buffer_len = decode_buffer.count();
        AVASSERT(decode_buffer_len >= 2);
        AVASSERT(decode_buffer_len <= max_decode_buffer_len);

        // detect delimiter

        if (decode_buffer[decode_buffer_len-1] == m_packet_delimiter_first_half &&
            decode_buffer[decode_buffer_len-2] == m_packet_delimiter_second_half &&
            !previous_block_was_padding)
        {
            found_delimiter = true;
            break;
        }

        // remove padding

        if (block == m_packet_padding && !previous_block_was_padding)
        {
            previous_block_was_padding = true;
            ++nr_intermediate_paddings;
            //decode_buffer.pop_back();
            decode_buffer.resize(decode_buffer_len-1);
            AVASSERT(decode_buffer.count() == decode_buffer_len-1);
            continue;
        }
        else
        {
            previous_block_was_padding = false;
        }

        // write the oldest block from the decode buffer if the buffer is full

        if (decode_buffer_len == max_decode_buffer_len)
        {
            // TODO: refactor
            AVASSERT(write_device->write(reinterpret_cast<char*>(&(decode_buffer[0])), BLOCKSIZE) == BLOCKSIZE);
            real_nr_bytes_written += BLOCKSIZE;
            decode_buffer.erase(decode_buffer.begin());
            AVASSERT(decode_buffer.count() == decode_buffer_len-1);
        }
    }

    //printQVector(decode_buffer);

    // process the last block

    if (!found_delimiter)
    {
        // we got the an end and did not find the delimiter -> error

        AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                        "got to the end, but did not find a delimiter",
                        qPrintable(m_debug_name));
        return false;
    }
    else if (decode_buffer.count() < 3)
    {
        // we found a delimiter but not end padding byte count

        AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                        "got to the end, but no end padding byte count in decode buffer",
                        qPrintable(m_debug_name));
        return false;
    }
    else if (decode_buffer.count() == 3)
    {
        quint32 nr_end_padding_bytes = decode_buffer[0];
        AVASSERT(nr_end_padding_bytes < BLOCKSIZE);

        if (nr_end_padding_bytes != 0)
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderRawPacket:internal_decode: %s: "
                            "got to the end and got end padding byte count %d !=0 but no data",
                            qPrintable(m_debug_name), nr_end_padding_bytes);
            return false;
        }
        else
        {
#if DEBUG_LOGGING
            AVLogger->Write(LOG_DEBUG1, "AVDataEncoderRawPacket:internal_decode: %s: "
                            "no data, intermediate padding: %u, end paddings: %u",
                            qPrintable(m_debug_name), nr_intermediate_paddings,
                            nr_end_padding_bytes);
#endif
            return true;
        }
    }
    else
    {
        // process the last block and take the number of end pading bytes into account

        AVASSERT(decode_buffer.count() == max_decode_buffer_len);

        quint32 last_data_block = decode_buffer[0];
        quint32 nr_end_padding_bytes = decode_buffer[1];
        AVASSERT(nr_end_padding_bytes < BLOCKSIZE);

#if DEBUG_LOGGING
        AVLogger->Write(LOG_DEBUG1, "AVDataEncoderRawPacket:internal_decode: %s: "
                        "intermediate padding: %u, end paddings: %u",
                        qPrintable(m_debug_name), nr_intermediate_paddings, nr_end_padding_bytes);
#endif

        qint64 bytes_to_write = BLOCKSIZE - nr_end_padding_bytes;

        // TODO: refactor
        AVASSERT(write_device->write(reinterpret_cast<char*>(&last_data_block), bytes_to_write) ==
                 bytes_to_write);
        real_nr_bytes_written += bytes_to_write;

        return true;
    }

    AVASSERT(0);
    return false;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
