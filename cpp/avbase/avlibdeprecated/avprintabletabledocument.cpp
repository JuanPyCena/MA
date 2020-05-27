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
    \brief   This class represents a printable table document which may be printed using
             AvPrinter.
 */


// QT includes
#include <qstring.h>
#include <QTreeWidget>

// avlib includes

// local includes
#include "avprintabletabledocument.h"


///////////////////////////////////////////////////////////////////////////////

AvPrintableTableDocument::AvPrintableTableDocument(AvPrinterTableColumnStyle& style,
                                                   QTreeWidget* table) :
   m_columnStyles(style),
   m_table(table)
{
}

///////////////////////////////////////////////////////////////////////////////

AvPrintableTableDocument::~AvPrintableTableDocument()
{
}

// End of file
