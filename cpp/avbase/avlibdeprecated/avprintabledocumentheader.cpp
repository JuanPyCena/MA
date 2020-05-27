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
    \brief   This class represents a header for a printable document
 */


// QT includes
#include <qstring.h>
#include <qimage.h>
#include <QPixmap>
#include <QFileInfo>
// avlib includes
#include "avlog.h"
// local includes
#include "avprintabledocumentheader.h"

///////////////////////////////////////////////////////////////////////////////

AvPrintableDocumentHeader::AvPrintableDocumentHeader(AvPrintableDocument::LogoPolicyHeader policy,
        QString filename,
        bool printPageNums,
        QString headerText) :
        m_logoPolicy(policy),
        m_printPageNumbers(printPageNums),
        m_headerText(headerText),
        m_logoFilename(filename)

{
}

///////////////////////////////////////////////////////////////////////////////

AvPrintableDocumentHeader::~AvPrintableDocumentHeader()
{
}

///////////////////////////////////////////////////////////////////////////////

void AvPrintableDocumentHeader::setPrintLogoPolicy(AvPrintableDocument::LogoPolicyHeader policy,
                                                   QString filename)
{
    if(policy != AvPrintableDocument::NoHeaderLogo && filename == "")
    {
        AVLogError << "AvPrintableDocumentHeader::setPrintLogoPolicy: No filename specified"
        << filename;
        m_logoPolicy = AvPrintableDocument::NoHeaderLogo;
    }
    else
    {
        QImage widgetImg(filename);

        if (widgetImg.isNull())
        {
            AVLogError << "AvPrintableDocumentHeader::setPrintLogoPolicy:"
                          " Could not read image from source "
                       << filename;

            m_logoPolicy = AvPrintableDocument::NoHeaderLogo;
            return;
        }

        m_logoPolicy = policy;
        m_logoFilename = filename;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AvPrintableDocumentHeader::getHeaderRichText(int pageNumber, QPixmap &pix)
{
    QString headerRichText = "<table width=100%><tr>";

    AVLogInfo << "AvPrintableDocumentHeader::getHeaderRichText entered";

    if (m_logoPolicy != AvPrintableDocument::NoHeaderLogo)
    {
        QImage widgetImg(m_logoFilename, "png");

        if (widgetImg.isNull())
        {
            AVLogError << "AvPrintableDocumentHeader::getHeaderRichText:"
                          " Could not read image from source "
                       << m_logoFilename;

            m_logoFilename = "";
            m_logoPolicy = AvPrintableDocument::NoHeaderLogo;

            return getHeaderRichText(pageNumber, pix);
        }

        //widgetImg = widgetImg.scaled (120, 60, QImage::ScaleMin);

        AVLogInfo << "Colors: "  << widgetImg.colorCount() << " Depth : " << widgetImg.depth();
        AVLogInfo << "Size: " << widgetImg.size().width() << "/" << widgetImg.size().height();
        AVLogInfo << "Resolution: " << widgetImg.dotsPerMeterX() << "/" << widgetImg.dotsPerMeterY();

        QPixmap pixmap(107,49);
        pixmap.load(m_logoFilename,"png", Qt::AutoColor | Qt::OrderedDither | Qt::DiffuseAlphaDither
                                          | Qt::PreferDither);


        widgetImg.setDotsPerMeterX(widgetImg.dotsPerMeterX() / 4);
        widgetImg.setDotsPerMeterY(widgetImg.dotsPerMeterY() / 4);

        //QMimeSourceFactory::defaultFactory()->setImage("screen", widgetImg);
        pix = pixmap;
        if (m_logoPolicy == AvPrintableDocument::LogoHeaderLeft)
        {
            headerRichText += QString::fromLatin1("<td align=left><img source=\"headerscreen\" width=100% height=100%></td>");
            headerRichText += QString::fromLatin1("<td align=center>%1</td>").arg(m_headerText);

            if (m_printPageNumbers)
                headerRichText += QString::fromLatin1("<td align=right>Page %1</td>").arg(pageNumber);
            else
                headerRichText += "<td align=right></td>";
        }
        else if (m_logoPolicy == AvPrintableDocument::LogoHeaderCentral)
        {
            headerRichText += QString::fromLatin1("<td align=left>%1</td>").arg(m_headerText);
            headerRichText += QString::fromLatin1("<td align=center><img source=\"headerscreen\"></td>");

            if (m_printPageNumbers)
                headerRichText += QString::fromLatin1("<td align=right>Page %1</td>").arg(pageNumber);
            else
                headerRichText += "<td align=right></td>";

        }

        else if (m_logoPolicy == AvPrintableDocument::LogoHeaderRight)
        {
            if (m_printPageNumbers)
                headerRichText += QString::fromLatin1("<td align=left>Page %1</td>").arg(pageNumber);
            else
                headerRichText += "<td align=left></td>";

            headerRichText += QString::fromLatin1("<td align=center>%1</td>").arg(m_headerText);
            headerRichText += QString::fromLatin1("<td align=right><img source=\"headerscreen\"></td>");
        }
    }
    else
    {
        //No logo, but text and page => text left aligned, page right aligned
        if (m_printPageNumbers)
        {
            headerRichText += QString("<td align=left>%1</td>").arg(m_headerText);
            headerRichText += QString("<td align=right>Page %1</td>").arg(pageNumber);
        }
        //No logo, no page => text centered in header
        else
        {
            headerRichText += QString("<td align=center>%1</td>").arg(m_headerText);
        }
    }


    AVLogInfo << "AvPrintableDocumentFooter::getheaderRichText returning";

    headerRichText += "</tr></table>";
    return headerRichText;
}


// End of file
