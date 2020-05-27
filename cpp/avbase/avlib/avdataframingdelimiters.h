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

#if !defined(AVDATAFRAMINGDELIMITERS_H_INCLUDED)
#define AVDATAFRAMINGDELIMITERS_H_INCLUDED

#include <QObject>
#include <QBuffer>
#include <QVector>

#include "avlib_export.h"

/////////////////////////////////////////////////////////////////////////////
/*!
 *  Framing between a start string and an end string. Junk which is not between the two
 *  characters is discarded. Start characters may be empty.
 *  // TODO CM let user specify maximum buffer size.
 *  // TODO CM write unit tests.
 *  // TODO CM use this in daq_awos.
 */
class AVLIB_EXPORT AVDataFramingDelimiters : public QObject
{
    Q_OBJECT
public:
    AVDataFramingDelimiters(const QByteArray& start_delimiter,
                            const QByteArray& end_delimiter);
    ~AVDataFramingDelimiters() override;

    //! resets the internal buffers
    void reset();

    void setDelimiters(const QByteArray& start_delimiter, const QByteArray& end_delimiter);

    //! telnet sends '\r\n' or '\r\0', stdinput just '\n'. This method allows to account for
    //! all those possibilities.
    void addOptionalEndDelimiter(const QByteArray& end_delimiter);

signals:
    void signalGotMessage(const QByteArray& data);

public slots:
    void slotGotData(const QByteArray& data);

protected:
    //! Skip bytes (adjusting m_buffer_region_start) until m_start_delimiter which is
    //! followed by m_end_delimiter (without m_start_delimiter in between),
    //! or which is the last m_start_delimiter in the valid buffer region.
    //! If no m_buffer_region_start is found, m_buffer_region_start and the m_receive_buffer
    //! current position are both set to 0.
    //!
    //! \param message_end_index [out] The index of the first character not included in the message
    //!                                (first end delimiter character), or -1
    //! \param delimiter_end_index [out] The first character after the end delimiter, or -1.
    void skipJunk(int& message_end_index, int& delimiter_end_index);
    //! Find str in the specified byte array, starting at index, and excluding bytes after "before".
    //! If no such string can be found, -1 is returned.
    //!
    //! \param str         The string to be found
    //! \param start_index The index in ba where to start searching.
    //! \param before      The string must end before this index. Pass -1 to work without this
    //!                    restriction.
    //! \param ba          The byte array to search for the string.
    //! \return            The index of str in ba, or -1 if no matching substring is found.
    int findBefore(const QByteArray& str, uint start_index, uint before, const QByteArray& ba);
    //! Messages are assembled in this byte array.
    QByteArray m_receive_ba;
    //! This buffer is used to write incoming data into our m_receive_ba, enlarging it as necessary.
    QBuffer m_receive_buffer;
    //! The start of the message currently being assembled in m_receive_ba.
    uint m_buffer_region_start;

    //! Marks the start of the message. Can be empty if there is only an end delimiter.
    QByteArray m_start_delimiter;
    //! Marks the end of the message. Cannot be empty.
    QVector<QByteArray> m_end_delimiter;
};

#endif

// End of file
