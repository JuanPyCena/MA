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
    \brief   Classes to handle old config params with the AVConfig2 implementation.
 */

#include <memory>

// QT includes
#include <QtGlobal>
#include <QDir>

// local includes
#include "avdeprecate.h"
#include "avfromtostring.h"


///////////////////////////////////////////////////////////////////////////////

#include "avconfig2classic.h"

#include "avfromtostring.h"

//! TODO CM define this in avconfig
const QString AVConfig2ImporterClassic::DEFAULT_PARASET_NAME = "default";

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClassic::AVConfig2ImporterClassic(const AVConfig2StorageLocation& location) :
        AVConfig2ImporterBase(location)
{
    // Strip path from config name for prefix
    m_param_name_prefix = location.m_name;
    int i = m_param_name_prefix.lastIndexOf("/");
    if (i != -1) m_param_name_prefix = m_param_name_prefix.mid(i+1);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClassic::~AVConfig2ImporterClassic()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClassic::import()
{
    AVASSERT(m_client != 0);

    QString full_path = m_location.getFullFilePath();
    QFileInfo full_path_info(full_path);
    if (!full_path_info.exists())
    {
        AVLogFatal << "AVConfig2ImporterClassic::import: Couldn't find "
                   << full_path;
    }

    AVLogDebug << "AVConfig2ImporterClassic::import: loading " << full_path;

    QStringList paraset_names;
    {
        // At this point, only determine parameter sets present in config file
        // proper loading of config is done below

        // Use the absolute path in case a relative -cfgdir is specified, the AVConfigBase implementation would other-
        // wise prepend the config path another time
        AVConfigBase base("", full_path_info.absoluteFilePath());
        base.setUnusedParameterPolicy(AVConfigBase::AddUnusedParams);
        base.readConfig();
        paraset_names = base.getParaSetList();
    }

    for (int ps=0; ps<paraset_names.size(); ++ps)
    {
        const QString& cur_paraset = paraset_names[ps];

        AVConfigBase base("", full_path_info.absoluteFilePath());
        base.setUnusedParameterPolicy(AVConfigBase::AddUnusedParams);
        base.readConfig(cur_paraset);

        for (AVConfigEntryList::iterator it = base.geteList()->begin();
                it != base.geteList()->end();
                ++it)
        {
            const AVConfigEntry &entry = **it;

            AVCVariant variant_val = entry.getVar();
            QString value;
            switch (variant_val.type())
            {
                case AVCVariant::StringList:
                {
                    QStringList list = variant_val.asStringList();
                    value = AVToString(list);
                    break;
                }
                case AVCVariant::IntList:
                {
                    AVIntList list = variant_val.asIntList();
                    value = AVToString(list);
                    break;
                }
                case AVCVariant::Bool:
                    value = variant_val.asBool() ? AVToString(true) : AVToString(false);
                    break;
                case AVCVariant::Int:
                case AVCVariant::UInt:
                case AVCVariant::Double:
                case AVCVariant::String:
                case AVCVariant::File:
                case AVCVariant::Directory:
                {
                    QTextStream value_out_stream(&value, QIODevice::WriteOnly);
                    variant_val.write(value_out_stream, true);
                    break;
                }
                case AVCVariant::Size:
                {
                    QSize size = variant_val.asSize();
                    value = AVToString(size);
                    break;
                }
                case AVCVariant::Font:
                {
                    QFont font = variant_val.asFont();
                    value = font.toString();
                    break;
                }
                case AVCVariant::Color:
                {
                    QColor color = variant_val.asColor();
                    value = AVToString(color);
                    break;
                }
                case AVCVariant::Rect:
                {
                    QRect rect = variant_val.asRect();
                    value = AVToString(rect);
                    break;
                }
                case AVCVariant::Point:
                {
                    QPoint point = variant_val.asPoint();
                    value = AVToString(point);
                    break;
                }
                default:
                    AVLogFatal << "AVConfig2LoaderClassic::load: "
                               << "Still need to implement variant type "
                               << variant_val.type()
                               << " (" << entry.key << " in "
                               << m_location.getFullFilePath()
                               << ")";
                    break;
            }

            // Prepend the config file as prefix only if no dot is present in the entry key (this
            // allows a smooth transition to relocated params).
            // Section "default" is ignored.
            QString full_param_name;
            if (entry.key.indexOf('.') == -1 && cur_paraset.indexOf('.') == -1)
            {
                full_param_name = m_param_name_prefix + ".";
            }
            if (cur_paraset != DEFAULT_PARASET_NAME)
            {
                full_param_name += cur_paraset + ".";
            }
            full_param_name += entry.key;

            // Handle metadata
            AVConfig2Metadata metadata(
                    full_param_name, variantTypeToString(variant_val.type()), entry.desc);
            metadata.m_restriction = buildRestriction(entry);
            if (!entry.option.isEmpty()) metadata.m_command_line_switch = entry.option;

            // Adopt the default value from the loaded config.
            //
            // The if clauses below are workarounds so the metadata check is consistent for the most
            // common case (default value had to be specified with the old config system, so if you
            // elect to specify no suggested value for your QStringList/QString etc param, you must
            // use the new config format).
            if (entry.getDefault().type() != AVCVariant::Undef)
            {
                if (entry.getDefault().type() == AVCVariant::Size)
                {
                    // AVToString QSize representation is different from AVCVariant
                    metadata.m_default_value = AVToString(entry.getDefault().asSizeConst());
                } else
                {
                    metadata.m_default_value = entry.getDefault().toString();
                }
            } else if (variant_val.type() == AVCVariant::StringList)
            {
                metadata.m_default_value = AVToString(QStringList());
            } else if (variant_val.type() == AVCVariant::IntList)
            {
                metadata.m_default_value = AVToString(AVIntList());
            } else if (variant_val.type() == AVCVariant::String)
            {
                metadata.m_default_value = QString(avfromtostring::QUOTE) + avfromtostring::QUOTE;
            }

            m_location.m_source = AVConfig2StorageLocation::PS_FILE_ORDINARY;
            if (entry.reference.isNull())
            {
                // Qt5: trimming an empty string results in a null string - not the desired behaviour here.
                value = value.trimmed();
                if (value.isNull()) value = "";
                m_client->addParameter(value, metadata, m_location);
            } else
            {
                // Qt5: trimming an empty string results in a null string - not the desired behaviour here.
                QString ref = entry.reference.trimmed();
                if (ref.isNull()) ref = "";
                m_client->addReference(ref, metadata, m_location);
            }
            m_location.m_source = AVConfig2StorageLocation::PS_UNKNOWN;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AVConfig2RestrictionBase> AVConfig2ImporterClassic::buildRestriction(
    const AVConfigEntry& entry) const
{
    // TODO CM complete this method
    switch (entry.getVar().type())
    {
        case AVCVariant::Int:
        {
            int min = entry.getMin().asIntConst();
            int max = entry.getMax().asIntConst();
            if (min == -AVConfigEntry::INT_DEFAULT_MAXMIN &&
                max ==  AVConfigEntry::INT_DEFAULT_MAXMIN)
            {
                return nullptr;
            }

            std::unique_ptr<AVConfig2RangeRestriction<int>> ret = std::make_unique<AVConfig2RangeRestriction<int>>();
            if (min != -AVConfigEntry::INT_DEFAULT_MAXMIN) ret->setMin(min);
            if (max !=  AVConfigEntry::INT_DEFAULT_MAXMIN) ret->setMax(max);

            return ret;
        }
        case AVCVariant::UInt:
        {
            int max = entry.getMax().asUIntConst();
            if (max ==  AVConfigEntry::INT_DEFAULT_MAXMIN) return nullptr;

            std::unique_ptr<AVConfig2RangeRestriction<int>> ret = std::make_unique<AVConfig2RangeRestriction<int>>();
            if (max !=  AVConfigEntry::INT_DEFAULT_MAXMIN) ret->setMax(max);

            return ret;
        }
        case AVCVariant::Double:
        {
            double min = entry.getMin().asDoubleConst();
            double max = entry.getMax().asDoubleConst();

            if (min == -AVConfigEntry::DOUBLE_DEFAULT_MAXMIN &&
                max ==  AVConfigEntry::DOUBLE_DEFAULT_MAXMIN)
            {
                return nullptr;
            }

            std::unique_ptr<AVConfig2RangeRestriction<double>> ret = std::make_unique<AVConfig2RangeRestriction<double>>();
            if (min != -AVConfigEntry::DOUBLE_DEFAULT_MAXMIN) ret->setMin(min);
            if (max !=  AVConfigEntry::DOUBLE_DEFAULT_MAXMIN) ret->setMax(max);

            return ret;
        }
        case AVCVariant::Bool:
        {
            return nullptr;
        }
        case AVCVariant::String:
        case AVCVariant::File:
        case AVCVariant::Directory:
        {
            int max = entry.getMax().asIntConst();
            if (max == AVConfigEntry::STRING_DEFAULT_MAX_LENGTH || max == 0) return nullptr;

            AVLogger->Write(LOG_FATAL, "Length restriction for strings not (yet) implemented "
                    "(%s restricted to %d characters).\nIf this is truly needed, contact "
                    "c.muschick@avibit.com", qPrintable(entry.key), max);
        }
        case AVCVariant::StringList:
        {
            int max = entry.getMax().asIntConst();
            if (max == AVConfigEntry::STRING_DEFAULT_MAX_LENGTH) return nullptr;

            // TODO CM implement size restriction for any kind of container
            AVLogFatal << "Length restriction for stringlist not (yet) implemented ("
                       << entry.key
                       << " restricted to "
                       << max
                       << " elements).\nIf this is needed, contact c.muschick@avibit.com";
        }
        case AVCVariant::IntList:
        {
            int min = entry.getMin().asIntConst();
            int max = entry.getMax().asIntConst();
            if (max ==  AVConfigEntry::INT_DEFAULT_MAXMIN &&
                min == -AVConfigEntry::INT_DEFAULT_MAXMIN) return nullptr;

            // TODO CM implement size restriction for any kind of container
            AVLogFatal << "Length restriction for intlist not (yet) implemented ("
                    << entry.key
                    << ").\nIf this is needed, contact c.muschick@avibit.com";
        }
        case AVCVariant::Size:
        {
            QSize min = entry.getMin().asSizeConst();
            QSize max = entry.getMax().asSizeConst();

            if (max == QSize( AVConfigEntry::POINT_DEFAULT_MAXMIN, AVConfigEntry::POINT_DEFAULT_MAXMIN) &&
                min == QSize(-AVConfigEntry::POINT_DEFAULT_MAXMIN,-AVConfigEntry::POINT_DEFAULT_MAXMIN))
            {
                return nullptr;
            }

            // TODO CM implement size restriction for any kind of container
            AVLogFatal << "Restriction for QSize not (yet) implemented ("
                    << entry.key
                    << ").\nIf this is needed, contact c.muschick@avibit.com";
        }
        case AVCVariant::Color:
        case AVCVariant::Font:
        case AVCVariant::Rect:
        case AVCVariant::Point:
        {
            return nullptr;
        }
        default:
        {
            AVLogFatal << "AVConfig2LoaderClassic::buildRestriction: not yet "
                       << "implemented for type "
                       << entry.getVar().type();
            return nullptr;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterClassic::variantTypeToString(AVCVariant::Type type) const
{
    QString ret;
    switch (type)
    {
        case AVCVariant::Int:        ret = AVConfig2Types::getTypeName<int>(); break;
        case AVCVariant::UInt:       ret = AVConfig2Types::getTypeName<uint>(); break;
        case AVCVariant::Double:     ret = AVConfig2Types::getTypeName<double>(); break;
        case AVCVariant::Bool:       ret = AVConfig2Types::getTypeName<bool>(); break;
        case AVCVariant::String:     ret = AVConfig2Types::getTypeName<QString>(); break;
        case AVCVariant::Rect:       ret = AVConfig2Types::getTypeName<QRect>(); break;
        case AVCVariant::Color:      ret = AVConfig2Types::getTypeName<QColor>(); break;
        case AVCVariant::Size:       ret = AVConfig2Types::getTypeName<QSize>(); break;
        case AVCVariant::Point:      ret = AVConfig2Types::getTypeName<QPoint>(); break;
        case AVCVariant::StringList: ret = AVConfig2Types::getTypeName<QStringList>(); break;
        case AVCVariant::IntList:    ret = AVConfig2Types::getTypeName<QList<int> >(); break;
        case AVCVariant::Font:       ret = AVConfig2Types::getTypeName<QFont>(); break;
        case AVCVariant::Directory:  ret = AVConfig2Types::getTypeName<QString>(); break;
        case AVCVariant::File:       ret = AVConfig2Types::getTypeName<QString>(); break;
        case AVCVariant::Undef: AVASSERT(false);
    }

    if (ret.isNull())
    {
        AVLogger->Write(LOG_FATAL, "AVConfig2LoaderClassic::variantTypeToString: "
                        "type %d is not (yet) known to the config system!", type);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClassicMapping::AVConfig2ImporterClassicMapping(
        const AVConfig2StorageLocation& location) :
        AVConfig2ImporterBase(location)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterClassicMapping::~AVConfig2ImporterClassicMapping()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterClassicMapping::import()
{
    AVASSERT(m_client != 0);
    QString full_path = m_location.getFullFilePath();
    QFile file(full_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        AVLogFatal << "AVConfig2ImporterClassicMapping::import: "
                   << "Failed to read mapping file " << full_path;
    }

    QString default_dir;
    QTextStream stream(&file);
    while (!stream.atEnd())
    {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        if (line.startsWith("@"))
        {
            m_client->addInclude(line.mid(1), m_location);
            continue;
        }

        QString key, value;
        QStringList pair = line.split(':');
        int count = pair.count();
        if (count == 1)
        {
            // Allow mappings of the form "filename" to be used. The DEFAULT mapping is
            // applied in this case. If the default mapping is "DEFAULT:dir/*",
            // then "filename" is equivalent to "filename:dir/filename".

            key = pair[0].trimmed();
            if (default_dir.isEmpty())
            {
                AVLogFatal << "AVConfig2ImporterClassicMapping::import: "
                           << "no DEFAULT mapping specified for " << key;
            }
        }
        else if (count == 2)
        {
            key = pair[0].trimmed();
            value = pair[1].trimmed();
        }
        else
        {
            AVLogFatal << "AVConfig2ImporterClassicMapping::import:"
                       << "invalid mapping: " << line;
        }

        // Check for DEFAULT mapping

        if (key == "DEFAULT")
        {
            default_dir = value;
            AVASSERT(default_dir.endsWith("/*"));
            default_dir = default_dir.left(default_dir.length()-2);
            continue;
        }

        if (count == 1)
        {
            m_client->addConfigMapping(key, default_dir, value, m_location);
        } else
        {
            m_client->addConfigMapping(key, QString::null, value, m_location);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addParameter(const QString& value, const AVConfig2Metadata& metadata,
                                            const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);
    add(value, metadata, false);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addReference(const QString& ref, const AVConfig2Metadata& metadata,
                                            const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);
    add(ref, metadata, true);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addSearchPath(
        const QString& path, const AVConfig2StorageLocation& location)
{
    Q_UNUSED(path);
    Q_UNUSED(location);
    AVLogFatal << "Search paths not implemented for classic format";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addInclude(const QString& include,
                                          const AVConfig2StorageLocation& location)
{
    Q_UNUSED(include);
    Q_UNUSED(location);
    AVLogFatal << "Sorry, includes are not supported with the classic config format.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addConfigMapping(const QString& map_from, const QString& mapped_dir,
                                                const QString& mapped_name,
                                                const AVConfig2StorageLocation& location)
{
    Q_UNUSED(map_from);
    Q_UNUSED(mapped_dir);
    Q_UNUSED(mapped_name);
    Q_UNUSED(location);
    AVLogFatal << "Sorry, config mappings are not supported with the classic config format.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addOverride(const QString& name, const QString& value,
                                           const AVConfig2StorageLocation& location)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(location);
    AVLogFatal << "Sorry, config overrides are not supported with the classic config format.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addDefine(const QString& name, const QString& value,
                                         const AVConfig2StorageLocation& location)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(location);
    AVLogFatal << "Sorry, defines are not supported with the classic config format.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                                             const AVConfig2StorageLocation& location)
{
    Q_UNUSED(value);
    Q_UNUSED(metadata);
    Q_UNUSED(location);
    AVLogError << "Subconfig templates are not supported with the classic config format.";
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ExporterClassic::doExport()
{
    QString ret;
    for (ParamContainer::const_iterator it = m_param.begin();
         it != m_param.end();
         ++it)
    {
        const QString&  cur_section    = it->first;
        const ParamList cur_param_list = it->second;

        ret += "\n[" + cur_section + "]\n\n";

        for (ParamList::const_iterator it2 = cur_param_list.begin();
             it2 != cur_param_list.end();
             ++it2)
        {
            const QString&             cur_value    = it2->m_value;
            const AVConfig2Metadata&   cur_metadata = it2->m_metadata;
            bool                       cur_is_ref   = it2->m_is_reference;

            QString string_rep = toString(cur_value, cur_metadata, cur_is_ref);

            if (string_rep.isNull())
            {
                AVLogFatal << "AVConfig2ExporterClassic::doExport: cannot save \""
                           << cur_section + "." + cur_metadata.m_name << "\" because support for type "
                           << cur_metadata.m_type << " is not implemented.";
            }

            ret += string_rep;
        }
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::reset()
{
    m_param.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ExporterClassic::removeParameter(
    const QString& full_name, const AVConfig2StorageLocation::FactConditionContainer& facts)
{
    Q_UNUSED(facts);

    QString section, name;
    splitSectionAndName(full_name, section, name);

    for (int i=0; i<m_param.size(); ++i)
    {
        if (m_param[i].first == section)
        {
            ParamList& cur_param_list = m_param[i].second;
            for (ParamList::iterator it = cur_param_list.begin(); it != cur_param_list.end(); ++it)
            {
                ParamOrReference& cur_param_or_ref = *it;
                if (cur_param_or_ref.m_metadata.m_name == name)
                {
                    cur_param_list.erase(it);
                    return true;
                }
            }
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::add(
    const QString& value, const AVConfig2Metadata& metadata, bool ref)
{
    QString section;
    QString name;
    splitSectionAndName(metadata.m_name, section, name);

    // Strip the section from the parameter name
    AVConfig2Metadata stored_metadata = metadata;
    stored_metadata.m_name = name;

    ParamList& cur_list = getOrCreateParamList(section);

    // Check whether the parameter already is present. If so, it is overwritten with the new value
    // (when saving config files, the exporter is preloaded with the existing file)
    for (ParamList::iterator it = cur_list.begin();
         it != cur_list.end();
         ++it)
    {
        if (it->m_metadata.m_name == stored_metadata.m_name)
        {
            *it = ParamOrReference(value, stored_metadata, ref);
            return;
        }
    }

    cur_list.push_back(ParamOrReference(value, stored_metadata, ref));
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ExporterClassic::toString(
        const QString& value, const AVConfig2Metadata& metadata, bool is_ref) const
{
    if (metadata.m_type == AVConfig2Types::getTypeName<int>())
    {
        return toStringWithRestriction<int>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<uint>())
    {
        return toStringWithRestriction<uint>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<double>())
    {
        return toStringWithRestriction<double>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<bool>())
    {
        return toString<bool>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QString>())
    {
        return toString<QString>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QStringList>())
    {
        return toString<QStringList>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<AVIntList>())
    {
        return toString<AVIntList>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QSize>())
    {
        return toString<QSize>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QFont>())
    {
        return toString<QFont>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QPoint>())
    {
        return toString<QPoint>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QColor>())
    {
        return toString<QColor>(value, metadata, is_ref);
    } else if (metadata.m_type == AVConfig2Types::getTypeName<QRect>())
    {
        return toString<QRect>(value, metadata, is_ref);
    }

    AVLogError << "AVConfig2ExporterClassic::toString: unsupported type \"" << metadata.m_type
               << "\". Implement it or switch to cstyle config format.";
    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassic::splitSectionAndName(
        const QString& full_name, QString& section, QString& name)
{
    int sep1 = full_name.lastIndexOf('.');
    if (sep1 == -1)
    {
        AVLogger->Write(LOG_FATAL, "AVConfig2ExporterClassic::splitSectionAndName: "
                "For technical compatibility reasons, it is not allowed to export parameters "
                "without section in the classic config format (%s)", qPrintable(full_name));
    }
    int sep2 = full_name.lastIndexOf('.', sep1-1);
    section = full_name.left(sep1);
    name    = full_name.mid(sep1 + 1);
    if (sep2 != -1)
    {
        // The parameter is nested more than 1 level - skip backwards compatibility behaviour
        AVASSERT(sep1 < full_name.length() -1);
    } else
    {
        // Nesting is exactly one level - if the section name matches the config file name, don't add it to the
        // parameter name
        if (m_config_name == section || m_config_name.endsWith("/" + section))
        {
            name = full_name.mid(sep1 + 1);
        } else name = full_name;
        section = AVConfig2ImporterClassic::DEFAULT_PARASET_NAME;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterClassic::ParamList& AVConfig2ExporterClassic::getOrCreateParamList(
        const QString& section)
{
    for (ParamContainer::iterator it = m_param.begin();
        it != m_param.end();
        ++it)
    {
        if (it->first == section) return it->second;
    }
    m_param.push_back(qMakePair(section, ParamList()));
    return m_param.back().second;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterClassic::ParamOrReference::ParamOrReference(
        const QString& value, const AVConfig2Metadata& metadata, bool is_reference) :
    m_value(value), m_metadata(metadata), m_is_reference(is_reference)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterClassicMapping::AVConfig2ExporterClassicMapping() :
    m_longest_from(0)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterClassicMapping::~AVConfig2ExporterClassicMapping()
{

}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addParameter(
        const QString& value, const AVConfig2Metadata& metadata,
        const AVConfig2StorageLocation& location)
{
    Q_UNUSED(value);
    Q_UNUSED(metadata);
    Q_UNUSED(location);
    AVASSERT(false);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addReference(
        const QString& ref, const AVConfig2Metadata& metadata,
        const AVConfig2StorageLocation& location)
{
    Q_UNUSED(ref);
    Q_UNUSED(metadata);
    Q_UNUSED(location);
    AVASSERT(false);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addSearchPath(
        const QString& path, const AVConfig2StorageLocation& location)
{
    Q_UNUSED(path);
    Q_UNUSED(location);
    AVLogFatal << "Search paths not implemented for classic mapping format";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addInclude(const QString& include,
                                                 const AVConfig2StorageLocation& location)
{
    Q_UNUSED(location);
    m_includes.insert(include);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addConfigMapping(
    const QString& map_from, const QString& mapped_dir,
    const QString& mapped_name,
    const AVConfig2StorageLocation& location)
{
    m_mappings[map_from] = LoadedMapping(map_from, mapped_dir, mapped_name, location);
    if (map_from.length() > static_cast<int>(m_longest_from)) m_longest_from = map_from.length();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addOverride(const QString& name, const QString& value,
                                                  const AVConfig2StorageLocation& location)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(location);
    AVLogFatal << "Sorry, config overrides are not supported with the classic mapping "
               << "config format.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addDefine(const QString& name, const QString& value,
                                                const AVConfig2StorageLocation& location)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(location);
    AVLogFatal << "Sorry, defines are not supported with the classic mapping "
               << "config format.";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                   const AVConfig2StorageLocation& location)
{
    Q_UNUSED(value);
    Q_UNUSED(metadata);
    Q_UNUSED(location);
    AVLogError << "Subconfig templates are not supported with the classic mapping config format.";
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ExporterClassicMapping::doExport()
{
    QString ret;
    QTextStream stream(&ret, QIODevice::WriteOnly);
    for (const QString& include : m_includes)
    {
        stream << "@" << include << "\n";
    }

    stream << "\n";



    // FIXXME duplicated from AVConfig2ExporterCstyle::writeMappings :(

    typedef QMap<QString, MappingContainer> MappingByDirContainer;
    MappingByDirContainer mappings_by_dir;

    for (MappingContainer::const_iterator it = m_mappings.begin();
         it != m_mappings.end();
         ++it)
    {
        const LoadedMapping& cur_mapping = it.value();
        mappings_by_dir[cur_mapping.m_mapped_dir][cur_mapping.m_from] =
            LoadedMapping(cur_mapping.m_from, cur_mapping.m_mapped_dir,
                          cur_mapping.m_mapped_name, cur_mapping.getLocation());
    }

    for (MappingByDirContainer::const_iterator dir_it = mappings_by_dir.begin();
            dir_it != mappings_by_dir.end();
            ++dir_it)
    {
        QString dir = dir_it.key();

        if (!dir.isNull())
        {
            dir = "DEFAULT:" + dir + "/*";
            stream << dir << "\n";
        }

        for (MappingContainer::const_iterator map_it = dir_it.value().begin();
                map_it != dir_it.value().end();
                ++map_it)
        {
            const QString& cur_from = map_it.key();
            const QString& cur_to   = map_it.value().m_mapped_name;

            if (!cur_to.isNull())
            {
                stream.setFieldAlignment(QTextStream::AlignLeft);
                stream << qSetFieldWidth(m_longest_from) << qSetPadChar(' ')
                       << cur_from << qSetFieldWidth(0) << " : "
                       << cur_to;
            } else
            {
                stream << cur_from;
            }
            stream << "\n";
        }
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterClassicMapping::reset()
{
    m_includes.clear();
    m_mappings.clear();
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2Container AVConfig2HybridConfig::m_dummy_container;

///////////////////////////////////////////////////////////////////////////////

AVConfig2HybridConfig::AVConfig2HybridConfig(const QString& help_group, const QString& prefix,
                                             AVConfig2Container *container) :
        AVConfig2(prefix,
                  container != 0 ? *container : (AVConfigBase::process_uses_avconfig2 ?
                                                 AVConfig2Global::singleton() :
                                                 m_dummy_container)),
        AVConfigBase(prefix, "", false)
{
    setHelpGroup(help_group);
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2HybridConfig::isNewConfigUsed() const
{
    return &getContainer() != &m_dummy_container;
}

// End of file
