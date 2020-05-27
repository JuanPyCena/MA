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



// QT includes
#include <qstring.h>
#include <qprinter.h>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <qimage.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <QHeaderView>
#include <QTreeWidget>
#include <qregexp.h>
#include <QList>
#include <QUrl>
#include <QPrintDialog>
// avlib includes
#include "avprinter.h"
#include "avconfig.h"
#include "avprintabletabledocument.h"
#include "avscreenshot.h"

///////////////////////////////////////////////////////////////////////////////

AvPrinter::AvPrinter(const QString &name,
                     const QDateTime &currentDateTime,
                     const QString &printerName,
                     QWidget* parent) :
  m_parent(parent),
  m_name(name),
  m_dateTime(currentDateTime),
  m_printerName(printerName)
{

}

///////////////////////////////////////////////////////////////////////////////

AvPrinter::~AvPrinter()
{

}

///////////////////////////////////////////////////////////////////////////////

bool AvPrinter::setupPrinter(QPrinter &printer)
{
    // select printer using m_printerName...
    if (!m_printerName.isEmpty())
    {
        printer.setPrinterName(m_printerName);
        return true;
    }
    // ...or by showing printer selection dialog
    QPrintDialog dialog(&printer, m_parent);
    if (dialog.exec() == QDialog::Accepted)
        return true;

    return false;
}

// helper function drawing with painter the document on x, y within clip
// (where clip is in full-document coordinates, offset by +x,+y compared to QTextDocument coordinates)
static void drawTextDocument(QPainter *painter, QTextDocument &document, int x, int y, const QRect &clip)
{
    painter->save();
    painter->translate(x, y);
    QRect clipRect;
    if (clip.isValid()) {
        clipRect = clip.translated(-x, -y);
    }
    document.drawContents(painter, clipRect);
    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////
bool AvPrinter::printWidget(QWidget* widget)
{
    QPrinter printer;

    printer.setFullPage(true);


    QString filename =
        AVEnvironment::getApplicationLog() +
        "/screen" +
        m_dateTime.toString("yyyyMMddhhmmss") +
        m_name;

    printer.setColorMode(QPrinter::Color);

    if (!setupPrinter(printer))
        return false;

    if(!AVScreenShot::doScreenShot(widget, filename, false))
    {
        AVLogError << "AvPrinter::printWidget: Error taking screenshot";
        return false;
    }

    printer.newPage();

    QPainter painter;
    if (!painter.begin(&printer))
    {
        AVLogError << "AvScreenshotPrinter::printWidget: Printing aborted";
        return false;
    }

    const QImage widgetImg = widget->grab().toImage();
    QTextDocument richText;
    richText.addResource(QTextDocument::ImageResource,
                         QUrl("mydata://screen"), QVariant(widgetImg));

    QString htmlTextWithImg = QString(
                "<p align=center>Printed on \"%1\" from \"%2\"<br>(filename: %3)<br>"
                "<br><br><br>"
                "<img src=\"mydata://screen\"><\"p>").arg(m_dateTime.toString("yyyy.MM.dd hh:mm:ss"),
                                                    m_name, filename);

    printRichText(htmlTextWithImg, printer, painter, &richText);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AvPrinter::printAvPrintableDocument(AvPrintableDocument& document)
{
    QPrinter printer;

    printer.setFullPage(true);

    printer.setColorMode(QPrinter::Color);

    if (!setupPrinter(printer))
        return false;

    QPainter painter;
    if (!painter.begin(&printer))
    {
        AVLogError << "AvPrinter::printAvPrintableDocument: Printing aborted";
        return false;
    }

    int dpiy = painter.device()->logicalDpiY();
    int dpix = painter.device()->logicalDpiX();
    int width_margin = (int) ((1 / 2.54) * dpiy); // 1 cm margins
    int height_margin = (int) ((1 / 2.54) * dpix);

    QRect body(width_margin, 3 * height_margin, painter.device()->width() - 2 * width_margin,
               painter.device()->height() - 5 * height_margin);

    if (document.getFooterLogoPolicy() == AvPrintableDocument::NoFooterLogo)
        body.setHeight(body.height() + height_margin);
    if (document.getHeaderLogoPolicy() == AvPrintableDocument::NoHeaderLogo)
        body.setTop(body.top() - height_margin);

    QString rich_text_string = document.getDocumentBody();

    QTextDocument richTextBody;
    richTextBody.setHtml(rich_text_string);
    richTextBody.setPageSize(body.size());

    QRect view(body);

    int page = 1;
    for (;;)
    {
        QTextDocument richTextHeader;
        QPixmap pix;
        const QString htmlHeader = document.getHeaderRichText(page, pix);
        addHeaderPixmap(richTextHeader, pix);
        richTextHeader.setHtml(htmlHeader);
        richTextHeader.setPageSize(QSize(body.width(), body.height() + height_margin));
        drawTextDocument(&painter, richTextHeader, view.left(), view.top() - 3 * height_margin, QRect());

        drawTextDocument(&painter, richTextBody, body.left(), body.top(), view);
        view.translate(0, body.height());
        painter.translate(0, -body.height());

        QTextDocument richTextFooter;
        QImage footerImage;
        const QString htmlFooter = document.getFooterRichText(page, footerImage);
        addFooterImage(richTextFooter, footerImage);

        richTextFooter.setHtml(htmlFooter);
        richTextFooter.setPageSize(QSize(body.width(), body.height() + 2 * height_margin));
        drawTextDocument(&painter, richTextFooter, view.left(), view.bottom(), QRect());

        if (view.top() >= richTextBody.size().height())
            break;

        printer.newPage();
        page++;
    }
    return true;
}

void AvPrinter::addHeaderPixmap(QTextDocument &richTextHeader, const QPixmap &pix)
{
    if (!pix.isNull()) {
        richTextHeader.addResource(QTextDocument::ImageResource,
                                   QUrl(QLatin1String("headerscreen")),
                                   pix);
    }
}

void AvPrinter::addFooterImage(QTextDocument &richTextFooter, const QImage &footerImage)
{
    if (!footerImage.isNull()) {
        richTextFooter.addResource(QTextDocument::ImageResource,
                                   QUrl(QLatin1String("footerscreen")),
                                   footerImage);
    }
}


///////////////////////////////////////////////////////////////////////////////

void AvPrinter::printRichText(QString &htmlTextWithImg,
                                        QPrinter& printer,
                                        QPainter& painter,
                                        QTextDocument *document)

{
    int dpiy = painter.device()->logicalDpiY();
    int dpix = painter.device()->logicalDpiX();
    int width_margin = (int) ((1 / 2.54) * dpiy);  // 1 cm margins
    int height_margin = (int) ((1 / 2.54) * dpix);

    QRect body(width_margin, height_margin,
               painter.device()->width() - 2*width_margin,
               painter.device()->height() - 2*height_margin);

    QTextDocument *richTextDocument = document ? document : new QTextDocument();


    richTextDocument->setHtml(htmlTextWithImg);
    richTextDocument->setPageSize(body.size());

    QRect view(body);

    int page = 1;
    for (;;)
    {
        drawTextDocument(&painter, *richTextDocument, body.left(), body.top(), view);

        view.translate(0, body.height());
        painter.translate(0, -body.height());
        painter.drawText(view.right() - painter.fontMetrics().width(QString("Page ") +
                                                                    QString::number(page)),
                         view.bottom() + painter.fontMetrics().ascent() + 5,
                         QString("Page ") + QString::number(page));

        if (view.top() >= richTextDocument->size().height())
            break;

        printer.newPage();
        page++;
    }
    if (richTextDocument != document) {
        delete richTextDocument;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AvPrinter::printTable(AvPrintableTableDocument& document,
                           int columnsPerPage)
{
    QPrinter printer;

    printer.setFullPage(true);

    printer.setOrientation(QPrinter::Landscape);
    printer.setColorMode(QPrinter::Color);

    //TODO: for now hardcoded, should be configurable!!
    printer.setPrintRange(QPrinter::AllPages);

    if (!setupPrinter(printer))
        return false;

    QPainter painter;
    if (!painter.begin(&printer))
    {
        AVLogError << "AvPrinter::printTable: Printing aborted";
        return false;
    }

    int dpiy = painter.device()->logicalDpiY();
    int dpix = painter.device()->logicalDpiX();
    int width_margin = (int) ((1 / 2.54) * dpiy); // 1 cm margins
    int height_margin = (int) ((1 / 2.54) * dpix);

    QRect body(width_margin,3*height_margin, painter.device()->width() - 2 * width_margin,
               painter.device()->height() - 5 * height_margin);

    if(document.getFooterLogoPolicy() == AvPrintableDocument::NoFooterLogo)
        body.setHeight(body.height() + height_margin);
    if(document.getHeaderLogoPolicy() == AvPrintableDocument::NoHeaderLogo)
        body.setTop(body.top() - height_margin);

    QRect view(body);

    int page = 1;

    QString html_text = "<table width=100% border=0><tr>";

    for (int column = 0; column < document.getTable()->columnCount(); column++)
    {
        html_text += QString("<th><b><i><center>") + document.getTable()->model()->headerData(column, Qt::Horizontal).toString()
                  + QString("</center></i></b></th>");
    }
    html_text += "</tr>";

    html_text += "<tr>";
    for (int column = 0; column < document.getTable()->columnCount(); column++)
    {
        html_text += "<td></td>";
    }
    html_text += "</tr>";

    QTreeWidgetItemIterator it(document.getTable());

    int count = 1;

    while (*it)
    {
        if(count % 2 == 0)
            html_text += "<tr bgcolor=\"lightgray\">";
        else
            html_text += "<tr>";

        for (int column = 0; column < document.getTable()->columnCount(); column++)
        {
            QList<AvPrinterTableColumnStyle::ColumnStyle> columnStyle =
                    document.getColumnStyle(column + 1);

            if(!columnStyle.isEmpty())
                getCellRichTextWithStyle(html_text, (*it)->text(column),  columnStyle,
                                         printer.colorMode() == QPrinter::GrayScale);
            else
                html_text += QString("<td>") + (*it)->text(column) + QString("</td>");
        }

        html_text += "</tr>";

        if (count % columnsPerPage == 0)
        {
            document.setDocumentBody(html_text + "</table>");

            printTablePage(document, painter, body, view, height_margin, page);

            html_text = "<table width=100% border=0><tr>";

            for (int column = 0; column < document.getTable()->columnCount(); column++)
            {
                html_text += QString("<th><b><i><center>") + document.getTable()->model()->headerData(column, Qt::Horizontal).toString()
                        + QString("</i></b></center></th>");
            }

            html_text += "</tr>";

            html_text += "<tr>";
            //Empty line after header in table
            for (int column = 0; column < document.getTable()->columnCount(); column++)
            {
                html_text += "<td></td>";
            }

            html_text += "</tr>";

            page++;
            printer.newPage();
        }

        count++;
        it++;
    }

    document.setDocumentBody(html_text + "</table>");

    printTablePage(document, painter, body, view, height_margin, page);
    painter.end();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void AvPrinter::getCellRichTextWithStyle(QString& html_text,
                                         QString content,
                                         QList<AvPrinterTableColumnStyle::ColumnStyle> style,
                                         bool disableColors)
{
    QString add_prefix = "";
    QString add_postfix = "";

    while (!style.empty())
    {
        AvPrinterTableColumnStyle::ColumnStyle current = style.last();
        style.pop_back();

        if (current.criterion.exactMatch(content))
        {
            if (current.alignment == AvPrinterTableColumnStyle::LeftAligned)
            {
                add_prefix += "<td align=left>";
            }
            else if (current.alignment == AvPrinterTableColumnStyle::CenterAligned)
            {
                add_prefix += "<td align=center>";
            }
            else if (current.alignment == AvPrinterTableColumnStyle::RightAligned)
            {
                add_prefix += "<td align=right>";
            }

            if (current.font_color != "" && !disableColors)
            {
                if (current.font_face != "")
                {
                    add_prefix += "<font face=" + current.font_face + " color=" + current.font_color
                            + ">";
                }
                else
                {
                    add_prefix += "<font color=" + current.font_color + ">";
                }

                add_postfix += "</font>";
            }

            if (current.font_color == "" && current.font_face != "")
            {
                add_prefix += "<font face=" + current.font_face + ">";
                add_postfix += "</font>";
            }

            if (current.bold)
            {
                add_prefix += "<b>";
                add_postfix += "</b>";
            }
            if (current.italic)
            {
                add_prefix += "<i>";
                add_postfix += "</i>";
            }

            add_postfix += "</td>";

            AVLogInfo << "Tags: " << add_prefix << content << add_postfix;

            html_text += (add_prefix + content + add_postfix);

            return;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////

void AvPrinter::printTablePage(AvPrintableDocument& document,
                               QPainter& painter,
                               QRect& body,
                               QRect& view,
                               int height_margin,
                               int page_num)
{
    QTextDocument richTextHeader;
    QPixmap pix;
    const QString htmlRichText = document.getHeaderRichText(page_num, pix);
    addHeaderPixmap(richTextHeader, pix);
    richTextHeader.setHtml(htmlRichText);
    richTextHeader.setPageSize(body.size());
    drawTextDocument(&painter, richTextHeader, view.left(), view.top() - 2*height_margin, QRect());

    QTextDocument richTextBody;
    richTextBody.setHtml(document.getDocumentBody());
    richTextBody.setPageSize(body.size());
    drawTextDocument(&painter, richTextBody, body.left(), body.top(), view);

    QTextDocument richTextFooter;
    QImage image;
    const QString htmlFooterText = document.getFooterRichText(page_num, image);
    addFooterImage(richTextFooter, image);
    richTextFooter.setHtml(htmlFooterText);
    richTextHeader.setPageSize(QSize(body.width(), body.height() + 2*height_margin));
    drawTextDocument(&painter, richTextFooter, view.left(), view.bottom(), QRect());
}

// End of file
