///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// Copyright:  AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config params stored in C-style format
 */


#include <memory>

// QT includes
#include <qglobal.h>

#include "avconfig.h" // TODO CM for application config, fix this
#include "avfromtostring.h"
#include "avconfig2.h"
#include "avdeprecate.h"
#include "avfromtostring.h"


///////////////////////////////////////////////////////////////////////////////

#include "avconfig2cstyle.h"

const QString VERSION_TOKEN            = "#avconfig_version";
const QString INCLUDE_TOKEN            = "#include";
const QString SEARCH_TOKEN             = "#search";
const QString MAPPING_START_TOKEN      = "#map";
const QString MAPPING_END_TOKEN        = "#endmap";
const QString DEFINE_TOKEN             = "#define";
const QString SECTION_TOKEN_V1         = "section";
const QString SECTION_TOKEN            = "namespace";
const QString FACTS_START_TOKEN        = "#if";
const QString FACTS_END_TOKEN          = "#endif";
const QString FACTS_EQUALS_TOKEN       = "==";
const QString SUBCONFIG_TEMPLATE_TOKEN = "subconfig_template";

const QString META_COMMENT_TOKEN      = "//!";
const QString META_PREFIX             = "\\";
const QString META_CMDLINE_TOKEN      = META_PREFIX + "cmdline";
const QString META_SUGGESTED_TOKEN    = META_PREFIX + "suggested";
const QString META_SUGGESTED_REF_TOKEN= META_PREFIX + "suggested_ref";
const QString META_RESTRICTION_TOKEN  = META_PREFIX + "restriction";
const QString META_USER_COMMENT_TOKEN = META_PREFIX + "comment";
const QString META_OPTIONAL_TOKEN     = META_PREFIX + "optional";

const QString QUOTE_TOKEN         = "\"";
const QString COMMENT_TOKEN       = "//";
const QString REF_TOKEN           = "&";
const QString ASSIGN_TOKEN        = "=";
const QString TERMINATION_TOKEN   = ";";
const QString MAP_TO_TOKEN        = ":";
const QString INHERIT_TOKEN       = ":";
const QString SPACE_TOKEN         = " ";
const QString END_OF_LINE_TOKEN   = "\n";

const QString SECTION_START_TOKEN = "{";
const QString SECTION_END_TOKEN   = "}";

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterCstyle::AVConfig2ImporterCstyle(const AVConfig2StorageLocation& location) :
        AVConfig2ImporterBase(location),
        m_cur_line_number(0),
        m_cur_version(V_UNKNOWN),
        m_current_namespace_level(0)
{
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterCstyle::AVConfig2ImporterCstyle(const QString &source_string) :
    AVConfig2ImporterBase(AVConfig2StorageLocation()),
    m_instream(&m_source_string, QIODevice::ReadOnly),
    m_cur_line_number(0),
    m_cur_version(V_UNKNOWN),
    m_current_namespace_level(0)
{
    m_source_string = source_string;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ImporterCstyle::~AVConfig2ImporterCstyle()
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::import()
{
    AVASSERT(m_client);

    if (m_source_string.isNull())
    {
        m_file.setFileName(m_location.getFullFilePath());

        if (!m_file.open(QIODevice::ReadOnly))
        {
            AVLogFatal << "AVConfig2ImporterCstyle::import: Failed to open " << m_file.fileName();
            return;
        }

        m_instream.setDevice(&m_file);
        // don't rely on correct locale; setDevice resets the codec...
        m_instream.setCodec("UTF-8");
    }

    m_cur_line_number = 0;

    readVersion();
    readDefines();
    readOverrides();
    readMappings();
    readSearchPaths();
    readIncludes();

    QSet<QString> loaded_subconfig_templates;

    while (!peekLine().isEmpty())
    {
        if (peekLine().startsWith(DEFINE_TOKEN))
            fatalError("Defines must be placed first in the config file.");
        if (peekLine().startsWith(MAPPING_START_TOKEN))
            fatalError("Mappings must be placed before search paths, includes and namespaces.");
        if (peekLine().startsWith(SEARCH_TOKEN))
            fatalError("Search path extensions mut be placed before includes and namespaces.");
        if (peekLine().startsWith(INCLUDE_TOKEN))
            fatalError("Includes mut be placed before namespaces.");

        if (peekLine().startsWith(SUBCONFIG_TEMPLATE_TOKEN))
        {
            QString subconfig_template = readSubconfigTemplate();
            if (loaded_subconfig_templates.contains(subconfig_template))
            {
                fatalError("Subconfig template \"" + subconfig_template + "\" has already been added. "
                           "Subconfig template must occur only once.");
            } else
            {
                loaded_subconfig_templates.insert(subconfig_template);
            }
        } else
        {
            readSectionOrFactOrParam("");
        }
    }

    if (!m_location.getFactConditions().isEmpty())
    {
        fatalError("Unbalanced #if");
    }

    m_file.close();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readVersion()
{
    readLine();
    nextToken(VERSION_TOKEN, true);
    bool ok;
    m_cur_version = static_cast<VERSION>(nextToken("config version number").toInt(&ok));
    if (!ok) fatalError("Failed to parse version");
    if (m_cur_version == V_UNKNOWN || m_cur_version > V_CURRENT) fatalError("Unsupported version");

    if (m_cur_version != V_CURRENT)
    {
        AVDEPRECATE("Obsolete cstyle config version");
        AVLogWarning << "Current version is " << V_CURRENT
                     << ", this file is " << m_cur_version << ". Consider -save.";
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readSearchPaths()
{
    for (;;)
    {
        readFacts();
        if (!peekLine().startsWith(SEARCH_TOKEN)) break;

        readLine();
        nextToken(SEARCH_TOKEN, true);
        QString path = readQuoted(false);
        m_client->addSearchPath(path, m_location);
        m_location.m_user_comments.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readIncludes()
{
    for (;;)
    {
        readFacts();
        if (!peekLine().startsWith(INCLUDE_TOKEN)) break;

        readLine();
        nextToken(INCLUDE_TOKEN, true);
        QString include = readQuoted(false);
        m_client->addInclude(include, m_location);
        m_location.m_user_comments.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readMappings()
{
    for (;;)
    {
        readFacts();
        if (!peekLine().startsWith(MAPPING_START_TOKEN)) break;

        readLine();
        nextToken(MAPPING_START_TOKEN, true);
        QString dir = readQuoted(true);

        while (!peekLine().startsWith(MAPPING_END_TOKEN))
        {
            readLine();
            if (m_cur_line.isNull()) fatalError("Missing " + MAPPING_END_TOKEN);
            if (m_cur_line.isEmpty()) continue;
            QString from = nextToken("mapping source");
            QString colon = nextToken(QString::null);
            QString to;
            if (colon == MAP_TO_TOKEN)
            {
                to = nextToken("mapping target");
            } else if (!colon.isEmpty()) fatalError("Unexpected token " + colon);
            m_client->addConfigMapping(from, dir, to, m_location);
            m_location.m_user_comments.clear();
        }

        readLine();
        nextToken(MAPPING_END_TOKEN, true);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readDefines()
{
    for (;;)
    {
        readFacts();
        if (!peekLine().startsWith(DEFINE_TOKEN)) break;

        readLine();
        nextToken(DEFINE_TOKEN, true);
        QString name  = nextToken("define name");
        QString value = nextToken("define value");
        m_client->addDefine(name, value, m_location);
        m_location.m_user_comments.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readOverrides()
{
    for (;;)
    {
        readFacts();
        QString peek = peekLine();
        if (peek.startsWith(SEARCH_TOKEN)             ||
            peek.startsWith(INCLUDE_TOKEN)            ||
            peek.startsWith(MAPPING_START_TOKEN)      ||
            peek.startsWith(SECTION_TOKEN_V1)         ||
            peek.startsWith(SECTION_TOKEN)            ||
            peek.startsWith(FACTS_START_TOKEN)        ||
            peek.startsWith(SUBCONFIG_TEMPLATE_TOKEN) ||
            peek.isEmpty())
        {
            break;;
        }

        if (peek.startsWith(DEFINE_TOKEN))
        {
            fatalError("#define must occur first in a config file (before overrides)!");
        }

        readLine();
        QString name = nextToken("override name");
        if (nextToken(QString::null) != ASSIGN_TOKEN)
        {
            fatalError("Missing " + ASSIGN_TOKEN + " in override for \"" + name + "\"");
        }
        QString value = readValue();
        m_client->addOverride(name, value, m_location);
        m_location.m_user_comments.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readSectionOrFactOrParam(const QString& parent_section)
{
    readFacts(m_current_namespace_level);

    QString line = peekLine();
    QString token = nextTokenFromLine(line);
    if (token == SECTION_TOKEN || token == SECTION_TOKEN_V1)
    {
        readSection(parent_section, false);
    } else
    {
        readParam(parent_section, false);
    }

    readFacts(m_current_namespace_level);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readSection(const QString& parent_section_name, bool is_subconfig_template)
{
    ++m_current_namespace_level;
    readLine();
    QString expected_token = m_cur_version == V_1 ? SECTION_TOKEN_V1 : SECTION_TOKEN;
    nextToken(expected_token, true);

    QString section_name = nextToken("section name");
    if (section_name.contains(".")) fatalError("Section names must not contain '.' character");

    QString full_section_name = parent_section_name;
    if (!full_section_name.isEmpty()) full_section_name += ".";
    full_section_name += section_name;

    // now there are three options: inheritance, opening brace or nothing
    QString next_token = nextToken(QString::null);
    if (next_token == INHERIT_TOKEN)
    {
        if (is_subconfig_template)
        {
            fatalError("Inheritance is not supported for subconfig templates");
        }
        QString inherited_section_name = nextToken("parent section");
        // copy all parameters from inherited section to this section.
        m_client->addInheritedSection(inherited_section_name, full_section_name, m_location);
        next_token = nextToken(QString::null);
    }

    // now there are two options: opening brace or nothing
    if (next_token.isEmpty())
    {
        readLine();
        nextToken(SECTION_START_TOKEN, true);
    } else if (next_token != SECTION_START_TOKEN)
    {
        fatalError("Trailing content after section name: " + next_token + " " + m_cur_line);
    }

    for (;;)
    {
        if (peekLine().isEmpty())
        {
            fatalError("Unexpected end of file encountered while parsing section " + full_section_name);
        } else if (peekLine().startsWith(SECTION_END_TOKEN))
        {
            break;
        } else
        {
            if (is_subconfig_template)
            {
                readSubconfigTemplateSectionOrParam(full_section_name);
            } else
            {
                readSectionOrFactOrParam(full_section_name);
            }
        }
    }
    --m_current_namespace_level;

    // read and check closing "}"
    readLine();
    QString end_token = nextToken(QString::null);
    if (end_token != SECTION_END_TOKEN)
    {
        fatalError("Namespace \"" + full_section_name + "\" is not properly terminated: " + end_token);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readFacts(int namespace_level)
{
    for (;;)
    {
        if (peekLine().startsWith(FACTS_START_TOKEN))
        {
            readLine();
            nextToken(FACTS_START_TOKEN, true);

            AVConfig2StorageLocation::FactCondition condition(namespace_level);
            QStringList condition_list = m_cur_line.split("||");
            for (int c=0; c<condition_list.size(); ++c)
            {
                QString condition_string = condition_list[c].simplified();

                if (!condition_string.contains(FACTS_EQUALS_TOKEN))
                {
                    fatalError("Missing " + FACTS_EQUALS_TOKEN);
                }

                QString fact_category = condition_string.section(FACTS_EQUALS_TOKEN, 0, 0);
                QString fact_value    = condition_string.section(FACTS_EQUALS_TOKEN, 1, 1);

                if (fact_category.isEmpty() || fact_value.isEmpty())
                {
                    fatalError("Missing fact category or value.");
                }
                AVFromString(fact_value, fact_value);
                if (fact_category.contains("&") || fact_category.contains("|") ||
                        fact_value.contains("&")    || fact_value.contains("|"))
                {
                    fatalError("Syntax error");
                }

                if (condition.getFactName().isEmpty())
                {
                    condition.setFactName(fact_category);
                } else if (condition.getFactName() != fact_category)
                {
                    fatalError("You can only specify conditions for one fact category at the same time.");
                }
                condition.addValue(fact_value);
            }

            // Update current fact condition stack
            const AVConfig2StorageLocation::FactConditionContainer& cur_facts = m_location.getFactConditions();
            for (int i=0; i<cur_facts.size(); ++i)
            {
                if (cur_facts[i].getFactName() == condition.getFactName())
                {
                    fatalError("You cannot nest conditions for the same fact category.");
                }
            }
            m_location.pushCondition(condition);
        } else if (peekLine().startsWith(FACTS_END_TOKEN))
        {
            readLine();
            nextToken(FACTS_END_TOKEN, true);
            const AVConfig2StorageLocation::FactConditionContainer cur_facts = m_location.getFactConditions();
            if (cur_facts.isEmpty())
            {
                fatalError("Unmatched " + FACTS_END_TOKEN);
            }
            m_location.popCondition();
        } else break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readParam(const QString& section, bool is_subconfig_template)
{
    if (section.isEmpty())
    {
        fatalError("Parameters must reside within namespaces. Note that overrides must occur before "
                "mappings, search paths, includes and sections.");
    }
    AVConfig2Metadata metadata;
    metadata.setHelp(readHelp());
    readOptionalMetadata(metadata);

    readLine();

    // Work around the fact that the type can consist of multiple tokens. Assume that the last
    // token before the assignment operator is the name.
    QStringList type_and_name;
    for(;;)
    {
        type_and_name << nextToken("parameter type, name and value");
        if (type_and_name.back() == ASSIGN_TOKEN) break;
        if (type_and_name.back().isEmpty())
        {
            fatalError("Missing " + ASSIGN_TOKEN);
        }
        if (type_and_name.back().contains(ASSIGN_TOKEN))
        {
            // this is not great, but at least produce a meaningful error message...
            fatalError("\"" + ASSIGN_TOKEN + "\" must be surrounded by whitespace");
        }
    }
    // pop assignment
    type_and_name.pop_back();

    if (type_and_name.count() < 2)
    {
        fatalError("Expected type and name, found \"" + type_and_name.join(" ") + "\"");
    }

    bool is_ref = false;


    for ( int i = 0; i < type_and_name.size(); i++)
    {
        QString string_incl_ref = type_and_name[i];
        // if the string contains only 1 character (&), theres no need for checking
        if ( string_incl_ref.size() > 1)
        {
            // if the & token is attached to the variable name
            if (string_incl_ref.startsWith(REF_TOKEN))
            {
                type_and_name.insert(i, REF_TOKEN);
                type_and_name[i+1].remove(REF_TOKEN);
                break;
            }
            // if the & token is attached to the type
            else if ( string_incl_ref.endsWith(REF_TOKEN))
            {
                type_and_name[i].remove(REF_TOKEN);
                type_and_name.insert(i + 1, REF_TOKEN);
                break;
            }
        }
    }

    metadata.m_name = section + "." + type_and_name.back();
    type_and_name.pop_back();


    if (type_and_name.back() == REF_TOKEN)
    {
        is_ref = true;
        type_and_name.pop_back();
    }

    // The remaining tokens are the type
    metadata.m_type = type_and_name.join(" ");

    QString value = readValue();

    m_location.m_source = AVConfig2StorageLocation::PS_FILE_ORDINARY;
    if (is_subconfig_template)
    {
        m_client->addSubconfigTemplateParameter(value, metadata, m_location);
    } else if (is_ref)
    {
        m_client->addReference(value, metadata, m_location);
    } else
    {
        m_client->addParameter(value, metadata, m_location);
    }
    m_location.m_source = AVConfig2StorageLocation::PS_UNKNOWN;
    m_location.m_user_comments.clear();
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::readSubconfigTemplate()
{
    readLine();
    QString expected_token = SUBCONFIG_TEMPLATE_TOKEN;
    nextToken(expected_token, true);

    if (!m_location.getFactConditions().empty())
    {
        fatalError("Subconfig template must not be nested within fact conditions");
    }

    QString subconfig_template_name = nextToken("subconfig template name");
    if (subconfig_template_name.endsWith("*"))
    {
        fatalError("Subconfig template name must not end with '*'");
    } else if (subconfig_template_name.endsWith("."))
    {
        fatalError("Subconfig template name must not end with '.'");
    }

    QString next_token = nextToken(QString::null);
    // there are two options: opening brace or nothing
    if (next_token.isEmpty())
    {
        readLine();
        nextToken(SECTION_START_TOKEN, true);
    } else if (next_token != SECTION_START_TOKEN)
    {
        fatalError("Trailing content after subconfig template name: " + next_token + " " + m_cur_line);
    }

    QString subconfig_template_section = subconfig_template_name + ".*";
    for (;;)
    {
        if (peekLine().isEmpty())
        {
            fatalError("Unexpected end of file encountered while parsing subconfig template " + subconfig_template_name);
        } else if (peekLine().startsWith(SECTION_END_TOKEN))
        {
            break;
        } else
        {
            readSubconfigTemplateSectionOrParam(subconfig_template_section);
        }
    }

    // read and check closing "}"
    readLine();
    QString end_token = nextToken(QString::null);
    if (end_token != SECTION_END_TOKEN)
    {
        fatalError("Subconfig template \"" + subconfig_template_name + "\" is not properly terminated: " + end_token);
    }

    return subconfig_template_name;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readSubconfigTemplateSectionOrParam(const QString& parent_section)
{
    QString line = peekLine();
    QString token = nextTokenFromLine(line);
    if (token == FACTS_START_TOKEN)
    {
        fatalError("Subconfig template must not have fact conditions");
    } else if (token == SECTION_TOKEN)
    {
        readSection(parent_section, true);
    } else
    {
        readParam(parent_section, true);
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::readHelp()
{
    QStringList ret;
    for (;;)
    {
        // Bail if there is no meta comment section at all
        if (!peekLine().startsWith(META_COMMENT_TOKEN)) break;
        // Bail if there is no help text, but other available metadata (help must not start with "\", this is
        // enforced at parameter registration time)
        if (ret.isEmpty() && peekLine().startsWith(META_COMMENT_TOKEN + " " + META_PREFIX)) break;

        readLine();
        nextToken(META_COMMENT_TOKEN, true);
        // The help text (if it exists) is separated from the meta data section by a single empty line
        if (m_cur_line.isEmpty()) break;
        ret << m_cur_line;
    }

    if (ret.isEmpty()) return QString::null;
    else return ret.join(" ");
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readOptionalMetadata(AVConfig2Metadata& metadata)
{
    while (peekLine().startsWith(META_COMMENT_TOKEN))
    {
        readLine();
        nextToken(META_COMMENT_TOKEN, true);
        // Do this because we want to preserve whitespace for user comments
        QString line_backup = m_cur_line;
        QString metadata_name = nextToken("meta data type");
        if (metadata_name == META_CMDLINE_TOKEN)
        {
            metadata.m_command_line_switch = m_cur_line;
        } else if (metadata_name == META_SUGGESTED_TOKEN)
        {
            metadata.m_default_value = m_cur_line;
        } else if (metadata_name == META_SUGGESTED_REF_TOKEN)
        {
            metadata.m_suggested_reference = m_cur_line;
        } else if (metadata_name == META_RESTRICTION_TOKEN)
        {
            metadata.m_restriction.reset(new AVConfig2UntypedRestriction(m_cur_line));
        } else if (metadata_name == META_OPTIONAL_TOKEN)
        {
            if (!AVFromString(m_cur_line, metadata.m_optional))
            {
                fatalError("Failed to parse optional state");
            }
        } else if (metadata_name == META_USER_COMMENT_TOKEN)
        {
            AVASSERT(nextTokenFromLine(line_backup, false) == META_USER_COMMENT_TOKEN);
            // This allows user comments to be interleaved with other metadata, which is arguably
            // incorrect, but done for simplicity.
            if (!metadata.m_user_comment.isNull()) metadata.m_user_comment += "\n";
            metadata.m_user_comment += line_backup;
        } else fatalError("Unknown metadata " + metadata_name);
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::readQuoted(bool optional)
{
    QString ret = nextToken(QString::null);
    if (ret.isNull() && optional) return QString::null;
    if (!ret.startsWith(QUOTE_TOKEN) || !ret.endsWith(QUOTE_TOKEN) || ret.length() < 2)
    {
        fatalError("Missing " + QUOTE_TOKEN);
    }
    return ret.mid(1, ret.length()-2);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::fatalError(const QString& message) const
{
    AVLogFatal << "Failed to parse " << (m_file.fileName().isEmpty() ? QString("input string") : m_file.fileName())
               << " at line " << m_cur_line_number
               << ":\n" << message;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::nextToken(QString expected_token, bool exact_match)
{
    // This stays QString::null if we are at the end of the file and m_cur_line is null.
    QString ret;
    if (!m_cur_line.isNull())
    {
        ret = nextTokenFromLine(m_cur_line);
        AVASSERT(!ret.isNull());
    }

    if (!expected_token.isEmpty() && ret.isEmpty())
    {
        // Quote it if it should be an exact match
        if (exact_match) expected_token = "\"" + expected_token + "\"";
        fatalError(QString("Unexpected end of ") + (ret.isNull() ? "file" : "line") +
                   "; expected " + expected_token + ".");
    }
    if (exact_match && ret != expected_token)
    {
        fatalError("\"" + expected_token + "\" expected, \"" + ret + "\" found.");
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::nextTokenFromLine(QString& line, bool strip_spaces)
{
    AVASSERT(!line.isNull());
    QString ret;
    if (!line.contains(" "))
    {
        ret = line;
        line = "";
    } else
    {
        ret = line.section(" ", 0, 0).trimmed();
        line   = line.section(" ", 1, -1);
        if (strip_spaces) line = line.trimmed();
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::peekLine(bool strip_spaces)
{
    if (m_peeked_line.isNull())
    {
        for (;;)
        {
            QString l = m_instream.readLine();
            if (strip_spaces) l = l.trimmed();
            if (l.isNull())
                return l;

            ++m_cur_line_number;
            if (l.isEmpty())
            {
                if(strip_spaces)
                {
                    continue;
                }
                else
                {
                    return l;
                }
            }
            // Don't use nextTokenFromLine for robustness reasons (otherwise a missing space after //
            // would break the parser)
            if (l.trimmed().startsWith(COMMENT_TOKEN) && !l.trimmed().startsWith(META_COMMENT_TOKEN))
            {
                m_location.m_user_comments += l.mid(COMMENT_TOKEN.length()).trimmed();
                continue;
            }
            m_peeked_line = l;
            break;
        }
    }
    return m_peeked_line;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ImporterCstyle::readLine(bool strip_spaces)
{
    if (m_peeked_line.isNull())
        peekLine(strip_spaces);
    else
        AVASSERT(strip_spaces);

    m_cur_line = m_peeked_line;
    m_peeked_line = QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ImporterCstyle::readValue()
{
    QStringList dummy;
    QString value = m_cur_line;

    QString last_correct_line = m_cur_line;

    //! Read lines repeatedly until the nestedSplit() method is successful.
    while (value.trimmed().isEmpty() || // value might start in a new line after "="
           !avfromtostring::nestedSplit(dummy, avfromtostring::SEPARATOR, value, false))
    {
        // Take care not to strip whitespaces from the lines to preserve formatting.
        readLine(false);

        if(m_instream.atEnd())
        {
            static const QString report_pattern("Unexpected end of file while parsing %1 at %2");
            fatalError(report_pattern.arg(m_location.getFullFilePath(), last_correct_line));
        }
        value += "\n" + m_cur_line;
    }
    if (!value.trimmed().endsWith(TERMINATION_TOKEN))
        fatalError("Missing " + TERMINATION_TOKEN);

    value = value.left(value.length()-1);

    return value;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterCstyle::AVConfig2ExporterCstyle() :
        m_export_commented(0),
        m_cur_indent(0),
        m_empty_line_written(false),
        m_pinned_version(AVConfig2ImporterCstyle::V_CURRENT)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addParameter(const QString& value, const AVConfig2Metadata& metadata,
                                           const AVConfig2StorageLocation& location)
{
    addParamOrReference(value, metadata, location, false);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addReference(const QString& ref, const AVConfig2Metadata& metadata,
                                           const AVConfig2StorageLocation& location)
{
    addParamOrReference(ref, metadata, location, true);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addSearchPath(
        const QString& path, const AVConfig2StorageLocation& location)
{
    LoadedSearchPath new_element(path, location);
    for (int i=0; i<m_search_paths.count(); ++i)
    {
        const LoadedSearchPath& cur_element = m_search_paths[i];
        if (cur_element == new_element) return;
    }
    m_search_paths.push_back(new_element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addInclude(const QString& include,
                                         const AVConfig2StorageLocation& location)
{
    LoadedInclude new_element(include, location);
    for (int i=0; i<m_includes.count(); ++i)
    {
        const LoadedInclude& cur_element = m_includes[i];
        if (cur_element == new_element) return;
    }
    m_includes.push_back(new_element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addConfigMapping(
    const QString& map_from, const QString& mapped_dir,
    const QString& mapped_name,
    const AVConfig2StorageLocation& location)
{
    LoadedMapping new_element(map_from, mapped_dir, mapped_name, location);
    for (int i=0; i<m_mappings.count(); ++i)
    {
        const LoadedMapping& cur_element = m_mappings[i];
        if (cur_element == new_element) return;
    }
    m_mappings.push_back(new_element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addOverride(const QString& name, const QString& value,
                                          const AVConfig2StorageLocation& location)
{
    LoadedOverride new_element(name, value, location);
    for (int i=0; i<m_overrides.count(); ++i)
    {
        const LoadedOverride& cur_element = m_overrides[i];
        if (cur_element == new_element) return;
    }
    m_overrides.push_back(new_element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addDefine(const QString& name, const QString& value,
                                        const AVConfig2StorageLocation& location)
{
    LoadedDefine new_element(name, value, location);
    for (int i=0; i<m_defines.count(); ++i)
    {
        const LoadedDefine& cur_element = m_defines[i];
        if (cur_element == new_element) return;
    }
    m_defines.push_back(new_element);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addInheritedSection(const QString &parent, const QString &section,
                                                  const AVConfig2StorageLocation &location)
{
    m_inheritances.push_back(LoadedInheritance(parent, section, location));

    ConditionalSection& conditional_section =
            getOrCreateConditionalSection(location.getFactConditions(), m_top_section);
    getOrCreateSection(section, conditional_section.m_section);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                                            const AVConfig2StorageLocation& location)
{
    int index = metadata.m_name.lastIndexOf(".*.");
    AVASSERT(index != -1);

    // subconfig template name is something like (also see example in confluence doc)
    // "alertchecker.filters.*.regions.*.area"
    // -> split name into subconfig template "namespace" and parameter name, the final ".*." is not part of the
    //    file representation.
    //
    // The example above would result in
    // subconfig_template_name = alertchecker.filters.*.regions
    // parameter_name = area

    QString subconfig_template_name = metadata.m_name.left(index);

    // workaround for SWE-6797:
    // Guard against strange usage of subconfigs (registering nested subconfigs without extending the prefix)
    if (subconfig_template_name.endsWith("*"))
    {
        AVLogWarning << "AVConfig2ExporterCstyle::addSubconfigTemplateParameter: unexpected config structure, "
                     << "not saving subconfig template for " << metadata.m_name;
        return;
    }
    QSharedPointer<SubconfigTemplateSection> subconfig_template_section =
            getOrCreateSubconfigTemplateSection(subconfig_template_name);

    SubconfigTemplateParameterSection::ParamContainer* param_container = &subconfig_template_section->m_section.m_param;

    QString parameter_name = metadata.m_name.mid(index + 3);
    QString name = parameter_name;

    int sep = parameter_name.lastIndexOf(".");
    if (sep != -1)
    {
        QString parameter_namespace = parameter_name.left(sep);
        name = parameter_name.mid(sep + 1);

        SubconfigTemplateParameterSection& param_section =
                getOrCreateSubconfigTemplateParameterSection(parameter_namespace, subconfig_template_section->m_section);

        param_container = &param_section.m_param;
    }

    SubconfigTemplateParameterSection::ParamContainer::value_type new_param_value =
            qMakePair(name, LoadedSubconfigTemplateParameter(value, metadata, location));

    // Find possibly existing parameter (e.g. from pre-loading the config file), and overwrite it
    SubconfigTemplateParameterSection::ParamContainer::iterator existing_position;
    for (existing_position = param_container->begin();
         existing_position != param_container->end();
         ++existing_position)
    {
        const QString& cur_name = existing_position->first;
        if (cur_name != name) continue;

        if (m_order_policy == OP_REPLACE)
        {
            *existing_position = new_param_value;
        }
        else
        {
            param_container->erase(existing_position);
            existing_position = param_container->end();
        }
        break;
    }
    // Param didn't exist yet, or was removed. Add it.
    if (existing_position == param_container->end())
    {
        param_container->append(new_param_value);
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ExporterCstyle::doExport()
{
    m_cur_export = "";
    m_cur_indent = 0;

    writeVersion();
    writeDefines();
    writeOverrides();
    writeMappings();
    writeSearchPaths();
    writeIncludes();
    if (m_pinned_version > AVConfig2ImporterCstyle::V_2)
    {
        writeSubconfigTemplates();
    }
    writeConditionalSection(m_top_section);
    writeLine("// EOF");

    return m_cur_export;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::reset()
{
    AVLogFatal << "TODO CM";
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2ExporterCstyle::removeParameter(
    const QString& full_name, const AVConfig2StorageLocation::FactConditionContainer& facts)
{
    if (!facts.isEmpty())
    {
        AVLogError << "AVConfig2ExporterCstyle::removeParameter: failed to remove " << full_name
                   << " with fact condition " << facts << "; this is a known bug";
        return false;
    }

    QStringList parts = full_name.split(".");
    AVASSERT(parts.count() > 1);
    ParameterSection *cur_section = &m_top_section.m_section;

    // Trace the parameter in our hierarchical sector structure.
    // Remember all touched sections so we can delete them if a parameter was removed and they became empty.
    typedef QVector<QPair<ParameterSection::SubsectionContainer*, QString> > AffectedSectionContainer;
    AffectedSectionContainer affected_sections;

    for (;;)
    {
        // Either the name or the current part of the namespace.
        QString cur_part = parts.first();
        parts.pop_front();

        if (!parts.isEmpty())
        {
            // Find matching section for this namespace part
            bool match = false;
            for (ParameterSection::SubsectionContainer::iterator it = cur_section->m_subsection.begin();
                    it != cur_section->m_subsection.end();
                    ++it)
            {
                QString           cur_subsection_name = it->first;
                ParameterSection& cur_subsection      = *it->second;

                if (cur_subsection_name != cur_part) continue;

                affected_sections.push_back(qMakePair(&cur_section->m_subsection, cur_subsection_name));
                match = true;
                cur_section = &cur_subsection;
                break;
            }
            if (!match) return false;
        } else
        {
            // We have reached the parameter name itself, search the current section for the parameter
            for (ParameterSection::ParamContainer::iterator it = cur_section->m_param.begin();
                    it != cur_section->m_param.end(); ++it)
            {
                const QString& cur_param_name = it->first;
                if (cur_param_name != cur_part) continue;

                // Actually remove the parameter
                cur_section->m_param.erase(it);

                // Clean up sections which became empty
                while (!affected_sections.isEmpty())
                {
                    ParameterSection::SubsectionContainer& cur_subsection_container = *affected_sections.back().first;
                    QString cur_subsection_name                                     =  affected_sections.back().second;
                    ParameterSection::SubsectionContainer::iterator it = cur_subsection_container.begin();
                    while (it != cur_subsection_container.end() && it->first != cur_subsection_name) ++it;
                    AVASSERT(it != cur_subsection_container.end());
                    const ParameterSection& cur_section = *it->second;
                    if (cur_section.m_param.isEmpty() && cur_section.m_subsection.isEmpty())
                    {
                        cur_subsection_container.erase(it);
                    } else break; // if this one isn't empty and wasn't removed, all following sections won't be empty.
                    affected_sections.pop_back();
                }
                return true;

            }
            return false;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::setPinnedVersion(int version)
{
    AVASSERT(version >= AVConfig2ImporterCstyle::V_2);
    m_pinned_version = version;
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterCstyle::ParamOrReference::ParamOrReference() : m_is_ref(false)
{

}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterCstyle::ParamOrReference::ParamOrReference(
        const QString& value, const AVConfig2Metadata& metadata, bool is_ref, const AVConfig2StorageLocation& location) :
        m_value(value),
        m_is_ref(is_ref),
        m_metadata(metadata),
        m_location(location)
{
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::addParamOrReference(
        const QString& value, const AVConfig2Metadata& metadata,
        const AVConfig2StorageLocation& location, bool is_ref)
{
    // Exclude parameters that were removed from the config.
    if (m_removed_param_names.contains(metadata.m_name))
    {
        return;
    }

    // First create / find the conditional section this parameter should go in
    ConditionalSection& conditional_section =
            getOrCreateConditionalSection(location.getFactConditions(), m_top_section);

    // Then, create / find the section to which our param should be added
    int sep = metadata.m_name.lastIndexOf(".");
    if (sep == -1)
    {
        AVLogFatal << "AVConfig2ExporterCstyle::addParamOrReference: "
                   << "Parameters must reside in a section ("
                   << metadata << ")";
    }
    QString section_name = metadata.m_name.left(sep);
    QString name         = metadata.m_name.mid (sep+1);

    ParameterSection::ParamContainer& param_container =
            getOrCreateSection(section_name, conditional_section.m_section).m_param;

    ParameterSection::ParamContainer::value_type new_param_value =
            qMakePair(name, ParamOrReference(value, metadata, is_ref, location));

    // Find possibly existing parameter (e.g. from pre-loading the config file), and overwrite it
    ParameterSection::ParamContainer::iterator existing_position;
    for (existing_position = param_container.begin();
         existing_position != param_container.end();
         ++existing_position)
    {
        const QString& cur_name = existing_position->first;
        if (cur_name != name) continue;

        if (m_order_policy == OP_REPLACE)
        {
            *existing_position = new_param_value;
        } else
        {
            param_container.erase(existing_position);
            existing_position = param_container.end();
        }
        break;
    }
    // Param didn't exist yet, or was removed. Add it.
    if (existing_position == param_container.end())
    {
        param_container.push_back(new_param_value);
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterCstyle::ConditionalSection& AVConfig2ExporterCstyle::getOrCreateConditionalSection(
            AVConfig2StorageLocation::FactConditionContainer conditions, ConditionalSection& parent)
{
    if (conditions.isEmpty()) return parent;
    AVConfig2StorageLocation::FactCondition condition = conditions.front();
    conditions.pop_front();

    ConditionalSection *sub_section = 0;
    for (int i=0; i<parent.m_sub_conditions.size(); ++i)
    {
        if (parent.m_sub_conditions[i].first == condition)
        {
            sub_section = parent.m_sub_conditions[i].second.data();
        }
    }
    if (sub_section == 0)
    {
        sub_section = new (LOG_HERE) ConditionalSection;
        QSharedPointer<ConditionalSection> section_pointer(sub_section);
        parent.m_sub_conditions.push_back(qMakePair(condition, section_pointer));
    }
    return getOrCreateConditionalSection(conditions, *sub_section);
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterCstyle::ParameterSection& AVConfig2ExporterCstyle::getOrCreateSection(
    const QString& name, ParameterSection& parent)
{
    // Recursively create nested sections
    if (name.contains('.'))
    {
        ParameterSection& next_parent = getOrCreateSection(name.section('.', 0, 0), parent);
        return getOrCreateSection(name.section('.', 1), next_parent);
    } else
    {
        for (ParameterSection::SubsectionContainer::iterator it = parent.m_subsection.begin();
             it != parent.m_subsection.end(); ++it)
        {
            const QString&    cur_name    = it->first;
            ParameterSection& cur_section = *it->second;
            if (cur_name == name) return cur_section;
        }
        ParameterSection *new_section = new (LOG_HERE) ParameterSection;
        QSharedPointer<ParameterSection> section_pointer(new_section);
        parent.m_subsection.push_back(qMakePair(name, section_pointer));
        return *new_section;
    }
}

///////////////////////////////////////////////////////////////////////////////

AVConfig2ExporterCstyle::SubconfigTemplateParameterSection& AVConfig2ExporterCstyle::getOrCreateSubconfigTemplateParameterSection(
        const QString& name, AVConfig2ExporterCstyle::SubconfigTemplateParameterSection& parent)
{
    // Recursively create nested sections
    if (name.contains('.'))
    {
        SubconfigTemplateParameterSection& next_parent = getOrCreateSubconfigTemplateParameterSection(name.section('.', 0, 0), parent);
        return getOrCreateSubconfigTemplateParameterSection(name.section('.', 1), next_parent);
    }
    else
    {
        for (SubconfigTemplateParameterSection::SubsectionContainer::iterator it = parent.m_subsection.begin();
             it != parent.m_subsection.end(); ++it)
        {
            const QString&    cur_name    = it->first;
            SubconfigTemplateParameterSection& cur_section = *it->second;
            if (cur_name == name) return cur_section;
        }
        SubconfigTemplateParameterSection *new_section = new (LOG_HERE) SubconfigTemplateParameterSection;
        QSharedPointer<SubconfigTemplateParameterSection> section_pointer(new_section);
        parent.m_subsection.append(qMakePair(name, section_pointer));
        return *new_section;
    }
}

///////////////////////////////////////////////////////////////////////////////

QSharedPointer<AVConfig2ExporterCstyle::SubconfigTemplateSection> AVConfig2ExporterCstyle::getOrCreateSubconfigTemplateSection(const QString& name)
{
    for (QSharedPointer<SubconfigTemplateSection>& section : m_subconfig_template_sections)
    {
        if (section->m_name == name)
        {
            return section;
        }
    }
    QSharedPointer<SubconfigTemplateSection> section(new SubconfigTemplateSection(name));
    m_subconfig_template_sections.append(section);
    return section;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeVersion()
{
    writeLine(VERSION_TOKEN + " " + QString("%1").arg(m_pinned_version));
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeSearchPaths()
{
    writeLine();
    for (SearchPathContainer::const_iterator it = m_search_paths.begin();
         it != m_search_paths.end();
         ++it)
    {
        const LoadedSearchPath& cur_path = *it;
        establishPreconditions(cur_path.getLocation());
        writeLine(SEARCH_TOKEN + " " + QUOTE_TOKEN + cur_path.getPath() + QUOTE_TOKEN);
    }

    establishPreconditions(AVConfig2StorageLocation());
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeIncludes()
{
    writeLine();
    for (IncludeContainer::const_iterator it = m_includes.begin();
         it != m_includes.end();
         ++it)
    {
        const LoadedInclude& cur_include = *it;
        establishPreconditions(cur_include.getLocation());
        writeLine(INCLUDE_TOKEN + " " + QUOTE_TOKEN + cur_include.m_include + QUOTE_TOKEN);
    }

    establishPreconditions(AVConfig2StorageLocation());
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeMappings()
{
    // Group by mapping dir and fact condition
    typedef QMap<QPair<QString, QString>, MappingContainer> GroupedMappingContainer;
    GroupedMappingContainer grouped_mappings;

    writeLine();

    for (MappingContainer::const_iterator it = m_mappings.begin();
         it != m_mappings.end();
         ++it)
    {
        const LoadedMapping& cur_mapping = *it;
        QString fact_condition_string = AVToString(cur_mapping.getLocation().getFactConditions());
        grouped_mappings[qMakePair(cur_mapping.m_mapped_dir, fact_condition_string)].push_back(
            LoadedMapping(cur_mapping.m_from, cur_mapping.m_mapped_dir,
                          cur_mapping.m_mapped_name, cur_mapping.getLocation()));
    }

    for (GroupedMappingContainer::const_iterator group_it = grouped_mappings.begin();
            group_it != grouped_mappings.end();
            ++group_it)
    {
        writeLine();
        const MappingContainer& cur_container = group_it.value();

        // Take dir and facts from first entry...
        establishPreconditions(cur_container.first().getLocation());

        QString dir = cur_container.first().m_mapped_dir;
        if (!dir.isNull()) dir = SPACE_TOKEN + QUOTE_TOKEN + dir + QUOTE_TOKEN;
        writeLine(MAPPING_START_TOKEN + dir);

        // Now write out all mappings which belong to this group
        for (MappingContainer::const_iterator map_it = group_it.value().begin();
                map_it != group_it.value().end();
                ++map_it)
        {
            const QString& cur_from = map_it->m_from;
            const QString& cur_to   = map_it->m_mapped_name;

            QString l = cur_from;
            if (!cur_to.isNull()) l += SPACE_TOKEN + MAP_TO_TOKEN + SPACE_TOKEN + cur_to;
            writeLine(l);
        }

        if (dir.isNull())
        {
            writeLine(MAPPING_END_TOKEN);
        } else
        {
            writeLine(MAPPING_END_TOKEN + SPACE_TOKEN + COMMENT_TOKEN + dir);
        }
    }

    establishPreconditions(AVConfig2StorageLocation());
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeDefines()
{
    writeLine();
    for (DefineContainer::const_iterator it = m_defines.begin();
         it != m_defines.end();
         ++it)
    {
        const LoadedDefine& cur_define = *it;
        establishPreconditions(cur_define.getLocation());
        writeLine(DEFINE_TOKEN + " " + cur_define.m_name + " " + cur_define.m_value);
    }

    establishPreconditions(AVConfig2StorageLocation());
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeOverrides()
{
    writeLine();
    QString prev_prefix;
    for (OverrideContainer::const_iterator it = m_overrides.begin();
         it != m_overrides.end();
         ++it)
    {
        const LoadedOverride& cur_override = *it;

        QString cur_prefix = cur_override.m_name.section('.', 0, 0);
        if (cur_prefix != prev_prefix)
        {
            writeLine();
            prev_prefix = cur_prefix;
        }

        establishPreconditions(cur_override.getLocation());
        writeLine(cur_override.m_name + " " + ASSIGN_TOKEN + " " +
                  cur_override.m_value + TERMINATION_TOKEN);

    }

    establishPreconditions(AVConfig2StorageLocation());
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeSubconfigTemplates()
{
    writeLine();
    for (SubconfigTemplateSectionContainer::const_iterator it =
            m_subconfig_template_sections.begin();
         it != m_subconfig_template_sections.end();
         ++it)
    {
        QSharedPointer<SubconfigTemplateSection> subconfig_template_section = *it;
        writeLine(COMMENT_TOKEN +
                  " -------------------------------------------------------------");
        writeLine(SUBCONFIG_TEMPLATE_TOKEN + " " + subconfig_template_section->m_name);
        writeLine(SECTION_START_TOKEN);

        ++m_cur_indent;

        writeSubconfigTemplateParameterSection(subconfig_template_section->m_section);

        --m_cur_indent;

        writeLine(SECTION_END_TOKEN + " " + COMMENT_TOKEN + " " +
                  SUBCONFIG_TEMPLATE_TOKEN + " " + subconfig_template_section->m_name);
        writeLine();
    }
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeSubconfigTemplateParameterSection(const SubconfigTemplateParameterSection& section)
{
    // Write subconfig template section parameters
    for (SubconfigTemplateParameterSection::ParamContainer::const_iterator param_it =
             section.m_param.begin();
         param_it != section.m_param.end();
         ++param_it)
    {
        const QString&                          cur_name         = param_it->first;
        const LoadedSubconfigTemplateParameter& cur_loaded_param = param_it->second;
        writeSubconfigTemplateParam(cur_name, cur_loaded_param);
    }

    // Write subconfig template section subsections
    for (SubconfigTemplateParameterSection::SubsectionContainer::const_iterator section_it =
             section.m_subsection.begin();
         section_it != section.m_subsection.end();
         ++section_it)
    {
        const QString&                           cur_section_name = section_it->first;
        const SubconfigTemplateParameterSection& cur_section      = *section_it->second;

        writeLine(COMMENT_TOKEN +
                  " -------------------------------------------------------------");
        writeLine(SECTION_TOKEN + " " + cur_section_name);
        writeLine(SECTION_START_TOKEN);
        ++m_cur_indent;

        writeSubconfigTemplateParameterSection(cur_section);

        --m_cur_indent;
        writeLine(SECTION_END_TOKEN + " " + COMMENT_TOKEN + " " +
                  SECTION_TOKEN + " " + cur_section_name);

        writeLine();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeSubconfigTemplateParam(const QString& name, const LoadedSubconfigTemplateParameter& param)
{
    const AVConfig2Metadata& metadata = param.getMetadata();
    writeMetadata(metadata);

    QString one_line = metadata.m_type + SPACE_TOKEN;
    one_line += name + SPACE_TOKEN + ASSIGN_TOKEN;

    if (!param.getValue().startsWith(END_OF_LINE_TOKEN))
    {
        one_line += SPACE_TOKEN;
    }

    one_line += param.getValue() + TERMINATION_TOKEN;

    writeLine(one_line);
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeConditionalSection(const ConditionalSection& section)
{
    AVASSERT(section.m_section.m_param.isEmpty());

    writeSection(section.m_section);

    for (ConditionalSection::SubConditionContainer::const_iterator it =
            section.m_sub_conditions.begin();
         it != section.m_sub_conditions.end();
         ++it)
    {
        writeLine(FACTS_START_TOKEN + " " + factConditionToString(it->first));
        m_fact_condition_stack.push_back(it->first);
        writeLine();
        writeConditionalSection(*it->second);
        writeLine(FACTS_END_TOKEN + " // " + factConditionToString(it->first));
        m_fact_condition_stack.pop_back();
        writeLine();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeSection(const ParameterSection& section)
{
    QString ret;
    QTextStream stream(&ret, QIODevice::WriteOnly);
    for (ParameterSection::SubsectionContainer::const_iterator section_it =
             section.m_subsection.begin();
         section_it != section.m_subsection.end();
         ++section_it)
    {
        const QString&          cur_section_name = section_it->first;
        const ParameterSection& cur_section      = *section_it->second;

        m_section_name_stack.push_back(cur_section_name);

        QString inherit_string;
        QString full_section_name = m_section_name_stack.join(".");

        for (const LoadedInheritance& cur_inheritance : qAsConst(m_inheritances))
        {
            if (cur_inheritance.m_section != full_section_name) continue;
            if (cur_inheritance.getLocation().getFactConditions() != m_fact_condition_stack) continue;
            inherit_string = " : " + cur_inheritance.m_parent;
            break;
        }

        writeLine(COMMENT_TOKEN +
                  " -------------------------------------------------------------");
        writeLine(SECTION_TOKEN + " " + cur_section_name + inherit_string);
        writeLine(SECTION_START_TOKEN);
        ++m_cur_indent;
        for (ParameterSection::ParamContainer::const_iterator param_it =
                 cur_section.m_param.begin();
             param_it != cur_section.m_param.end();
             ++param_it)
        {
            const QString&          cur_name         = param_it->first;
            const ParamOrReference& cur_loaded_param = param_it->second;
            writeParam(cur_name, cur_loaded_param);
        }

        writeSection(cur_section);

        --m_cur_indent;
        writeLine(SECTION_END_TOKEN + " " + COMMENT_TOKEN + " " +
                  SECTION_TOKEN + " " + cur_section_name);

        m_section_name_stack.pop_back();

        writeLine();
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeParam(const QString& name, const ParamOrReference& param)
{
    const AVConfig2Metadata& metadata   = param.m_metadata;

    writeMetadata(metadata);

    QString one_line = metadata.m_type + SPACE_TOKEN;
    if (param.m_is_ref)
    {
        one_line += REF_TOKEN + SPACE_TOKEN;
    }

    one_line += name + SPACE_TOKEN + ASSIGN_TOKEN;

    if (!param.m_value.startsWith(END_OF_LINE_TOKEN))
    {
        one_line += SPACE_TOKEN;
    }

    one_line += param.m_value + TERMINATION_TOKEN;

    writeLine(one_line);
    writeLine();
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeMetadata(const AVConfig2Metadata& metadata)
{
    QStringList metadata_lines;

    if (!metadata.m_default_value.isNull())
    {
        metadata_lines <<
                (META_COMMENT_TOKEN + " " + META_SUGGESTED_TOKEN + " " + metadata.m_default_value);
    }
    if (!metadata.m_suggested_reference.isNull())
    {
        metadata_lines <<
                (META_COMMENT_TOKEN + " " + META_SUGGESTED_REF_TOKEN + " " + metadata.m_suggested_reference);
    }
    if (!metadata.m_command_line_switch.isNull())
    {
        metadata_lines <<
                (META_COMMENT_TOKEN + " " + META_CMDLINE_TOKEN + " " +
                        metadata.m_command_line_switch);
    }
    if (metadata.m_optional)
    {
        metadata_lines <<
                (META_COMMENT_TOKEN + " " + META_OPTIONAL_TOKEN + " " +
                 QString("%1").arg(metadata.m_optional));
    }
    if (metadata.m_restriction != 0)
    {
        metadata_lines <<
                (META_COMMENT_TOKEN + " " + META_RESTRICTION_TOKEN + " " +
                        metadata.m_restriction->toString());
    }
    if (!metadata.m_user_comment.isNull())
    {
        QStringList lines = metadata.m_user_comment.split("\n");
        AVASSERT(!lines.isEmpty());
        for (int l=0; l<lines.size(); ++l)
        {
            metadata_lines << (META_COMMENT_TOKEN + " " + META_USER_COMMENT_TOKEN);
            if (!lines[l].isEmpty()) metadata_lines.back() += " " + lines[l];
        }
    }

    QStringList broken_help = AVBreakString(
            metadata.getHelp(), MAX_HELP_LENGTH - m_cur_indent*INDENT - 4, true);
    for (int h=0; h<broken_help.size(); ++h)
    {
        writeLine(META_COMMENT_TOKEN + " " + broken_help[h]);
    }

    // Separate metadata lines from help text by empty newline
    if (!broken_help.isEmpty() && !metadata_lines.isEmpty()) writeLine(META_COMMENT_TOKEN);
    for (QStringList::const_iterator it = metadata_lines.begin(); it != metadata_lines.end(); ++it)
    {
        writeLine(*it);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::writeLine(const QString& line)
{
    if (line.isEmpty())
    {
        if (!m_empty_line_written)
        {
            if (m_export_commented != 0) m_cur_export += "//"; // avoid trailing whitespace on empty line
            m_cur_export += "\n";
        }
        m_empty_line_written = true;
    } else
    {
        if (m_export_commented != 0) m_cur_export += "// ";
        m_empty_line_written = false;
        m_cur_export += QString().fill(' ', m_cur_indent*INDENT) + line + "\n";
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2ExporterCstyle::establishPreconditions(const AVConfig2StorageLocation& location)
{
    const AVConfig2StorageLocation::FactConditionContainer& facts = location.getFactConditions();

    // First, find the common starting ground by comparing conditions front-to-back
    int start_pos = 0;
    while (start_pos < m_fact_condition_stack.count() &&
           start_pos < facts.count()                  &&
           m_fact_condition_stack[start_pos] == facts[start_pos])
    {
        ++start_pos;
    }

    // Then, pop any conditions which currently are in mismatch
    int num_conditions_to_pop = m_fact_condition_stack.count() - start_pos;
    for (int i=0; i<num_conditions_to_pop; ++i)
    {
        writeLine(FACTS_END_TOKEN + " // " + factConditionToString(m_fact_condition_stack.back()));
        writeLine();
        m_fact_condition_stack.pop_back();
    }

    // Finally, push the additionally required conditions
    for (int i=start_pos; i<facts.count(); ++i)
    {
        m_fact_condition_stack.push_back(facts[i]);
        writeLine();
        writeLine(FACTS_START_TOKEN + " " + factConditionToString(facts[i]));
    }

    const QStringList user_comments = location.m_user_comments;
    for (int i=0; i<user_comments.count(); ++i)
    {
        writeLine(COMMENT_TOKEN + " " + user_comments[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfig2ExporterCstyle::factConditionToString(
        const AVConfig2StorageLocation::FactCondition& condition) const
{
    AVConfig2StorageLocation::FactConditionContainer conjunction;
    conjunction.push_back(condition);
    return AVConfig2StorageLocation::getFactConditionString(conjunction);
}


// End of file
