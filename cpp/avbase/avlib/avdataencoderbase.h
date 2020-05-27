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
    \brief   Base class for data encoder classes.
*/

#ifndef __AVDATAENCODERBASE_H__
#define __AVDATAENCODERBASE_H__

#include "avlib_export.h"
#include "avlog.h"

#include "qiodevice.h"
#include "qstring.h"
#include "qdatetime.h"

#define DEBUG_LOGGING 0

/////////////////////////////////////////////////////////////////////////////

// data codec defines - this codes are used by the data encoders

#define DATA_CODEC_PLAIN_ID 1
#define DATA_CODEC_RAW_PACKET_ID 2
#define DATA_CODEC_RAW_PACKET_LENGTH_ID 3

/////////////////////////////////////////////////////////////////////////////


//! Base class for data encoder classes.
//! Classes derived from this interface should overwrite the internal_encode and
//! internal_decode methods. The encode and decode methods implemented in
//! this base class will check the given read/write devices for reading,
//! writing, thus any derived class does not have to deal with it.
//! Additionally, this base class will print out some debug statistics.
//! When writing a new encoder class, please make sure to add a DATA_CODEC_XXX_ID
//! definition to this header file and use this code to identify the codec of
//! the processed data stream, thus the stream may not be decoded by the wrong
//! encoder class.
class AVLIB_EXPORT AVDataEncoderBase
{
public:

    //! Constructor
    /*!
      \param debug_name Will be used for debugging and logging purposes only.
    */
    explicit AVDataEncoderBase(const QString& debug_name = "")
    {
        m_debug_name = debug_name;
    };

    virtual ~AVDataEncoderBase() {};

    /////////////////////////////////////////////////////////////////////////////

    //! Reads raw data from "read_device" and writes out the encoded
    //! packet to "write_device".
    /*!
      \param read_device Must be in "ready to read" configuration.
      \param write_device Must be in "ready to write" configuration.
      \param nr_bytes_to_read The number of bytes we should read from the
      read_device. If this is "0", all bytes will be read.
      \param real_nr_bytes_read Returns the real number of bytes read.
      \param real_nr_bytes_written Returns the real number of bytes written.
      \return Returns false on error, true otherwise.
    */
    bool encode(QIODevice* read_device, QIODevice* write_device, quint32 nr_bytes_to_read,
                quint32& real_nr_bytes_read, quint32& real_nr_bytes_written)
    {
        real_nr_bytes_read = 0;
        real_nr_bytes_written = 0;

#if DEBUG_LOGGING
        AVLogger->Write(LOG_DEBUG1, "AVDataEncoderBase:encode: %s: "
                        "start (wanted bytes: %u (0=all))", qPrintable(m_debug_name),
                        nr_bytes_to_read);
#endif

        // check devices

        if (!checkReadDevice(read_device) || !checkWriteDevice(write_device))
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:encode: %s: "
                            "Devices are not setup correctly", qPrintable(m_debug_name));
            return false;
        }

        // prepare statistics

        QTime start_time;
        start_time.restart();

        // encode

        bool ret = internal_encode(read_device, write_device, nr_bytes_to_read,
                                   real_nr_bytes_read, real_nr_bytes_written);

        // print statistics

#if DEBUG_LOGGING
        AVLogger->Write(LOG_DEBUG1, "AVDataEncoderBase:encode: %s: "
                        "read %u Bytes in %.1f s, wrote %u bytes (+%u bytes)",
                        qPrintable(m_debug_name),
                        real_nr_bytes_read, start_time.elapsed()/1000.0, real_nr_bytes_written,
                        real_nr_bytes_written-real_nr_bytes_read);
#endif

        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////

    //! Reads one encoded packet from "read_device" and writes out the decoded
    //! raw data to "write_device".
    /*!
      \param read_device Must be in "ready to read" configuration.
      \param write_device Must be in "ready to write" configuration.
      \param nr_max_bytes_to_read The max. number of bytes to read. The codec
      will abort if it wants to read more bytes than this. If this is "0", the
      codec will read an unrestricted amount of bytes to finish its work.
      \param nr_bytes_to_read The number of bytes we should read from the
      read_device. If this is "0", all bytes will be read. ATTENTION: Some
      codecs may not use this parameter because of internal calculation of how
      many bytes should be read (e.g. delimiter based codecs).
      \param real_nr_bytes_read Returns the real number of bytes read.
      \param real_nr_bytes_written Returns the real number of bytes written.
      \return Returns false on error, true otherwise.
    */
    bool decode(QIODevice* read_device, QIODevice* write_device,
                quint32 nr_max_bytes_to_read, quint32 nr_bytes_to_read,
                quint32& real_nr_bytes_read, quint32& real_nr_bytes_written)
    {
        real_nr_bytes_read = 0;
        real_nr_bytes_written = 0;

#if DEBUG_LOGGING
        AVLogger->Write(LOG_DEBUG1, "AVDataEncoderBase:decode: %s: "
                        "start (wanted bytes: %u (0=all))", qPrintable(m_debug_name),
                        nr_bytes_to_read);
#endif

        // check devices

        if (!checkReadDevice(read_device) || !checkWriteDevice(write_device))
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:decode: %s: "
                            "Devices are not setup correctly", qPrintable(m_debug_name));
            return false;
        }

        // prepare statistics

        QTime start_time;
        start_time.restart();

        // decode

        bool ret = internal_decode(read_device, write_device,
                                   nr_max_bytes_to_read, nr_bytes_to_read,
                                   real_nr_bytes_read, real_nr_bytes_written);

        // print statistics

#if DEBUG_LOGGING
        AVLogger->Write(LOG_DEBUG1, "AVDataEncoderBase:decode: %s: "
                        "wrote %u Bytes in %.1f s, read %u bytes", qPrintable(m_debug_name),
                        real_nr_bytes_written, start_time.elapsed()/1000.0, real_nr_bytes_read);
#endif
        return ret;
    }

protected:

    QString m_debug_name;

    /////////////////////////////////////////////////////////////////////////////

    virtual bool checkReadDevice(QIODevice *read_device)
    {
        if (!read_device)
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:checkReadDevice: %s: "
                            "device is NULL", qPrintable(m_debug_name));
            return false;
        }

        if (!read_device->isOpen())
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:checkReadDevice: %s: "
                            "device is not opened", qPrintable(m_debug_name));
            return false;
        }

        if (!read_device->isReadable())
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:checkReadDevice: %s: "
                            "device is not readable", qPrintable(m_debug_name));
            return false;
        }

        return true;
    }

    /////////////////////////////////////////////////////////////////////////////

    virtual bool checkWriteDevice(QIODevice *write_device)
    {
        if (!write_device)
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:checkWriteDevice: %s: "
                            "device is NULL", qPrintable(m_debug_name));
            return false;
        }

        if (!write_device->isOpen())
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:checkWriteDevice: %s: "
                            "device is not opened", qPrintable(m_debug_name));
            return false;
        }

        if (!write_device->isWritable())
        {
            AVLogger->Write(LOG_ERROR, "AVDataEncoderBase:checkWriteDevice: %s: "
                            "device is not writable", qPrintable(m_debug_name));
            return false;
        }

        return true;
    }

    /////////////////////////////////////////////////////////////////////////////

    //! Reads raw data from "read_device" and writes out the encoded packet to
    //! "write_device".  This method must be overwritten by derived classes and
    //! will get the devices already checked and set up for processing.
    //! Returns false on error, true otherwise.
    virtual bool internal_encode(QIODevice* read_device,
                                 QIODevice* write_device,
                                 quint32 nr_bytes_to_read,
                                 quint32& real_nr_bytes_read,
                                 quint32& real_nr_bytes_written) = 0;

    /////////////////////////////////////////////////////////////////////////////

    //! Reads one encoded packet from "read_device" and writes out the decoded raw data to
    //! "write_device". This method must be overwritten by derived classes and
    //! will get the devices already checked and set up for processing.
    //! Returns false on error, true otherwise.
    virtual bool internal_decode(QIODevice* read_device,
                                 QIODevice* write_device,
                                 quint32 nr_max_bytes_to_read,
                                 quint32 nr_bytes_to_read,
                                 quint32& real_nr_bytes_read,
                                 quint32& real_nr_bytes_written) = 0;

private:

    //! Dummy copy constructor
    AVDataEncoderBase(const AVDataEncoderBase&);

    //! Dummy assignment operator
    AVDataEncoderBase& operator=(const AVDataEncoderBase&);
};

#endif /* __AVDATAENCODERBASE_H__ */

// End of file
