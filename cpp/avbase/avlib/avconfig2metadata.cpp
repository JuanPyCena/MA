///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
 */

#include "avconfig2metadata.h"

#include "avconfig2restrictions.h"

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata::AVConfig2Metadata() :
        m_optional(false)
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata::AVConfig2Metadata(const QString& name) :
        m_name(name),
        m_optional(false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata::AVConfig2Metadata(
        const QString& name, const QString& type, const QString& help) :
        m_name(name),
        m_type(type),
        m_optional(false),
        m_help(help.simplified())
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata::AVConfig2Metadata(const AVConfig2Metadata& other) :
    m_name               (other.m_name),
    m_type               (other.m_type),
    m_default_value      (other.m_default_value),
    m_suggested_reference(other.m_suggested_reference),
    m_optional           (other.m_optional),
    m_command_line_switch(other.m_command_line_switch),
    m_user_comment       (other.m_user_comment),
    m_restriction        (other.m_restriction != 0 ? other.m_restriction->clone() : 0),
    m_help               (other.m_help)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata::~AVConfig2Metadata()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata& AVConfig2Metadata::operator=(const AVConfig2Metadata& other)
{
    if (this == &other) return *this;

    m_name                = other.m_name;
    m_type                = other.m_type;
    m_help                = other.m_help;
    m_default_value       = other.m_default_value;
    m_suggested_reference = other.m_suggested_reference;
    m_optional            = other.m_optional;
    m_command_line_switch = other.m_command_line_switch;
    m_user_comment        = other.m_user_comment;
    m_restriction.reset(other.m_restriction != nullptr ? other.m_restriction->clone() : nullptr);

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Metadata::operator==(const AVConfig2Metadata &other) const
{
    if (m_name                != other.m_name)                 return false;
    if (m_type                != other.m_type)                 return false;
    if (m_default_value       != other.m_default_value)        return false;
    if (m_suggested_reference != other.m_suggested_reference)  return false;
    if (m_optional            != other.m_optional)             return false;
    if (m_command_line_switch != other.m_command_line_switch ) return false;
    if (m_user_comment        != other.m_user_comment)         return false;
    if ((m_restriction == 0)  != (other.m_restriction == 0))   return false;
    if (m_restriction && (*m_restriction != *other.m_restriction)) return false;
    if (m_help                != other.m_help)                 return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Metadata::isEquivalent(const AVConfig2Metadata& other, bool strict) const
{
    if (m_type != other.m_type) return false;
    if (m_restriction != 0 && other.m_restriction != 0)
    {
        if (m_restriction->toString() != other.m_restriction->toString()) return false;
    } else
    {
        if ((m_restriction == 0) != (other.m_restriction == 0)) return false;
    }

    if (strict)
    {
        // Don't compare the name: it's our key, and for deprecated parameters it won't match.
        // if (m_name                != other.m_name)                return false;
        if (m_help                != other.m_help)                return false;
        if (m_command_line_switch != other.m_command_line_switch) return false;
        if (m_optional            != other.m_optional)            return false;
        if (m_suggested_reference != other.m_suggested_reference )return false;

        // Compatibility issue between old and new config: The old representation of bool is "0" and "1", the new is
        // "true" and "false". Workaround goes below.
        // TODO CM this is obsolete as metadata checks are disabled for the old config anyway now...
        QString compared_default_value[2];
        compared_default_value[0] = m_default_value;
        compared_default_value[1] = other.m_default_value;
        if (m_type == AVConfig2Types::getTypeName<bool>())
        {
            for (uint i=0; i<2; ++i)
            {
                compared_default_value[i].replace("0", "false");
                compared_default_value[i].replace("1", "true");
            }
        }
        if (compared_default_value[0] != compared_default_value[1]) return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Metadata::setHelp(const QString& help)
{
    m_help = help.simplified();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2Metadata::getHelp() const
{
    return m_help;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Metadata::isIncomplete() const
{
    return m_type.isNull();
}

///////////////////////////////////////////////////////////////////////////////

template<>
QString AVToString(const AVConfig2Metadata& arg, bool enable_escape)
{
    if (arg.isIncomplete())
    {
        return "\nName:" + arg.m_name + ", INCOMPLETE (specified on command line or in dev override)";
    }

    QString ret = QString("\nName: %1\nType: %2\nOptional: %3\nHelp: %4").
        arg(arg.m_name).arg(arg.m_type).arg(arg.m_optional).arg(arg.getHelp());

    if (!arg.m_default_value.isNull())
    {
        ret += "\nDefault value: " + arg.m_default_value;
    }
    if (!arg.m_suggested_reference.isNull())
    {
        ret += "\nSuggested reference: " + arg.m_suggested_reference;
    }
    if (!arg.m_command_line_switch.isNull())
    {
        ret += "\nCmdline switch: " + arg.m_command_line_switch;
    }

    if (arg.m_restriction != 0)
    {
        ret += "\n" + AVToString(*arg.m_restriction, enable_escape);
    }
    return ret;
}

// End of file
