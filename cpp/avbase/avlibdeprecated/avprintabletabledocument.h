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

#if !defined AVPRINTABLETABLEDOCUMENT_INCLUDED
#define AVPRINTABLETABLEDOCUMENT_INCLUDED

// QT includes
#include <qobject.h>
#include <QList>

// avlib includes
#include "avmacros.h"
#include "avprintabledocument.h"
#include "avprintertablecolumnstyle.h"
#include "avlibdeprecated_export.h"

// local includes

// forward declarations
class QTreeWidget;

///////////////////////////////////////////////////////////////////////////////
//! This class represents a printable table document which may be printed using
//! AvPrinter.
/*! If this class is printed using an AvPrinter, the contents of the list view
 *  associated with the AvPrintableTableDocument are rendered into an HTML table
 *  and printed with the headers and footers of the parent class.
*/

class AVLIBDEPRECATED_EXPORT AvPrintableTableDocument : public AvPrintableDocument
{
    Q_OBJECT

public:
    //! Constructor for class AvPrintableTableDocument
    AvPrintableTableDocument(AvPrinterTableColumnStyle& style, QTreeWidget *table);
    //! Destructor for class AvPrintableTableDocument
    ~AvPrintableTableDocument() override;

    QTreeWidget* getTable() { return m_table; }

    QList<AvPrinterTableColumnStyle::ColumnStyle> getColumnStyle(int column)
            { return m_columnStyles.getColumnStyle(column); }

private:
    AVDISABLECOPY(AvPrintableTableDocument);

    AvPrinterTableColumnStyle& m_columnStyles;
    QTreeWidget* m_table;
};

#endif // AVPRINTABLETABLEDOCUMENT_INCLUDED

// End of file
