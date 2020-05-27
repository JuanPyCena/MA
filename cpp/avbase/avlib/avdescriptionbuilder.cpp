///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Alexander Terbu, a.terbu@avibit.com
    \author   Stefan Kunz, skunz@avibit.com
    \author   Kevin Krammer, k.krammer@avibit.com
    \author   QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief    This file defines the description builder.
*/


// Qt includes
#include <QDateTime>

// local includes
#include "avdescriptionbuilder.h"
#include "avlog.h"


///////////////////////////////////////////////////////////////////////////////

AVDescriptionBuilder::AVDescriptionBuilder(bool omitEmptyFields)
    : m_omitEmptyFields(omitEmptyFields)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVDescriptionBuilder::describe(const QString& label, const QString& value)
{
    if (m_omitEmptyFields && value.isEmpty()) return;
    addValue(label, value);
}

///////////////////////////////////////////////////////////////////////////////

void AVDescriptionBuilder::describe(const QString& label, const QDateTime& value)
{
    if (m_omitEmptyFields && value.isNull()) return;
    QString text;
    if (value.isNull())         text = "(null)";
    else if (! value.isValid()) text = "(invalid)";
    else                        text = value.toString(Qt::ISODate);
    describe(label, text);
}

///////////////////////////////////////////////////////////////////////////////

void AVDescriptionBuilder::describe(const QString& label, const QTime& value)
{
    Q_UNUSED(m_omitEmptyFields);
    QString text;
    if (! value.isValid()) text = "(invalid)";
    else                   text = value.toString(Qt::ISODate);
    describe(label, text);
}

///////////////////////////////////////////////////////////////////////////////

void AVDescriptionBuilder::describe(const QString& label, bool value)
{
    QString text = value ? "true" : "false";
    describe(label, text);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVHtmlTableDescriptionBuilder::AVHtmlTableDescriptionBuilder(
    uint cols, const QString& colorName, bool omitEmptyFields)
    : AVDescriptionBuilder(omitEmptyFields),
      m_cols(cols), m_colorName(colorName)
{
    AVASSERT(cols > 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVHtmlTableDescriptionBuilder::finalize()
{
    if (m_list.count() == 0) return;
    QString result = "<table>";

    QStringList::const_iterator it    = m_list.begin();
    QStringList::const_iterator endIt = m_list.end();
    for (uint row = 0; it != endIt; ++row)
    {
        result += trTag(row);
        for (uint col = 0; col < m_cols; ++col)
        {
            if (it != endIt)
            {
                result += *it;
                ++it;
            }
            else
                result += "<td></td><td></td>";
        }
        result += "</tr>";
    }
    result += "</table>";

    m_description = result;
}

///////////////////////////////////////////////////////////////////////////////

QString AVHtmlTableDescriptionBuilder::trTag(uint row)
{
    if (m_colorName.isEmpty() || row % 2 == 1) return "<tr>";
    return QString("<tr bgcolor=\"%1\">").arg(m_colorName);
}

///////////////////////////////////////////////////////////////////////////////

void AVHtmlTableDescriptionBuilder::addValue(const QString& label, const QString& value)
{
    QString valueWithoutTags = value;
    valueWithoutTags.replace("&", "&amp;");
    valueWithoutTags.replace("<", "&lt;");
    m_list += QString("<td align=right><nobr><b>%1:</b></nobr></td><td>%2</td>")
        .arg(label, valueWithoutTags);
}

// End of file
