///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author  QT4-PORT: Wolfgang Eder, w.eder@avibit.com
    \brief    This file defines AVEnumTable.
*/

// Qt includes
#include <QMetaEnum>

// AviBit common includes
#include "avenumtable.h"
#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////

void AVEnumTable::initialize(const QMetaObject& metaObject, const char *enumType, const QString& stripPrefix)
{
    AVASSERT(isEmpty());

    m_className = metaObject.className();
    m_enumType = enumType;
    m_prefix = stripPrefix;

    AVLogInfo << "initializeEnumTable for class " << m_className << " enum " << enumType;
    QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator(enumType));
    int len = stripPrefix.length();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        QString enumName = metaEnum.key(i);
        if (enumName.left(len) != stripPrefix)
            AVLogFatal << enumName << " does not start with " << stripPrefix;
        QString key = enumName.mid(len);
        int enumValue = metaEnum.value(i);
        insert(key, enumValue);
    }

    AVASSERT(!isEmpty());
}

///////////////////////////////////////////////////////////////////////////////

bool AVEnumTable::isEmpty () const
{
    return m_table.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////

void AVEnumTable::insert(const QString& key, int value)
{
    AVASSERT(!key.isEmpty());
    AVASSERT(value != -1);

    if (m_table.contains(key))
        AVLogFatal << "duplicate key " << key;
    m_table.insert(key, value);
}

///////////////////////////////////////////////////////////////////////////////

int AVEnumTable::find(const QString& key) const
{
    return m_table.value(key, -1);
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVEnumTable::keys() const
{
    QStringList result = m_table.keys();
    result.sort();
    return result;
}

// End of file
