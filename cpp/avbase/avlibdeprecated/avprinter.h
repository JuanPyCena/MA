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
    \brief   This class may be used to print different widget types on a
             printer specified by the enduser
 */

#if !defined AVPRINTER_INCLUDED
#define AVPRINTER_INCLUDED

// QT includes
#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <QList>
#include <QTextDocument>

// avlib includes
#include "avmacros.h"
#include "avprintertablecolumnstyle.h"

// Local includes
#include "avlibdeprecated_export.h"

// forward declarations
class QWidget;
class Q3StyleSheet;
class QPrinter;
class QPainter;
class QRect;
class QColorGroup;
class AvPrintableDocument;
class AvPrintableTableDocument;
///////////////////////////////////////////////////////////////////////////////
//! Prints different widget types
/*! This class provides methods to print different kinds of elements
 *  on a printer specified by the enduser.
 *
*/

class AVLIBDEPRECATED_EXPORT AvPrinter : public QObject
{
    Q_OBJECT

public:
    //! Constructor for class AvPrinter
    //! \param name  The person who is sending the print order
    //! \param currentDateTime The (local) current date and time
    //! \param printerName Name of the preferred printer
    //! \param parent   The parent widget
    AvPrinter(const QString &name,
              const QDateTime &currentDateTime,
              const QString &printerName = "",
              QWidget* parent = 0);
    //! Destructor for class AvPrinter
    ~AvPrinter() override;

    //! Takes and prints a screenshot taken of a QWidget
    //! \param widget The widget to print out
    bool printWidget(QWidget* widget);


    //! Takes an AvPrintableDocument as input and prints it
    //! \param document The document to print (its body rich text is rendered and printed)
    bool printAvPrintableDocument(AvPrintableDocument& document);

    //! Takes an AvPrintableTableDocument containg a list view and prints it by
    //! rendering it into an HTML table using QSimpleRichText.
    //!
    //! \param document The document containing the list view
    //! \param columnsPerPage How many rows of the table should be printed per page
    //! \param enablePrintToFile Determines if it should be possible to print to a ps file
    bool printTable(AvPrintableTableDocument& document,
                    int columnsPerPage);

private:

    //! auto-select printer based on m_printerName or open setup dialog
    bool setupPrinter(QPrinter &printer);

    void printRichText(QString &htmlTextWithImg,
                       QPrinter& printer,
                       QPainter& painter, QTextDocument *document = nullptr);

    void printTablePage(AvPrintableDocument& document, QPainter& painter,
                        QRect& body, QRect& view, int height_margin, int page_num);

    void getCellRichTextWithStyle(QString& html_text,
                                  QString content,
                                  QList<AvPrinterTableColumnStyle::ColumnStyle> style,
                                  bool disableColors);

private:
    AVDISABLECOPY(AvPrinter);

    QWidget*  m_parent;
    QString   m_name;
    QDateTime m_dateTime;
    QString   m_printerName;
    void addHeaderPixmap(QTextDocument &richTextHeader, const QPixmap &pix);
    void addFooterImage(QTextDocument &richTextFooter, const QImage &footerImage);
};

#endif // AVPRINTER_INCLUDED

// End of file
