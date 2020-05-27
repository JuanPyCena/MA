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
    \brief   This class represents a printable document that may be printed using
             AvPrinter. It includes configurable headers and footers which will be shown
             on every page.
 */


// QT includes
#include <qstring.h>

// avlib includes
#include "avlog.h"

// local includes
#include "avprintabledocument.h"
#include "avprintabledocumentheader.h"
#include "avprintabledocumentfooter.h"


///////////////////////////////////////////////////////////////////////////////

AvPrintableDocument::AvPrintableDocument() :
    m_header(0),
    m_footer(0)
{
    initHeaderFooter();
}

///////////////////////////////////////////////////////////////////////////////

AvPrintableDocument::~AvPrintableDocument()
{
    delete m_footer;
    delete m_header;
}

///////////////////////////////////////////////////////////////////////////////

AvPrintableDocument::LogoPolicyFooter AvPrintableDocument::getFooterLogoPolicy()
{
    return m_footer->getFooterLogoPolicy();
}
///////////////////////////////////////////////////////////////////////////////

AvPrintableDocument::LogoPolicyHeader AvPrintableDocument::getHeaderLogoPolicy()
{
    return m_header->getHeaderLogoPolicy();
}

///////////////////////////////////////////////////////////////////////////////

void AvPrintableDocument::initHeaderFooter()
{
    m_footer = new AvPrintableDocumentFooter();
    AVASSERT(m_footer);

    m_header = new AvPrintableDocumentHeader();
    AVASSERT(m_header);

    //Default settings for footer and header
}

void AvPrintableDocument::setHeaderProperties(LogoPolicyHeader policy,
                                              QString logoFile,
                                              QString headerText,bool printPageNumber)
{
    AVASSERT(m_header);
    m_header->setPrintLogoPolicy(policy, logoFile);
    m_header->setPrintText(headerText);
    m_header->setPrintPageNumbers(printPageNumber);
}

void AvPrintableDocument::setFooterProperties(LogoPolicyFooter policy,
                                              QString logoFile, QString footerText,
                                              bool printPageNumber)
{
    AVASSERT(m_footer);
    m_footer->setPrintLogoPolicy(policy, logoFile);
    m_footer->setPrintText(footerText);
    m_footer->setPrintPageNumbers(printPageNumber);
}

QString AvPrintableDocument::getHeaderRichText(int pageNumber, QPixmap &pix)
{
    AVASSERT(m_header);
    return m_header->getHeaderRichText(pageNumber, pix);
}


QString AvPrintableDocument::getFooterRichText(int pageNumber, QImage &image)
{
    AVASSERT(m_footer);
    return m_footer->getFooterRichText(pageNumber, image);
}



// End of file
