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

#if !defined AVPRINTABLEDOCUMENT_INCLUDED
#define AVPRINTABLEDOCUMENT_INCLUDED

// QT includes
#include <qobject.h>

// avlib includes
#include "avmacros.h"

// local includes
#include "avlibdeprecated_export.h"

// forward declarations
class AvPrintableDocumentHeader;
class AvPrintableDocumentFooter;

///////////////////////////////////////////////////////////////////////////////
//! This class represents a printable document that may be printed using AvPrinter.
//! If an instance of this class is printed using an AvPrinter, the body (m_body) of this document
//! is rendered using the QStyleSheet class.For example, if the body contains "<em>test</em>"
//! then the string test is printed in italic. For more information about possible tags
//! see documentation of QStyleSheet.
//! Every page of the document is printed with the associated header and footer.

class AVLIBDEPRECATED_EXPORT AvPrintableDocument : public QObject
{
    Q_OBJECT

public:

    enum LogoPolicyFooter{
         LogoFooterLeft = 0,
         LogoFooterCenter = 1,
         NoFooterLogo = 2
    };

    enum LogoPolicyHeader{
         LogoHeaderLeft = 0,
         LogoHeaderCentral = 1,
         LogoHeaderRight = 2,
         NoHeaderLogo = 3
    };

    //! Constructor for class AvPrintableDocument
    AvPrintableDocument();
    //! Destructor for class AvPrintableDocument
    ~AvPrintableDocument() override;

    /**!
     * Sets the properties of the header of the document.
     * \param policy     The policy of displaying the AviBit logo in the header
     * \param headerText The text to display in the header. If the logo is displayed on the
     *                   left, then the text is displayed on the right and vice versa.
     *                   If no logo is displayed, the text is displayed in the middle
     * \param printPageNumber Defines if the page numbers should be displayed in the header
     */
    void setHeaderProperties(LogoPolicyHeader policy,
                             QString logoFile,
                             QString headerText = "",
                             bool printPageNumber = false);
    /**!
     * Sets the properties of the footer of the document.
     * \param policy     The policy of displaying the AviBit logo in the footer
     * \param footerText The text to display in the footer. If the logo is displayed on the
     *                   left, then the text is display in the middle and vice versa.
     * \param printPageNumber Defines if the page numbers should be displayed in the footer
     */
    void setFooterProperties(LogoPolicyFooter policy,
                             QString logoFile,
                             QString footerText = "",
                             bool printPageNumber = true);

    QString getHeaderRichText(int pageNumber, QPixmap &pix);
    QString getFooterRichText(int pageNumber, QImage &image);

    LogoPolicyFooter getFooterLogoPolicy();
    LogoPolicyHeader getHeaderLogoPolicy();

    QString getDocumentBody() { return m_body; }
    void setDocumentBody(QString new_text) { m_body = new_text; }


private:
    AVDISABLECOPY(AvPrintableDocument);

    void initHeaderFooter();

    QString                    m_body;
    AvPrintableDocumentHeader* m_header;
    AvPrintableDocumentFooter* m_footer;
};

#endif // AVPRINTABLEDOCUMENT_INCLUDED

// End of file
