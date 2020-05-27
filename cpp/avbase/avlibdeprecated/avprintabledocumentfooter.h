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

#if !defined AVPRINTABLEDOCUMENTFOOTER_INCLUDED
#define AVPRINTABLEDOCUMENTFOOTER_INCLUDED

// QT includes
#include <qobject.h>

// avlib includes
#include "avmacros.h"
#include "avprintabledocument.h"

// local includes

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! This class represents a footer for a printable document

class AvPrintableDocumentFooter : public QObject
{
    Q_OBJECT

public:
    //! Constructor for class AvPrintableDocumentFooter
    explicit AvPrintableDocumentFooter(AvPrintableDocument::LogoPolicyFooter policy =
            AvPrintableDocument::NoFooterLogo,
            QString filename = "",
            bool printPageNums = false,
            QString footerText = "");

    //! Destructor for class AvPrintableDocumentFooter
    ~AvPrintableDocumentFooter() override;

    void setPrintLogoPolicy(AvPrintableDocument::LogoPolicyFooter policy, QString filename = "");
    void setPrintPageNumbers(bool print) { m_printPageNumbers = print; }
    void setPrintText(QString text) {m_footerText = text; }

    QString getFooterRichText(int pageNumber, QImage &image);

    AvPrintableDocument::LogoPolicyFooter getFooterLogoPolicy() { return m_logoPolicy; }

private:
    AVDISABLECOPY(AvPrintableDocumentFooter);

private:
    bool m_printPageNumbers;
    AvPrintableDocument::LogoPolicyFooter m_logoPolicy;
    QString m_footerText;
    QString m_logoFileName;
};

#endif // AVPRINTABLEDOCUMENTFOOTER_INCLUDED

// End of file
