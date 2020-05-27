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


#include <memory>

// QT includes
#include <QtGlobal>
#include <QDir>

#include "avconfig.h" // TODO CM for application config, fix this
#include "avfromtostring.h"
#include "avdeprecate.h"


///////////////////////////////////////////////////////////////////////////////

#include "avconfig2xml.h"

#include "avfromtostring.h"

namespace
{

const char *CONFIG_ROOT_TAGNAME    = "avconfig";

const char *MAPPING_TAGNAME        = "map";
const char *MAPPING_FROM_ATTRIBUTE = "from";
const char *MAPPING_TO_ATTRIBUTE   = "to";

const char *INCLUDE_TAGNAME        = "include";
const char *INCLUDE_FILE_ATTRIBUTE = "file";

const char *OVERRIDE_TAGNAME         = "override";
const char *OVERRIDE_NAME_ATTRIBUTE  = "param";
const char *OVERRIDE_VALUE_ATTRIBUTE = "value";

const char *SECTION_TAGNAME        = "section";

const char *PARAM_TAGNAME          = "param";
const char *NAME_ATTRIBUTE         = "name";
const char *VALUE_TAGNAME          = "value";
const char *REF_TAGNAME            = "ref";
const char *SUGGESTED_TAGNAME      = "suggested";
const char *HELP_TAGNAME           = "help";
const char *TYPE_TAGNAME           = "type";
const char *RESTRICTION_TAGNAME    = "restriction";
const char *CMDLINE_TAGNAME        = "cmdline";

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterXml::AVConfig2ImporterXml(const AVConfig2StorageLocation& location) :
        AVConfig2ImporterBase(location)
{
    // Use cstyle config instead
    AVDEPRECATE(AVConfig2ImporterXml);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterXml::~AVConfig2ImporterXml()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::import()
{
    QFile file(m_location.getFullFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        AVLogFatal << "AVConfig2ImporterXml::import: Failed to open " << file.fileName();
        return;
    }

    QDomDocument document;
    if (!document.setContent(&file))
    {
        AVLogError << "AVConfig2ImporterXml::import: Failed to parse XML file "
                   << file.fileName();
        return;
    }

    QDomProcessingInstruction instr = document.firstChild().toProcessingInstruction();
    if (instr.isNull()) return printError("XML processing instruction expected");

    QDomElement root = instr.nextSibling().toElement();
    if (root.isNull() || root.tagName() != CONFIG_ROOT_TAGNAME)
    {
        return printError(QString("Root element ") + CONFIG_ROOT_TAGNAME + " is missing");
    }

    QDomNode node = root.firstChild();
    for (;;)
    {
        QDomElement elem = node.toElement();
        if (!elem.isNull())
        {
            if (elem.tagName() == INCLUDE_TAGNAME)
            {
                readInclude(elem);
            } else if (elem.tagName() == MAPPING_TAGNAME)
            {
                readMapping(elem);
            } else if (elem.tagName() == OVERRIDE_TAGNAME)
            {
                readOverride(elem);
            } else if (elem.tagName() == SECTION_TAGNAME)
            {
                readSection(elem, "");
            } else
            {
                printError(QString("AVConfig2ImporterXml::import: "
                        "Unknown element encountered: ") + elem.tagName());
            }
        }

        if (node.nextSibling().isNull()) break;
        node = node.nextSibling();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::readInclude(QDomElement element)
{
    if (!element.hasAttribute(INCLUDE_FILE_ATTRIBUTE))
    {
        return printError("No file attribute in include directive");
    }
    QString file = element.attribute(INCLUDE_FILE_ATTRIBUTE);

    m_client->addInclude(file, m_location);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::readMapping(QDomElement element)
{
    if (!element.hasAttribute(MAPPING_FROM_ATTRIBUTE) ||
        !element.hasAttribute(MAPPING_TO_ATTRIBUTE))
    {
        return printError("No from/to attribute in mapping");
    }
    QString from = element.attribute(MAPPING_FROM_ATTRIBUTE);
    QString to   = element.attribute(MAPPING_TO_ATTRIBUTE);

    m_client->addConfigMapping(from, QString::null, to, m_location);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::readOverride(QDomElement element)
{
    if (!element.hasAttribute(OVERRIDE_NAME_ATTRIBUTE) ||
        !element.hasAttribute(OVERRIDE_VALUE_ATTRIBUTE))
    {
        return printError("No name/value attribute in override");
    }
    QString name  = element.attribute(OVERRIDE_NAME_ATTRIBUTE);
    QString value = element.attribute(OVERRIDE_VALUE_ATTRIBUTE);

    m_client->addOverride(name, value, m_location);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::readSection(
        QDomElement section_element, const QString& super_section_name)
{
    if (!section_element.hasAttribute(NAME_ATTRIBUTE))
    {
        return printError("No name in section");
    }

    // Concatenate section names
    QString section_name = section_element.attribute(NAME_ATTRIBUTE);
    if (!super_section_name.isEmpty()) section_name = super_section_name + "." + section_name;

    QDomNode node = section_element.firstChild();

    for (;;)
    {
        QDomElement elem = node.toElement();
        if (!elem.isNull())
        {
            if (elem.tagName() == PARAM_TAGNAME)
            {
                readParam(elem, section_name);
            } else if (elem.tagName() == SECTION_TAGNAME)
            {
                readSection(elem, section_name);
            } else
            {
                printError(QString("AVConfig2ImporterXml::readSection (" + section_name + "): "
                                   "Unknown element encountered: ") + elem.tagName());
            }
        }

        if (node.nextSibling().isNull()) break;
        node = node.nextSibling();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::readParam(QDomElement param_element, const QString& section_name)
{
    if (!param_element.hasAttribute(NAME_ATTRIBUTE))
    {
        return printError("No name in param");
    }

    QString value = readCdataOrTextElement(param_element, VALUE_TAGNAME, false);
    QString ref   = readCdataOrTextElement(param_element, REF_TAGNAME, false);

    if ( value.isNull() &&  ref.isNull()) return printError("Neither value nor reference in param");
    if (!value.isNull() && !ref.isNull()) return printError("Both value and reference in param");

    AVConfig2Metadata metadata;
    metadata.m_command_line_switch = readCdataOrTextElement(param_element, CMDLINE_TAGNAME, false);
    metadata.m_default_value      = readCdataOrTextElement(param_element, SUGGESTED_TAGNAME, false);
    metadata.setHelp(                readCdataOrTextElement(param_element, HELP_TAGNAME, true));
    metadata.m_name                = section_name + "." + param_element.attribute(NAME_ATTRIBUTE);
    // TODO CM implement restriction factory
    QString restriction = readCdataOrTextElement(param_element, RESTRICTION_TAGNAME, false);
    if (!restriction.isNull())
    {
        AVLogFatal << "TODO CM implement restriction factory";
    }
    metadata.m_type                = readCdataOrTextElement(param_element, TYPE_TAGNAME, true);

    if (ref.isNull()) m_client->addParameter(value, metadata, m_location);
    else              m_client->addReference(ref, metadata, m_location);
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterXml::readCdataOrTextElement(QDomElement param_element, const QString& name,
                                                     bool mandatory)
{
    QDomNodeList l = param_element.elementsByTagName(name);
    if (l.count() != 1)
    {
        if (l.count() != 0 || mandatory) printError(QString("Need exactly one tag ") + name);
        return QString::null;
    }

    QDomElement name_element = l.item(0).toElement();
    if (name_element.isNull())
    {
        printError(QString("Invalid element ") + name);
        return QString::null;
    }

    QDomNodeList text_children = name_element.childNodes();
    // We never directly can read QString::null from a config file, see escapement with \0.
    if (text_children.length() == 0) return "";
    if (text_children.length() > 1)
    {
        printError(QString("Invalid text in %1").arg(name));
        return QString::null;
    }

    QDomCharacterData data = text_children.item(0).toCharacterData();
    if (data.isNull())
    {
        printError(QString("No character data in ") + name);
        return QString::null;
    }

    return data.data();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterXml::printError(const QString& reason) const
{
    AVLogError << "Error reading " << m_location.getFullFilePath()
               << " : " << reason;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterXml::AVConfig2ExporterXml()
{
    // Use cstyle config instead
    AVDEPRECATE(AVConfig2ExporterXml);

    m_document.appendChild(m_document.createProcessingInstruction(
            "xml", "version='1.0' encoding='utf-8'"));

    // Everything must be stored below a single XML node...
    m_root_element = m_document.createElement(CONFIG_ROOT_TAGNAME);
    m_root_element.setAttribute("version", "1.0");
    m_document.appendChild(m_root_element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addParameter(const QString& value, const AVConfig2Metadata& metadata,
                                        const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);
    addParamOrReference(value, metadata, false);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addReference(const QString& ref, const AVConfig2Metadata& metadata,
                                        const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);
    addParamOrReference(ref, metadata, true);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addSearchPath(
        const QString& path, const AVConfig2StorageLocation& location)
{
    Q_UNUSED(path);
    Q_UNUSED(location);
    AVLogFatal << "Search paths not implemented for XML format";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addInclude(const QString& include,
                                      const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);

    if (m_includes.find(include) != m_includes.end()) return;

    QDomElement element = m_document.createElement(INCLUDE_TAGNAME);
    m_root_element.appendChild(element);
    element.setAttribute(INCLUDE_FILE_ATTRIBUTE, include);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addConfigMapping(const QString& map_from, const QString& mapped_dir,
                                            const QString& mapped_name,
                                            const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);
    Q_UNUSED(mapped_dir); // TODO CM

    ElementContainer::iterator it = m_mappings.find(map_from);

    QDomElement element;
    if (it != m_mappings.end())
    {
        element = it.value();
    } else
    {
        element = m_document.createElement(MAPPING_TAGNAME);
        m_root_element.appendChild(element);
        m_mappings[map_from] = element;
    }

    element.setAttribute(MAPPING_FROM_ATTRIBUTE, map_from);
    element.setAttribute(MAPPING_TO_ATTRIBUTE, mapped_name);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addOverride(const QString& name, const QString& value,
                                       const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);

    ElementContainer::iterator it = m_overrides.find(name);

    QDomElement element;
    if (it != m_overrides.end())
    {
        element = it.value();
    } else
    {
        element = m_document.createElement(OVERRIDE_TAGNAME);
        m_root_element.appendChild(element);
    }

    element.setAttribute(OVERRIDE_NAME_ATTRIBUTE,  name);
    element.setAttribute(OVERRIDE_VALUE_ATTRIBUTE, value);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addDefine(const QString& name, const QString& value,
                                     const AVConfig2StorageLocation& location)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(location);

    AVLogFatal << "Defines are not yet implemented for the XML config format";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                                         const AVConfig2StorageLocation& location)
{
    Q_UNUSED(value);
    Q_UNUSED(metadata);
    Q_UNUSED(location);
    AVLogError << "Subconfig templates are not supported with the XML config format.";
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ExporterXml::doExport()
{
    return m_document.toString(INDENT);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::reset()
{
    m_document = QDomDocument();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addParamOrReference(
        const QString& value, const AVConfig2Metadata& metadata, bool is_ref)
{
    // First create / find the section element to which our param should be added
    QString section;
    QString name = metadata.m_name;
    int sep = name.lastIndexOf(".");
    if (sep != -1)
    {
        section = name.left(sep);
        name = name.mid(sep+1);
    }
    QDomElement section_element = getOrCreateSectionElement(section);

    // Delete any existing element which corresponds to this parameter (=overwrite old information)
    // This information might be here because of preloading.
    ElementContainer::iterator existing_param_it = m_params.find(metadata.m_name);
    if (existing_param_it != m_params.end())
    {
        QDomElement existing_param_element = existing_param_it.value();
        existing_param_element.parentNode().removeChild(existing_param_element);
    }

    // Now create the parameter element and its subelements...
    QDomElement param_element = m_document.createElement(PARAM_TAGNAME);
    section_element.appendChild(param_element);
    m_params[metadata.m_name] = param_element;

    param_element.setAttribute(NAME_ATTRIBUTE, name);

    addParamSubElement(param_element, is_ref ? REF_TAGNAME : VALUE_TAGNAME, value);

    if (!metadata.m_default_value.isNull())
    {
        addParamSubElement(param_element, SUGGESTED_TAGNAME, metadata.m_default_value);
    }
    if (!metadata.getHelp().isEmpty())
    {
        addParamSubElement(param_element, HELP_TAGNAME, metadata.getHelp());
    }

    addParamSubElement(param_element, TYPE_TAGNAME, metadata.m_type);

    if (metadata.m_restriction != 0)
    {
        addParamSubElement(param_element, RESTRICTION_TAGNAME, metadata.m_restriction->toString());
    }
    if (!metadata.m_command_line_switch.isEmpty())
    {
        addParamSubElement(param_element, CMDLINE_TAGNAME, metadata.m_command_line_switch);
    }
}

///////////////////////////////////////////////////////////////////////////////

QDomElement AVConfig2ExporterXml::getOrCreateSectionElement(const QString& full_section)
{
    // Return the section element if we already have it
    ElementContainer::iterator it = m_sections.find(full_section);
    if (it != m_sections.end())
    {
        return it.value();
    }

    // Recursively create any parent sections
    QString section;
    QDomElement parent_element;
    if (full_section.contains('.'))
    {
        parent_element = getOrCreateSectionElement(full_section.section('.', 0, -2));
        section = full_section.section('.', -1, -1);
    } else
    {
        parent_element = m_root_element;
        section = full_section;
    }

    // Now create our section element

    // Use '~' instead of '-' because '-' seems to break the QT XML parser (?)
    QDomComment c1 =
            m_document.createComment(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ");
    QDomComment c2 = m_document.createComment(" ^ " + section + " ^ ");

    QDomElement ret = m_document.createElement(SECTION_TAGNAME);
    ret.setAttribute(NAME_ATTRIBUTE, section);

    parent_element.appendChild(c1);
    parent_element.appendChild(ret);
    parent_element.appendChild(c2);

    m_sections[full_section] = ret;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::addParamSubElement(
        QDomElement param_element, const QString& name, const QString& value)
{
    QDomElement element = m_document.createElement(name);
    appendCdataOrTextElement(element, value);
    param_element.appendChild(element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterXml::appendCdataOrTextElement(QDomElement parent, const QString& data)
{
    QRegExp re("[<>\"]");
    if (data.contains(re))
    {
        QString remaining = data;
        while (!remaining.isEmpty())
        {
            // This sequence has to be split up. Split it after "]]".
            int i = remaining.indexOf("]]>");
            QString current;
            if (i == -1)
            {
                current   = remaining;
                remaining = "";
            } else
            {
                current   = remaining.left(i+2);
                remaining = remaining.mid (i+2);
            }
            parent.appendChild(m_document.createCDATASection(current));
        }
    } else
    {
        parent.appendChild(m_document.createTextNode(data));
    }
}

// End of file
