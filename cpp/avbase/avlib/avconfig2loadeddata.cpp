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

#include "avconfig2loadeddata.h"

///////////////////////////////////////////////////////////////////////////////

AVConfig2LoadedData::SubconfigTemplateSpecification::SubconfigTemplateSpecification()
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2LoadedData::SubconfigTemplateSpecification::SubconfigTemplateSpecification(const QString& template_name,
                                                                                    const AVConfig2StorageLocation& location) :
    m_template_name(template_name),
    m_location(location)
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::SubconfigTemplateSpecification::operator==(const SubconfigTemplateSpecification& other) const
{
    return m_template_name == other.m_template_name && m_location == other.m_location;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::SubconfigTemplateSpecification::setTemplateName(const QString& template_name)
{
    m_template_name = template_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::SubconfigTemplateSpecification::setLocation(const AVConfig2StorageLocation& location)
{
    m_location = location;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2LoadedData::SubconfigTemplateSpecification::getTemplateName() const
{
    return m_template_name;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2StorageLocation AVConfig2LoadedData::SubconfigTemplateSpecification::getLocation() const
{
    return m_location;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2LoadedData::AVConfig2LoadedData(bool unique_elements) :
    m_unique_elements(unique_elements)
{
    // always do this initialization, to avoid problems when assiging between
    // objects with different value for unique_elements
    m_loaded_element_dicts[&m_loaded_parameter]   = ElementDict();
    m_loaded_element_dicts[&m_loaded_reference]   = ElementDict();
    m_loaded_element_dicts[&m_loaded_mapping]     = ElementDict();
    m_loaded_element_dicts[&m_loaded_search_path] = ElementDict();
    m_loaded_element_dicts[&m_loaded_include]     = ElementDict();
    m_loaded_element_dicts[&m_loaded_override]    = ElementDict();
    m_loaded_element_dicts[&m_loaded_define]      = ElementDict();
    m_loaded_element_dicts[&m_loaded_inheritance] = ElementDict();
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2LoadedData::AVConfig2LoadedData(const AVConfig2LoadedData& other) :
    AVConfig2LoadedData(other.m_unique_elements)
{
    for (const LoadedElement* element : other.m_total_element_list)
    {
        element->addToImporterClient(*this);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2LoadedData::~AVConfig2LoadedData()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::operator==(const AVConfig2LoadedData& other) const
{
    if (m_unique_elements != other.m_unique_elements) return false;

    if (m_total_element_list.count() != other.m_total_element_list.count()) return false;
    for (int e=0; e<m_total_element_list.count(); ++e)
    {
        if (*m_total_element_list[e] != *other.m_total_element_list[e]) return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2LoadedData &AVConfig2LoadedData::operator=(const AVConfig2LoadedData &other)
{
    if (this == &other) return *this;

    // clear our previous state...
    {
        m_loaded_parameter.clear();
        m_loaded_reference.clear();
        m_loaded_mapping.clear();
        m_loaded_search_path.clear();
        m_loaded_include.clear();
        m_loaded_override.clear();
        m_loaded_define.clear();
        m_loaded_inheritance.clear();
        for (ElementDict& dict : m_loaded_element_dicts)
        {
            dict.clear();
        }
        m_total_element_list.clear();
    }

    // assign new state
    m_unique_elements = other.m_unique_elements;
    for (const LoadedElement* element : other.m_total_element_list)
    {
        element->addToImporterClient(*this);
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::loadConfigTree(const QString& dir_path)
{
    if (!QDir(dir_path).isReadable())
    {
        AVLogError << "AVConfig2LoadedData::loadConfigTree: " << dir_path << " is not readable.";
        return false;
    }

    QVector<QDir> dir_stack;
    dir_stack.push_back(dir_path);
    if (!dir_stack.back().makeAbsolute())
    {
        AVLogError << "AVConfig2LoadedData::loadConfigTree: " << dir_path << " cannot be made absolute.";
        return false;
    }

    while (!dir_stack.isEmpty())
    {
        QDir cur_dir(dir_stack.back());
        dir_stack.pop_back();
        cur_dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        QStringList entries = cur_dir.entryList();
        for (QStringList::const_iterator entry_it = entries.begin(); entry_it != entries.end(); ++entry_it)
        {
            QString cur_path = cur_dir.absoluteFilePath(*entry_it);
            if (QFileInfo(cur_path).isDir()) dir_stack.push_back(cur_path);
            if (AVConfig2ImporterBase::import(this, cur_path))
            {
                AVLogDebug << "AVConfig2LoadedData::loadConfigTree: Scanned " << cur_dir.path() << "/" << *entry_it;
            } else
            {
                AVLogDebug << "AVConfig2LoadedData::loadConfigTree: Skipped " << cur_dir.path() << "/" << *entry_it;
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::load(const QString& file)
{
    return AVConfig2ImporterBase::import(this, file);
}

///////////////////////////////////////////////////////////////////////////////

uint AVConfig2LoadedData::getElementTotalCount() const
{
    return m_total_element_list.count();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::isEmpty() const
{
    return m_total_element_list.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////

const AVConfig2ImporterClient::LoadedElement &AVConfig2LoadedData::getElementByTotalIndex(uint index) const
{
    AVASSERT(index < static_cast<uint>(m_total_element_list.count()));
    return *m_total_element_list[index];
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addParameter(
        const QString& value, const AVConfig2Metadata& metadata, const AVConfig2StorageLocation& location)
{
    addElement(LoadedParameter(value, metadata, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addReference(
        const QString& ref, const AVConfig2Metadata& metadata, const AVConfig2StorageLocation& location)
{
    addElement(LoadedReference(ref, metadata, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addSearchPath(const QString& path, const AVConfig2StorageLocation& location)
{
    addElement(LoadedSearchPath(path, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addInclude(const QString& include, const AVConfig2StorageLocation& location)
{
    addElement(LoadedInclude(include, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addConfigMapping(
        const QString& map_from, const QString& mapped_dir, const QString& mapped_name,
        const AVConfig2StorageLocation& location)
{
    addElement(LoadedMapping(map_from, mapped_dir, mapped_name, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addOverride(const QString& name, const QString& value,
                                      const AVConfig2StorageLocation& location)
{
    addElement(LoadedOverride(name, value, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addDefine(const QString& name, const QString& value, const AVConfig2StorageLocation& location)
{
    addElement(LoadedDefine(name, value, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addInheritedSection(const QString &parent, const QString &section, const AVConfig2StorageLocation &location)
{
    addElement(LoadedInheritance(parent, section, location));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2LoadedData::addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                                        const AVConfig2StorageLocation& location)
{
    addElement(LoadedSubconfigTemplateParameter(value, metadata, location));
}

///////////////////////////////////////////////////////////////////////////////

const std::set<AVConfig2StorageLocation>& AVConfig2LoadedData::getLoadedLocations() const
{
    return m_loaded_locations;
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::instantiateSubconfigTemplate(const SubconfigTemplateSpecification& spec,
                                                       const QString& instance_name)
{
    return instantiateSubconfigTemplate(spec, QVector<QString>({ instance_name }));
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2LoadedData::instantiateSubconfigTemplate(
        const SubconfigTemplateSpecification& spec, const QVector<QString>& instance_names)
{
    AVASSERT(!instance_names.empty());

    QString full_subconfig_prefix = spec.getTemplateName();
    int sep = full_subconfig_prefix.indexOf(".*.");
    int idx = 0;
    while (sep != -1)
    {
        full_subconfig_prefix.replace(sep + 1, 1, instance_names[idx++]);
        sep = full_subconfig_prefix.indexOf(".*.");
        if (idx >= instance_names.size())
        {
            AVLogError   << "Unable to instantiate subconfig template " << spec.getTemplateName()
                         << ". Instance name is missing.\n"
                         << "   Actual (instance_names.size()): "
                         << instance_names.size() << "\n"
                         << "   Expected                      : "
                         << spec.getTemplateName().count(".*.") + 1;
            return false;
        }
    }
    QList<SubconfigTemplateSpecification> templates = listSubconfigTemplates(full_subconfig_prefix);
    if (templates.empty())
    {
        AVLogError   << "Unable to instantiate subconfig template " << spec.getTemplateName()
                     << ". Can't find loaded subconfig template matching the SubconfigTemplateSpecification.";
        return false;
    }
    QString subconfig_template_param_prefix = templates.front().getTemplateName() + ".*";
    full_subconfig_prefix += "." + instance_names[idx];
    int subconfig_template_end_index = subconfig_template_param_prefix.size();

    for (const LoadedSubconfigTemplateParameter& template_param : m_loaded_subconfig_template_parameter)
    {
        if (template_param.getLocation() == spec.getLocation() &&
                template_param.getName().startsWith(subconfig_template_param_prefix) &&
                template_param.getName().lastIndexOf('*') < subconfig_template_end_index)
        {
            AVConfig2Metadata metadata = template_param.getMetadata();
            metadata.m_name.replace(0, subconfig_template_end_index, full_subconfig_prefix);

            addElement(LoadedParameter(template_param.getValue(), metadata, spec.getLocation()));
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QList<AVConfig2LoadedData::SubconfigTemplateSpecification> AVConfig2LoadedData::listSubconfigTemplates(
        const QString& prefix) const
{
    QList<SubconfigTemplateSpecification> subconfig_templates;

    QStringList prefix_tokens = prefix.split('.');
    for (const LoadedSubconfigTemplateParameter& param : m_loaded_subconfig_template_parameter)
    {
        QStringList template_tokens = param.getName().split('.');
        if (template_tokens.size() > prefix_tokens.size())
        {
            bool is_matching = true;
            for (int i = 0; i < prefix_tokens.size(); ++i)
            {
                if (prefix_tokens[i] != template_tokens[i] && template_tokens[i] != "*")
                {
                    is_matching = false;
                    break;
                }
            }
            if (is_matching && template_tokens[prefix_tokens.size()] == "*")
            {
                template_tokens.erase(template_tokens.begin() + prefix_tokens.size(), template_tokens.end());
                QString subconfig_template_name = template_tokens.join(".");
                SubconfigTemplateSpecification spec = { subconfig_template_name, param.getLocation() };
                if (!subconfig_templates.contains(spec))
                {
                    subconfig_templates.append(spec);
                }
            }
        }
    }

    return subconfig_templates;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedParameter> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_parameter;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedReference> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_reference;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedMapping> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_mapping;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedSearchPath> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_search_path;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedInclude> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_include;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedOverride> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_override;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedDefine> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_define;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedInheritance> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_inheritance;
}

///////////////////////////////////////////////////////////////////////////////

template<>
QList<AVConfig2ImporterClient::LoadedSubconfigTemplateParameter> &AVConfig2LoadedData::getElementContainer()
{
    return m_loaded_subconfig_template_parameter;
}

// End of file
