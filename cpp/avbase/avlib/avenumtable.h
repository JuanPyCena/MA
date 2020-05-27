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
    \brief    This file declares AVEnumTable, which is used to
              map enum names to enum values. It uses the Qt metaobject
              facility to resolve the enum names.
*/

#ifndef AVENUMTABLE_H
#define AVENUMTABLE_H

// Qt includes
#include <QHash>
#include <QMetaObject>
#include <QStringList>

#include "avlib_export.h"

///////////////////////////////////////////////////////////////////////////////

//! AVEnumTable is used to map enum names to enum values.
//! It uses the Qt metaobject facility to resolve the enum names.
//! If you just want easy conversion between enums and strings, check out the DEFINE_ENUM macro in avmacros.h

class AVLIB_EXPORT AVEnumTable
{
public:
    AVEnumTable() { }
    virtual ~AVEnumTable() { }
    void initialize(const QMetaObject& metaObject, const char *enumType, const QString& stripPrefix);
    bool isEmpty () const;
    void insert(const QString& key, int value);
    int find(const QString& key) const;
    QStringList keys() const;
private:
    //! Copy-constructor: defined but not implemented
    AVEnumTable(const AVEnumTable& rhs);
    //! Assignment operator: defined but not implemented
    AVEnumTable& operator=(const AVEnumTable& rhs);
private:
    //! keys are enum names, values are enum values
    QHash<QString, int> m_table;
    QString    m_className;
    QString    m_enumType;
    QString    m_prefix;
};

#endif // AVENUMTABLE_H

// End of file
