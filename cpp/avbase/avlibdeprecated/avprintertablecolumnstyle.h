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

#if !defined AVPRINTERTABLECOLUMNSTYLE_INCLUDED
#define AVPRINTERTABLECOLUMNSTYLE_INCLUDED

// QT includes
#include <qobject.h>
#include <qmap.h>
#include <qlist.h>
#include <qregexp.h>
#include <QList>

// avlib includes
#include "avmacros.h"

// local includes
#include "avlibdeprecated_export.h"

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Style choices for table columns printed with AvPrinter
/*! Choices are fonttype, fontcolor, bold, italic, alignment etc.
 *  Furthermore, constraints may be specified to only use the style if the table cell
 *  contents of the current row for the column matches a certain criterion
*/

class AVLIBDEPRECATED_EXPORT AvPrinterTableColumnStyle : public QObject
{
    Q_OBJECT

 public:

    enum ColumnVerticalAlignment{
        LeftAligned = 1,
        CenterAligned = 2,
        RightAligned = 3
    };

    struct ColumnStyle
    {
        bool italic;
        bool bold;
        ColumnVerticalAlignment  alignment;
        QRegExp criterion;
        QString font_face;
        QString font_color;
    };

    //! Constructor for class AVPrinterTableColumnStyle
    explicit AvPrinterTableColumnStyle(int numColumns);
    //! Destructor for class AVPrinterTableColumnStyle
    ~AvPrinterTableColumnStyle() override;

    bool setColumnStyle(int column, ColumnStyle style, bool overwritePrevious = false);
    QList<ColumnStyle> getColumnStyle(int column);

private:
    int  m_columnCount;
    QMap<int, QList<ColumnStyle> > m_columnStyle;
};

#endif // AVPRINTERTABLECOLUMNSTYLE_INCLUDED

// End of file
