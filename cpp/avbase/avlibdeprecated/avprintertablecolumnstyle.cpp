///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT5
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVLIBDEPRECATED - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christoph Bichler, c.bichler@avibit.com
    \author  QT4-PORT: Karlheinz Wohlmuth, k.wohlmuth@avibit.com
    \brief   This class encapsulates style choices for table columns to print with AvPrinter::
             printTable()
 */


// QT includes
#include <qstring.h>
#include <QList>

// avlib includes
#include "avlog.h"

// local includes
#include "avprintertablecolumnstyle.h"


///////////////////////////////////////////////////////////////////////////////

AvPrinterTableColumnStyle::AvPrinterTableColumnStyle(int numColumns) :
        m_columnCount(numColumns)
{
    ColumnStyle default_settings = {false , false, CenterAligned, QRegExp(".*"), "", ""};

    //set default settings
    for(int i = 1; i <= m_columnCount; i++)
    {
        QList<ColumnStyle> list;
        list.push_back(default_settings);
        m_columnStyle.insert(i, list);
    }

}

///////////////////////////////////////////////////////////////////////////////

AvPrinterTableColumnStyle::~AvPrinterTableColumnStyle()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AvPrinterTableColumnStyle::setColumnStyle(int column,
                                               ColumnStyle style,
                                               bool overwritePrevious)
{
    if(column > m_columnCount)
    {
        AVLogError << "AVPrinterTableColumnStyle::setColumnStyle: Column index "
                   << column << " not within range";
        return false;
    }

    if(overwritePrevious)
       m_columnStyle.remove(column);

    if(m_columnStyle.contains(column))
    {
        QList<ColumnStyle>& list = m_columnStyle[column];
        list.push_back(style);

        //m_columnStyle.insert(column, list);
    }
    else
    {
        QList<ColumnStyle> list;
        list.push_back(style);
        m_columnStyle.insert(column, list);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QList<AvPrinterTableColumnStyle::ColumnStyle> AvPrinterTableColumnStyle::
    getColumnStyle(int column)
{
    if(column > m_columnCount)
    {
        AVLogError << "AVPrinterTableColumnStyle::setColumnStyle: Column index "
                   << column << " not within range";
        return QList<ColumnStyle>();
    }

    else
        return m_columnStyle[column];

}
// End of file
