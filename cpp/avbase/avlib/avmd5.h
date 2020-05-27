///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dr. Thomas Leitner, t.leitner@avibit.com
    \author  QT4-PORT: Alex Wemmer, a.wemmer@avibit.com
    \brief   RSA MD5 routines.
*/

#if !defined(AVMD5_H_INCLUDED)
#define AVMD5_H_INCLUDED

#include <QString>

#include "avlib_export.h"

///////////////////////////////////////////////////////////////////////////////
//! calculate the MD5 sum of an arbitrary string and return it as another
//! hex-encoded string
/*! this function calculates the MD5 sum and places it into a hex-encoded
    string, which is exactly 32 bytes in size.
    \param s the string to calculate the MD5 sum for.
    \param buf pointer to the buffer, where the MD5 sum will be stored;
           the caller has to make sure buf provides enough memory, or you
           will be in trouble.
 */
AVLIB_EXPORT char          *AVMD5EncodeHex(const char *s, char *buf);

//! calculate the MD5 sum of the contents of a file and return it as another
//! hex-encoded string
/*! this function calculates the MD5 sum and places it into a hex-encoded
    string, which is exactly 32 bytes in size.
    \param filename the name of the file holding the data to digest
    \param md5 the MD5 sum of the file contents
    \return true on success, false otherwise
 */
AVLIB_EXPORT bool AVMD5EncodeHexFromFile(const QString& filename, QString& md5);

//! calculate the MD5 sum of a byte array and return it a hex-encoded QString
/*! this function calculates the MD5 sum and places it into a hex-encoded
    string, which is exactly 32 bytes in size.
    \param data the name of the unsigned char array which holds the binary data to digest
    \param num_bytes the number of bytes to process in the data array
    \param md5 the MD5 sum of the binary data
    \return true on success, false otherwise
 */
AVLIB_EXPORT bool AVMD5EncodeHexFromBinData(const unsigned char* data, uint num_bytes, QString& md5);

//! calculate the MD5 sum of an arbitrary string and return it as a byte array
/*! this function calculates the MD5 sum and places it into string,
    which is exactly 16 bytes in size.
    \param s the string to calculate the MD5 sum for.
    \param buf pointer to the buffer, where the MD5 sum will be stored;
           the caller has to make sure buf provides enough memory, or you
           will be in trouble.
 */
AVLIB_EXPORT unsigned char *AVMD5Encode(const char *s, unsigned char *buf);

//! calculate the MD5 sum of an arbitrary QString and return it as a
//! hex-encoded QString
inline void    AVMD5Encode(const QString &s, QString &sum) {
    char buf[50];
    QByteArray array = s.toLatin1();
    sum = QString(AVMD5EncodeHex(array.constData(), buf));
}

//! calculate the MD5 sum of an arbitrary QString and return it as a
//! hex-encoded QString
inline QString AVMD5Encode(const QString &s)
{
    char buf[50];
    QByteArray array = s.toLatin1();
    return QString(AVMD5EncodeHex(array.constData(), buf));
}

#endif

// End of file
