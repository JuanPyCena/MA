///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright:  AviBit data processing GmbH, 2001-2020
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>

#include "avlib_export.h"
#include "avmacros.h"
#include "avcstring.h"
#include "avfromtostring.h"

/*!
 * \brief The AVToStringRepresentation class represents a string which corresponds to an output of AVToString().
 *
 * In contrast to a QString, strings of this type will not be (un)quoted when passed to AVFromString/AVToString (and there
 * is no special handling for null / empty strings).
 * Useful for building complex AVToString representations (e.g. maps) from their components.
 * Using e.g. QMap<QString, QString> would interfere by treating those components as user strings and doing quoting.
 */
class AVLIB_EXPORT AVToStringRepresentation : public QString
{
public:

    inline AVToStringRepresentation() = default;
    inline AVToStringRepresentation(const AVToStringRepresentation& other) = default;
    inline AVToStringRepresentation(const char* c_str) : QString(c_str) {}
    inline AVToStringRepresentation(const AVCString& avcstring) : QString(avcstring) {}

    // Forwarding constructors
    explicit AVToStringRepresentation(const QChar *unicode, int size = -1) : QString(unicode, size) {}
    AVToStringRepresentation(QChar c) : QString(c) {}
    AVToStringRepresentation(int size, QChar c) : QString(size, c) {}
    inline AVToStringRepresentation(QLatin1String latin1) : QString(latin1) {}
    inline AVToStringRepresentation(const QString& string) : QString(string) {}

#ifdef Q_COMPILER_RVALUE_REFS
    AVToStringRepresentation(QString&& string) : QString(std::move(string)) {}
#endif
};

///////////////////////////////////////////////////////////////////////////////

template <>
bool AVLIB_EXPORT AVFromString(const QString& str, AVToStringRepresentation& arg);

///////////////////////////////////////////////////////////////////////////////

template <>
QString AVLIB_EXPORT AVToString(const AVToStringRepresentation& arg, bool enable_escape);

// End of file
