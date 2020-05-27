///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT4-Equivalent:  avconfig2restrictions.cpp
// QT4-Approach: port
// QT4-Progress: finished
// QT4-Tested: partial
// QT4-Problems:
// QT4-Comment:
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle restrictions on config parameters.
 */


#include "avmisc.h"

#include "avconfig2restrictions.h"


///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2RestrictionBase::VALUE_LIST_RESTRICTION_PREFIX      = "Valid values";
const QString AVConfig2RestrictionBase::CONTAINER_VALUE_RESTRICTION_PREFIX =
        "Valid container values";
const QString AVConfig2RestrictionBase::RANGE_RESTRICTION_PREFIX           = "Range";
const QString AVConfig2RestrictionBase::REGEX_RESTRICTION_PREFIX           = "Matches Regex";

///////////////////////////////////////////////////////////////////////////////

AVConfig2UntypedRestriction::AVConfig2UntypedRestriction(const QString& restriction_string) :
        m_restriction_string(restriction_string)
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2UntypedRestriction::~AVConfig2UntypedRestriction()
{

}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2UntypedRestriction::operator==(const AVConfig2RestrictionBase& other) const
{
    const AVConfig2UntypedRestriction *other_typed =
            dynamic_cast<const AVConfig2UntypedRestriction*>(&other);
    if (other_typed == 0) return false;

    return m_restriction_string == other_typed->m_restriction_string;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2RestrictionBase *AVConfig2UntypedRestriction::clone() const
{
    return new (LOG_HERE) AVConfig2UntypedRestriction(m_restriction_string);
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2UntypedRestriction::errorMessage(const QString& param, QString value) const
{
    Q_UNUSED(param);
    Q_UNUSED(value);
    AVLogFatal << "Unexpected call to AVConfig2UntypedRestriction::errorMessage()";
    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2UntypedRestriction::toString() const
{
    return m_restriction_string;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2UntypedRestriction::fromString(const QString &str)
{
    m_restriction_string = str;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const AVConfig2RestrictionBase& arg, bool enable_escape)
{
    Q_UNUSED(enable_escape);
    return arg.toString();
}

// End of file
