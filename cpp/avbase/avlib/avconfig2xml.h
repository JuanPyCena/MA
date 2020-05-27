///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config params stored in XML.
 */

#if !defined AVCONFIG2XML_H_INCLUDED
#define AVCONFIG2XML_H_INCLUDED

#include "avlib_export.h"
#include "avconfig2.h"

#include <QDomElement>


///////////////////////////////////////////////////////////////////////////////
/**
 *  Loads configs stored in XML format.
 */
class AVLIB_EXPORT AVConfig2ImporterXml : public AVConfig2ImporterBase
{
public:
    explicit AVConfig2ImporterXml(const AVConfig2StorageLocation& location);
    ~AVConfig2ImporterXml() override;

    void import() override;

private:

    void readInclude(QDomElement element);
    void readMapping(QDomElement element);
    void readOverride(QDomElement element);
    void readSection(QDomElement section_element, const QString& super_section_name);
    void readParam  (QDomElement param_element, const QString& section_name);

    QString readCdataOrTextElement(QDomElement param_element, const QString& name, bool mandatory);

    //! TODO CM make this fatal only if not -save or -helping or find a different way of error
    //! reporting
    void printError(const QString& reason) const;
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Stores configs in XML format.
 *
 *  Design considerations:
 *  - Everything that potentially contains XML characters (types, help text, parameter values)
 *    is contained in elements (not attributes) and will be wrapped in CDATA section if necessary
 *    for better readability.
 *
 *    TODO CM handle & save back include directives
 */
class AVLIB_EXPORT AVConfig2ExporterXml : public AVConfig2ExporterBase
{
public:

    AVConfig2ExporterXml();
    ~AVConfig2ExporterXml() override {}

    void addParameter(const QString& value, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    void addReference(const QString& ref, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    void addSearchPath(const QString& include, const AVConfig2StorageLocation& location) override;
    void addInclude(const QString& include, const AVConfig2StorageLocation& location) override;
    void addConfigMapping(const QString& map_from, const QString& mapped_dir, const QString& mapped_name,
                          const AVConfig2StorageLocation& location) override;

    void addOverride(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;
    void addDefine(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;
    void addInheritedSection(const QString&, const QString&, const AVConfig2StorageLocation&) override {
    }  // TODO CM review

    void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                       const AVConfig2StorageLocation& location) override;

    QString doExport() override;
    void    reset() override;

private:

    enum {
        INDENT = 3
    };

    //! Helper method because params and refs are stored almost the same way.
    void addParamOrReference(const QString& value, const AVConfig2Metadata& metadata, bool is_ref);

    //! Perhaps creates and returns a section element in the proper place.
    //!
    //! \param full_section The full name of the section.
    //! \return             A (perhaps newly created) element which is correctly nested in the dom
    //!                     document.
    QDomElement getOrCreateSectionElement(const QString& full_section);

    void addParamSubElement(QDomElement param_element, const QString& name, const QString& value);

    //! Depending on whether data contains any special XML characters, appends a text section or a
    //! cdata section to the given parent element.
    //! This done to enhance readability of the resulting XML file (trade off escapes inside the
    //! text vs. the cdata nesting)
    void appendCdataOrTextElement(QDomElement parent, const QString& data);

    typedef QMap<QString, QDomElement> ElementContainer;
    ElementContainer m_params;
    ElementContainer m_sections;
    ElementContainer m_includes;
    ElementContainer m_mappings;
    ElementContainer m_overrides;

    QDomDocument m_document;
    QDomElement  m_root_element;
};

#endif

// End of file
