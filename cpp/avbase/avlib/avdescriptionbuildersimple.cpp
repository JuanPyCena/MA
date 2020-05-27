///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Alexander Terbu, a.terbu@avibit.com
    \author   QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief    This file defines a simple description builder with values only.
*/


// Qt includes
#include <qdatetime.h>

// local includes
#include "avdescriptionbuildersimple.h"
#include "avlog.h"


///////////////////////////////////////////////////////////////////////////////

AVDescriptionBuilderSimple::AVDescriptionBuilderSimple(
    uint cols, uint groupCols, const QString& colorName, bool omitEmptyFields)
    : AVDescriptionBuilder(omitEmptyFields),
    m_cols(cols), m_groupCols(groupCols), m_colorName(colorName)
{
     AVASSERT(cols > 0);
     AVASSERT(groupCols > 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVDescriptionBuilderSimple::finalize()
{
    if (m_list.count() == 0) return;
    uint rows = m_list.count() / (m_cols * m_groupCols);
    if ((m_list.count() % (m_cols * m_groupCols)) > 0) rows++;

    QString result = "<table>";

    for (uint row = 0; row < rows; ++row)
    {
        result += "<tr>";
        for (uint col = 0; col < m_cols; ++col)
        {
            for (uint groupCol = 0; groupCol < m_groupCols; ++groupCol)
            {
                int idx = (col * rows + row) * m_groupCols + groupCol;
                if (idx < m_list.count())
                    result += m_list[idx];
                else
                    result += "<td></td>";
            }
            result += "<td></td>";
        }
        result += "</tr>";
    }
    result += "</table>";

    m_description = result;
}

///////////////////////////////////////////////////////////////////////////////

void AVDescriptionBuilderSimple::addValue(const QString& label, const QString& value)
{
    Q_UNUSED(label);
    QString valueWithoutTags = value;
    valueWithoutTags.replace("&", "&amp;");
    valueWithoutTags.replace("<", "&lt;");
    valueWithoutTags.replace(" ", "&nbsp;");
    QString str;
    if (m_colorName.isEmpty())
        str = QString("<td><nobr>%1</nobr></td>").arg(valueWithoutTags);
    else
        str = QString("<td bgcolor=\"%1\"><nobr>%2</nobr></td>")
            .arg(m_colorName).arg(valueWithoutTags);
    m_list += str;
}

// End of file
