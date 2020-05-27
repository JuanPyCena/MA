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

#if !defined AVPRINTABLEDOCUMENTHEADER_INCLUDED
#define AVPRINTABLEDOCUMENTHEADER_INCLUDED

// QT includes
#include <qobject.h>

// avlib includes
#include "avmacros.h"
#include "avprintabledocument.h"

// local includes

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Represents a header for an AvPrintableDocument to be printed at the top of each page


class AvPrintableDocumentHeader : public QObject
{
    Q_OBJECT



public:
    //! Constructor for class AvPrintableDocumentHeader
    explicit AvPrintableDocumentHeader(AvPrintableDocument::LogoPolicyHeader policy =
                                       AvPrintableDocument::LogoHeaderCentral,
                                       QString filename = "",
                                       bool printPageNums = false,
                                       QString headerText = "");

    //! Destructor for class AvPrintableDocumentHeader
    ~AvPrintableDocumentHeader() override;

    void setPrintLogoPolicy(AvPrintableDocument::LogoPolicyHeader policy, QString filename = "");
    void setPrintPageNumbers(bool print) { m_printPageNumbers = print; }
    void setPrintText(QString text) {m_headerText = text; }

    QString getHeaderRichText(int pageNumber, QPixmap &pix);


    AvPrintableDocument::LogoPolicyHeader getHeaderLogoPolicy() { return m_logoPolicy; }

private:
    AVDISABLECOPY(AvPrintableDocumentHeader);


protected:
    AvPrintableDocument::LogoPolicyHeader m_logoPolicy;
    bool m_printPageNumbers;
    QString m_headerText;
    QString m_logoFilename;
};

#endif // AVPRINTABLEDOCUMENTHEADER_INCLUDED

// End of file
