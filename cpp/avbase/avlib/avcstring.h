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
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   avcstring class header.
 */

#if !defined AVCSTRING_H_INCLUDED
#define AVCSTRING_H_INCLUDED

// QT includes
#include <QByteArray>
#include <QString>
#include <QMetaType>

// AVCOMMON includes
#include "avlib_export.h"
#include "avlog.h"

// local includes


/////////////////////////////////////////////////////////////////////////////
//! The AVCString provides a low-level string comparable to QCString of QT3
/*!
 * To be compatible with QT3 QCString it also terminates with an additional \0 byte
 * (which the QByteArray does not). The \0 byte is not appended in case of a string of size 0.
 */

class AVLIB_EXPORT AVCString : public QByteArray
{
public:
    AVCString() = default;

    template <typename T, typename = decltype (QByteArray(std::declval<T>()))>
    AVCString(T&& data) noexcept(noexcept(QByteArray(std::forward<T>(data)))) : QByteArray(std::forward<T>(data))
    {
    }

    template <typename T, int N>
    Q_DECL_DEPRECATED_X("Use QByteArrayLiteral instead") AVCString(T(&string_literal)[N]) = delete;

    Q_DECL_DEPRECATED_X("Use QByteArrayLiteral instead") AVCString(const char*& string) : QByteArray(string)
    {
    }

    using QByteArray::operator=;
    Q_DECL_DEPRECATED_X("Use QByteArrayLiteral instead") AVCString& operator=(const char* string)
    {
        QByteArray::operator=(string);
        return *this;
    }

};

//! Serialization operator, compatible with QCString of QT3
AVLIB_EXPORT QDataStream &operator<< (QDataStream & out, const AVCString & ba);

//! Deserialization operator, compatible with QCString of QT3
AVLIB_EXPORT QDataStream &operator>> (QDataStream & in, AVCString & ba);

Q_DECLARE_METATYPE(AVCString)


#endif

// End of file
