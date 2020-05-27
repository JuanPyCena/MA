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
    \brief   This class represents a footer for a printable document
 */


// QT includes
#include <qstring.h>
#include <qimage.h>
// avlib includes
#include "avlog.h"
// local includes
#include "avprintabledocumentfooter.h"


///////////////////////////////////////////////////////////////////////////////

AvPrintableDocumentFooter::AvPrintableDocumentFooter(AvPrintableDocument::LogoPolicyFooter policy,
        QString filename,
        bool printPageNums,
        QString footerText)
       : m_printPageNumbers(printPageNums),
         m_logoPolicy(policy),
         m_footerText(footerText),
         m_logoFileName(filename)

{
}

///////////////////////////////////////////////////////////////////////////////

AvPrintableDocumentFooter::~AvPrintableDocumentFooter()
{
}

///////////////////////////////////////////////////////////////////////////////

void AvPrintableDocumentFooter::setPrintLogoPolicy(AvPrintableDocument::LogoPolicyFooter policy,
                                                   QString filename)
{
    if(policy != AvPrintableDocument::NoFooterLogo && filename == "")
    {
        AVLogError << "AvPrintableDocumentFooter::setPrintLogoPolicy: No filename specified"
        << filename;

        m_logoPolicy = AvPrintableDocument::NoFooterLogo;
    }
    else
    {
        QImage widgetImg(filename);

        if (widgetImg.isNull())
        {
            AVLogError << "AvPrintableDocumentFooter::setPrintLogoPolicy:"
                          " Could not read image from source "
                       << filename;
            m_logoPolicy = AvPrintableDocument::NoFooterLogo;

            return;
        }

        m_logoPolicy = policy;
        m_logoFileName = filename;
    }
}
///////////////////////////////////////////////////////////////////////////////

QString AvPrintableDocumentFooter::getFooterRichText(int pageNumber, QImage &image)
{
    QString footerRichText = "<table width=100%><tr>";

    AVLogInfo << "AvPrintableDocumentFooter::getFooterRichText entered";

    if (m_logoPolicy != AvPrintableDocument::NoFooterLogo)
    {
        QImage widgetImg(m_logoFileName,"png");

        if (widgetImg.isNull())
        {
            AVLogError << "AvPrintableDocumentFooter::getFooterRichText:"
                          " Could not read image from source "
                       << m_logoFileName;

            m_logoFileName = "";
            m_logoPolicy = AvPrintableDocument::NoFooterLogo;

            return getFooterRichText(pageNumber, image);
        }

        //widgetImg = widgetImg.scaled (120, 60, QImage::ScaleMin);

        image = widgetImg;
        if (m_logoPolicy == AvPrintableDocument::LogoFooterLeft)
        {
            footerRichText += QString::fromLatin1("<td align=left><img source=\"footerscreen\" width=100% height=100%></td>");
            footerRichText += QString::fromLatin1("<td align=center>%1</td>").arg(m_footerText);
        }
        else if (m_logoPolicy == AvPrintableDocument::LogoFooterCenter)
        {
            footerRichText += QString::fromLatin1("<td align=left>%1</td>").arg(m_footerText);
            footerRichText += QString::fromLatin1("<td align=center><img source=\"footerscreen\"></td>");
        }
    }
    else
    {
        footerRichText += QString::fromLatin1("<td align=left>%1</td>").arg(m_footerText);
    }

    if(m_printPageNumbers)
        footerRichText += QString::fromLatin1("<td align=right>Page %1</td>").arg(pageNumber);

    AVLogInfo << "AvPrintableDocumentFooter::getFooterRichText returning";

    footerRichText += "</tr></table>";
    return footerRichText;
}


// End of file
