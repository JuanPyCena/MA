//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVLIB - Avibit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   This file contains helpers for usage of Q_ENUMs
 */

#ifndef AVENUMHELPERS_H
#define AVENUMHELPERS_H

#include <QMetaType>

//! This meta function determines if a given type is a enum declared with Q_ENUM
template <typename T>
struct is_qenum : public std::integral_constant<bool, QtPrivate::IsQEnumHelper<T>::Value>
{};

///////////////////////////////////////////////////////////////////////////////
//! \brief Function to get all enum values in a QList
//! \param Dummy parameter to enable ADL
//! \return QList of all values of this enum.
//! T needs to be declared with Q_ENUM macro.
//! ATTENTION: A function with the same name is defined for AVEnums by the DEFINE_{TYPESAFE_}ENUM marcros (avmacros.h)
//! and needs to have the same signature in order to be found via ADL. This enables usage in a generic context.
template <typename T>
typename std::enable_if<is_qenum<T>::value, QList<T>>::type
AVGetEnumValues(T = T{}) {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        int key_count = metaEnum.keyCount();
        QList<T> result;
        for (int i = 0; i < key_count; i++)
        {
            int value = metaEnum.value(i);
            result << static_cast<T>(value);
        }
        return result;
}

#endif // AVENUMHELPERS_H
