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
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
 */

#include "avconfig2interfaces.h"

#include <memory>

#include "avconfig2classic.h"
#include "avconfig2cstyle.h"
#include "avconfig2xml.h"
#include "avlog.h"
#include "avenvstring.h"

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedElement::LoadedElement(const AVConfig2StorageLocation &location) :
    m_location(location)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedElement::setLocation(const QString &loc, const QString &name, AVConfig2Format format)
{
    m_location.m_location = loc;
    m_location.m_name     = name;
    m_location.m_format   = format;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2StorageLocation &AVConfig2ImporterClient::LoadedElement::getLocation() const
{
    return m_location;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation &AVConfig2ImporterClient::LoadedElement::getLocationForModification()
{
    return m_location;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedParameter::LoadedParameter()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedParameter::LoadedParameter(const QString& value, const AVConfig2Metadata& metadata,
        const AVConfig2StorageLocation& location) :
        LoadedElement(location),
        m_value(value),
        m_original_value(value),
        m_metadata(metadata)
{
    if (location.m_source == AVConfig2StorageLocation::PS_UNKNOWN)
    {
        AVLogFatal << "AVConfig2ImporterClient::LoadedParameter::LoadedParameter: unknown parameter source for " << metadata.m_name
                   << " (" << value << ")";
    }
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedParameter::operator==(const LoadedElement& o) const
{
    if (o.isLoadedParameter())
    {
        const LoadedParameter& other = static_cast<const LoadedParameter&>(o);
        return m_value          == other.m_value &&
               m_original_value == other.m_original_value &&
               m_metadata       == other.m_metadata &&
               getLocation()    == other.getLocation();
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedParameter::setOverrideValue(const QString &value, AVConfig2StorageLocation::ParameterSource source)
{
    m_original_value = m_value;
    m_value = value;
    m_location.m_source = source;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedParameter::getName() const
{
    return m_metadata.m_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedParameter::setName(const QString &name)
{
    m_metadata.m_name = name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedParameter::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addParameter(m_value, m_metadata, m_location);
}

///////////////////////////////////////////////////////////////////////////////

const QString AVConfig2ImporterClient::LoadedParameter::getValue() const
{
    return m_value;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedParameter::setValue(const QString &value)
{
    m_value = value;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2Metadata &AVConfig2ImporterClient::LoadedParameter::getMetadata() const
{
    return m_metadata;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedParameter::updateMetadata(const AVConfig2Metadata &metadata)
{
    // don't allow renaming, as advertised
    QString original_name = m_metadata.m_name;
    m_metadata = metadata;
    m_metadata.m_name = original_name;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedParameter::getOriginalValue() const
{
    return m_original_value;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedParameter::setOriginalValue(const QString &value)
{
    m_original_value = value;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedReference::LoadedReference()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedReference::LoadedReference(
        const QString& referenced_param, const AVConfig2Metadata& metadata,
        const AVConfig2StorageLocation& location) :
        LoadedElement(location),
        m_referenced_param(referenced_param),
        m_metadata(metadata)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedReference::operator==(const LoadedElement& o) const
{
    const LoadedReference *other = dynamic_cast<const LoadedReference*>(&o);
    if (other == nullptr) return false;

    return m_referenced_param == other->m_referenced_param &&
           m_metadata         == other->m_metadata &&
           getLocation()      == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedReference::getName() const
{
    return m_metadata.m_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedReference::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addReference(m_referenced_param, m_metadata, m_location);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedSearchPath::LoadedSearchPath()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedSearchPath::LoadedSearchPath(
        const QString& path, const AVConfig2StorageLocation& location) :
            LoadedElement(location),
            m_path(path)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedSearchPath::operator==(const LoadedElement& o) const
{
    const LoadedSearchPath *other = dynamic_cast<const LoadedSearchPath*>(&o);
    if (other == nullptr) return false;

    return m_path == other->m_path && getLocation() == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedSearchPath::getName() const
{
    // using relative paths here would be ambiguous...
    return expandPath();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedSearchPath::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addSearchPath(m_path, m_location);
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedSearchPath::expandPath() const
{
    QString ret = AVEnvString(m_path).expandEnvString();
    if (QDir::isRelativePath(ret)) ret = getLocation().getFullDirPath() + "/" + ret;
    ret = QDir(ret).absolutePath(); // get rid of "../" ambiguities
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedSearchPath::getPath() const
{
    return m_path;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedInclude::LoadedInclude()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedInclude::LoadedInclude(
        const QString& include, const AVConfig2StorageLocation& location) :
        LoadedElement(location),
        m_include(include)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedInclude::operator==(const LoadedElement& o) const
{
    const LoadedInclude *other = dynamic_cast<const LoadedInclude*>(&o);
    if (other == nullptr) return false;

    return m_include == other->m_include && getLocation() == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedInclude::getName() const
{
    return m_include;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedInclude::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addInclude(m_include, m_location);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedMapping::LoadedMapping()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedMapping::LoadedMapping(
        const QString& from, const QString& mapped_dir, const QString& mapped_name,
        const AVConfig2StorageLocation& location) :
        LoadedElement(location),
        m_from(from), m_mapped_dir(mapped_dir), m_mapped_name(mapped_name)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedMapping::operator==(const LoadedElement& o) const
{
    const LoadedMapping *other = dynamic_cast<const LoadedMapping*>(&o);
    if (other == nullptr) return false;

    return m_from == other->m_from && m_mapped_dir == other->m_mapped_dir && m_mapped_name == other->m_mapped_name &&
            getLocation() == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedMapping::getName() const
{
    return m_from;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedMapping::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addConfigMapping(m_from, m_mapped_dir, m_mapped_name, m_location);

}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedMapping::getFullMapping() const
{
    QString ret;
    if (!m_mapped_dir.isEmpty())
    {
        ret = m_mapped_dir;
        if (ret.right(1) != "/") ret += "/";
    }
    if (m_mapped_name.isNull()) ret += m_from;
    else                        ret += m_mapped_name;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedOverride::LoadedOverride()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedOverride::LoadedOverride(
        const QString& name, const QString& value, const AVConfig2StorageLocation& location) :
        LoadedElement(location),
        m_name(name), m_value(value)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedOverride::operator==(const LoadedElement& o) const
{
    const LoadedOverride *other = dynamic_cast<const LoadedOverride*>(&o);
    if (other == nullptr) return false;

    return m_name == other->m_name && m_value == other->m_value && getLocation() == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedOverride::getName() const
{
    return m_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedOverride::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addOverride(m_name, m_value, m_location);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedDefine::LoadedDefine()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedDefine::LoadedDefine(
        const QString& name, const QString& value, const AVConfig2StorageLocation& location) :
        LoadedElement(location),
        m_name(name), m_value(value)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedDefine::operator==(const LoadedElement& o) const
{
    const LoadedDefine *other = dynamic_cast<const LoadedDefine*>(&o);
    if (other == nullptr) return false;

    return m_name == other->m_name && m_value == other->m_value && getLocation() == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedDefine::getName() const
{
    return m_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedDefine::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addDefine(m_name, m_value, m_location);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedInheritance::LoadedInheritance()
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedInheritance::LoadedInheritance(
        const QString &parent, const QString &section, const AVConfig2StorageLocation &location) :
    LoadedElement(location),
    m_parent(parent),
    m_section(section)
{

}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedInheritance::operator==(const AVConfig2ImporterClient::LoadedElement &o) const
{
    const LoadedInheritance *other = dynamic_cast<const LoadedInheritance*>(&o);
    if (other == nullptr) return false;

    return m_parent == other->m_parent &&
            m_section == other->m_section &&
            getLocation() == other->getLocation();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedInheritance::getName() const
{
    return m_parent + "->" + m_section;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedInheritance::addToImporterClient(AVConfig2ImporterClient &client) const
{
    client.addInheritedSection(m_parent, m_section, m_location);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::LoadedSubconfigTemplateParameter() : LoadedElement ()
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::LoadedSubconfigTemplateParameter(
        const QString& value, const AVConfig2Metadata& metadata, const AVConfig2StorageLocation& location) :
    LoadedElement (location),
    m_value(value),
    m_metadata(metadata)
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::~LoadedSubconfigTemplateParameter()
{

}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::operator==(const LoadedElement& o) const
{
    const LoadedSubconfigTemplateParameter *other = dynamic_cast<const LoadedSubconfigTemplateParameter*>(&o);
    if (other == nullptr) return false;

    return m_value == other->m_value && m_metadata == other->m_metadata && m_location == other->m_location;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::getName() const
{
    return m_metadata.m_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::addToImporterClient(AVConfig2ImporterClient& client) const
{
    client.addSubconfigTemplateParameter(m_value, m_metadata, m_location);
}

///////////////////////////////////////////////////////////////////////////////

const QString& AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::getValue() const
{
    return m_value;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::setValue(const QString& value)
{
    m_value = value;
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2Metadata& AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::getMetadata() const
{
    return m_metadata;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Metadata& AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::getMetadata()
{
    return m_metadata;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClient::LoadedSubconfigTemplateParameter::setMetadata(const AVConfig2Metadata& metadata)
{
    m_metadata = metadata;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterBase::import(AVConfig2ImporterClient *client, const AVConfig2StorageLocation& location)
{
    std::unique_ptr<AVConfig2ImporterBase> importer(createImporter(location));
    if (importer.get() == 0) return false;
    importer->m_client = client;
    // TODO CM import() return value
    importer->import();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ImporterBase::import(AVConfig2ImporterClient *client, const QString& filename)
{
    QFileInfo info(filename);
    if (!info.isAbsolute())
    {
        // Implement when required...
        AVLogFatal << filename << " is not an absolute path.";
    }

    AVConfig2StorageLocation location;

    for (uint f = 0; f < CF_LAST; ++f)
    {
        AVConfig2Format cur_format = static_cast<AVConfig2Format>(f);
        const QString& cur_extension = AVConfig2StorageLocation::getConfigExtension(cur_format);
        if (filename.endsWith(cur_extension))
        {
            location.m_format = cur_format;
            location.m_name = info.fileName();
            location.m_name.chop(cur_extension.length());
            location.m_location = info.absolutePath();
            break;
        }
    }
    if (location.m_format == CF_LAST) return 0;

    return import(client, location);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterBase *AVConfig2ImporterBase::createImporter(
        const AVConfig2StorageLocation& location)
{
    AVASSERT(location.m_format < CF_LAST);
    switch (location.m_format)
    {
        case CF_CLASSIC:
            return new (LOG_HERE) AVConfig2ImporterClassic(location);
        case CF_CLASSIC_MAPPING:
            return new (LOG_HERE) AVConfig2ImporterClassicMapping(location);
        case CF_XML:
            return new (LOG_HERE) AVConfig2ImporterXml(location);
        case CF_CSTYLE:
            return new (LOG_HERE) AVConfig2ImporterCstyle(location);
        case CF_LAST: // ..or compiler complains..
            AVASSERT(false);
            break;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterBase::AVConfig2ExporterBase() :
        m_order_policy(OP_REPLACE)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterBase::setOrderPolicy(OrderPolicy policy)
{
    m_order_policy = policy;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterBase::setRemovedParamNames(const QStringList& removed_param_names)
{
    m_removed_param_names = removed_param_names;
}

// End of file
