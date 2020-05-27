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

#if !defined(AVDATAFRAMINGLENGTH_H_INCLUDED)
#define AVDATAFRAMINGLENGTH_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

// Qt includes
#include "qglobal.h"
#include "qobject.h"
#include "qbuffer.h"

// AVLib includes
#include "avlib_export.h"
#include "avcstring.h"
#include "avlog.h"
#include "avmacros.h"

//////////////////////////////////////////////////////////////////////////////
//! This is a data framing class parsing length of data out of header.
/*! Parses the data length out of the header with given offset. No start/end
 * delimiters are used.
 */

class AVLIB_EXPORT AVDataFramingLength : public QObject
{
    Q_OBJECT

    friend class AVDataFramingLengthTest;

public:
    enum LengthFieldSize
    {
        SHORT = 2,
        INT   = 4
    };

    //! constructor
    /*!
     * @param headerLength        byte size of the header
     * @param offsetLengthField   offset in bytes of the length field within the header
     * @param sizeLengthField     byte size of the length field
     * @param maxLength           size of the buffer
     * @param byte_order          the byte order in which the data is interpreted.
     *                            Normally network byte order is big endian
     */
    AVDataFramingLength(int headerLength, int offsetLengthField,
                        LengthFieldSize sizeLengthField, int buffer_size=32768,
                        QDataStream::ByteOrder byte_order = QDataStream::BigEndian);
    ~AVDataFramingLength() override;

    //! sets the reset request in order to clear the internal buffers before the next message is
    //! read.
    void reset();

signals:
    //! Signal which is emitted if the header's length field exceeds the buffer bounds
    void signalMaxLengthExceeded(const int length);

    //! Signal which is emitted if a message was framed.
    /*!
     * @param header    The header bytes
     * @param data      The data bytes
     */
    void signalGotMessage(const QByteArray& header, const QByteArray& data);

    //! Signal which emits the full message (header&data)
    void signalGotFullMessage(const QByteArray& full_msg);

public slots:
    //! slot which takes data to be framed
    void slotGotData(const QByteArray& data);

protected:
    //! performs the actual reset
    void performReset();

    //! Parses m_receiveBuffer and extracts messages. Returns wheter more data is available to
    //! be parsed
    bool parseMessage();

    //! returns the number of currently stored bytes
    inline int numBytesInBuffer() const
    {
        int num = m_write_index - m_read_index;
        AVASSERT(num >= 0);
        return num;
    }

    //! Messages are assembled in this byte array.
    QByteArray      		 m_buffer;
    //! maximum size of the buffer
    int                     m_max_buffer_size;
    //! the index in buffer where writing takes currently place
    int                     m_write_index;
    //! the index in buffer where reading takes currently place
    int                     m_read_index;
    //! The current value of the length field of the message header.
    int          		     m_current_data_length;
    //! The status of the header we currently parsing.
    bool             		 m_current_header_valid;
    //! The length of the header of a message.
    int            		 m_header_length;
    //! This is the offset of the length field in the header of the message.
    int            		 m_offset_length_field;
    //! The size of the length field in bytes.
    LengthFieldSize 		 m_size_length_field;
    //! The maximum length of the data (used for error detection).
    int           			 m_max_data_length;
    //! The byte order in the data
    QDataStream::ByteOrder   m_byte_order;
    //! true if a reset is requested in order to handle it asynchronous
    bool                     m_reset_request;
};

#endif

// End of file
