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
    \brief   En/Decoded for raw data packets
*/

#ifndef __AVDATAENCODERRAWPACKET_H__
#define __AVDATAENCODERRAWPACKET_H__

#include <qvector.h>

#include "avlib_export.h"
#include "avdataencoderbase.h"

/////////////////////////////////////////////////////////////////////////////

/*!
  This class provides methods to en/decode raw binary data into packets,
  which will not contain the given 64bit delimiter. To do so, 32bit padding
  bytes will be added/removed to/from the raw data by the encode/decode
  algorithm.
  <br>
  The encoded byte stream will look like this:
  <pre>IeeePeeePeeeePeCD</pre>
  Where:
  <ul>
  <li>I = 32 bit encoder ID</li>
  <li>e = 32 bit encoded data</li>
  <li>P = 32 bit padding</li>
  <li>C = 32 bit end padding bytes count</li>
  <li>D = 64 bit delimiter</li>
  </ul>
*/
class AVLIB_EXPORT AVDataEncoderRawPacket : public AVDataEncoderBase
{
public:

    //! Constructor
    /*!
      \param packet_delimiter_first_half The first 32 bit of the delimiter.
      \param packet_delimiter_first_half The seconde 32 bit of the delimiter.
      \param packet_padding The padding, which must differ from both delimiter parts.
    */
    explicit AVDataEncoderRawPacket(const QString& debug_name,
                                    const quint32& packet_delimiter_first_half = UINT_MAX,
                                    const quint32& packet_delimiter_second_half = UINT_MAX,
                                    const quint32& packet_padding = UINT_MAX -1);

    ~AVDataEncoderRawPacket() override{};

    quint32 getDelimiterFirstHalf() const { return m_packet_delimiter_first_half; }
    quint32 getDelimiterSecondHalf() const { return m_packet_delimiter_second_half; }
    quint32 getPadding() const { return m_packet_padding; }

protected:

    quint32 m_packet_delimiter_first_half;
    quint32 m_packet_delimiter_second_half;
    quint32 m_packet_padding;

    //-----

    //! Reads raw data from "read_device" and writes out the encoded packet to
    //! "write_device".
    //! Returns false on error, true otherwise.
    bool internal_encode(QIODevice* read_device, QIODevice* write_device, quint32 nr_bytes_to_read,
                         quint32& real_nr_bytes_read, quint32& real_nr_bytes_written) override;

    //! Reads one encoded packet from "read_device" and writes out the decoded raw data to
    //! "write_device". This method will not use the "nr_bytes_to_read"
    //! parameter, because this is checked via the delimiters.
    //! Returns false on error, true otherwise.
    bool internal_decode(QIODevice* read_device, QIODevice* write_device, quint32 nr_max_bytes_to_read,
                         quint32 nr_bytes_to_read, quint32& real_nr_bytes_read,
                         quint32& real_nr_bytes_written) override;

    bool internal_decode_payload(QIODevice* read_device,
                                 QIODevice* write_device,
                                 quint32 nr_max_bytes_to_read,
                                 quint32 nr_bytes_to_read,
                                 quint32& real_nr_bytes_read,
                                 quint32& real_nr_bytes_written);

    void printQVector(QVector<quint32>& buffer)
    {
        AVLogger->Write(LOG_INFO, "AVDataEncoderRawPacket:printQValueVector: %s:"
                        "%d entries", qPrintable(m_debug_name), buffer.count());

        for(int index=0; index < buffer.count(); ++index)
        {
            quint32 value = buffer[index];

            printf("\t%d\t%d\t%d\t%d\n",
                   value & 0xff,
                   (value >> 8) & 0xff,
                   (value >> 16) & 0xff,
                   (value >> 24) & 0xff);
        }

        fflush(stdout);
    }

private:

    //! Dummy copy constructor
    AVDataEncoderRawPacket(const AVDataEncoderRawPacket&);

    //! Dummy assignment operator
    AVDataEncoderRawPacket& operator=(const AVDataEncoderRawPacket&);
};

#endif /* __AVRAWPACKETDATAENCODER_H__ */

// End of file
