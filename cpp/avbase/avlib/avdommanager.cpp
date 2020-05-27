///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Anton Skrabal, a.skrabal@avibit.com
    \brief
*/

// QT includes
#include <qglobal.h>

// local includes
#include "avdommanager.h"
#include "avlog.h"
#include "avdatetime.h"

///////////////////////////////////////////////////////////////////////////////

#include "avdommanager.h"

AVDomManager::AVDomManager(QSharedPointer<AVDomManagerConfig> config) : m_cfg(config), m_dom_doc_loaded(false)
{
}

///////////////////////////////////////////////////////////////////////////////

AVDomManager::~AVDomManager()
{
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::loadDomDocument(const QString& full_filename)
{
    AVLogDebug << "AVDomManager::loadDomDocument: " << full_filename;

    m_full_filename = full_filename;

    QFile infile(full_filename);

    if (!infile.open(QIODevice::ReadOnly))
    {
        AVLogInfo << "AVDomManager::loadDomDocument: "
                "Cannot read file " << full_filename << ": " << infile.errorString() <<
                ", creating anew.";

        infile.open(QIODevice::ReadWrite);
        QDomNode xmlNode = m_dom_doc.createProcessingInstruction("xml",
                                                                 "version=\"1.0\""
                                                                 " encoding=\"ISO-8895-1\"");
        m_dom_doc.insertBefore(xmlNode, m_dom_doc.firstChild());

        QDomElement root = m_dom_doc.createElement(m_cfg->m_root_entry_name);
        m_dom_doc.appendChild(root);

        writeDomDocToFile(full_filename);
    }

    QString error_msg;
    int line, column = 0;
    if (!m_dom_doc.setContent(&infile, &error_msg, &line, &column))
    {
        AVLogError <<  "AVDomManager::loadDomDocument: "
                "unable to load xml file into DomDoc: " << infile.fileName() << " error: " <<
                error_msg << " at line: " << line << " column: " << column;
        return false;
    }
    else
    {
        AVLogDebug <<  "AVDomManager::loadDomDocument: "
                "XML file loaded into domdoc: " + infile.fileName();
    }

    QDomElement root = m_dom_doc.documentElement();
    if (root.tagName() != m_cfg->m_root_entry_name)
    {
        AVLogError <<  "AVDomManager::loadDomDocument: "
                "Unexpected root entry found: " << root.tagName() << ", expected: "
                << m_cfg->m_root_entry_name;
        return false;
    }

    if (!checkForConsistency(root))
    {
        AVLogError <<  "AVDomManager::loadDomDocument: "
                "Check for Consistency failed";
        return false;
    }

    m_dom_doc_loaded = true;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::checkForConsistency(const QDomNode& current_element)
{
    QDomNode child = current_element.firstChild();

    bool check_ok = true;

    while (!child.isNull())
    {
        if (child.toElement().tagName() == m_cfg->m_date_entry_name)
        {
            check_ok &= checkForSubEntryConsistency(child);
        }
        else
        {
            AVLogError <<  "AVDomManager::parseDateEntries: "
                    "Unknown subitem found: %s" + child.toElement().tagName();
            return false;
        }

        child = child.nextSibling();
    }

    return check_ok;
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::checkForSubEntryConsistency(const QDomNode& current_element)
{
    QDomNode child = current_element.firstChild();

    bool check_ok = true;

    while (!child.isNull())
    {
        if (child.toElement().tagName() == m_cfg->m_main_sub_entry_name)
        {
            check_ok &= checkForStartEndConsistency(child);
        }
        else
        {
            AVLogError <<  "AVDomManager::parseSubEntries: "
                    "Unknown subitem found: %s" + child.toElement().tagName();
            return false;
        }

        child = child.nextSibling();
    }

    return check_ok;
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::checkForStartEndConsistency(const QDomNode& current_element)
{
    if (current_element.toElement().tagName() != m_cfg->m_start_timestamp_entry_name &&
            current_element.toElement().attribute(m_cfg->m_start_timestamp_entry_name).isNull())
    {
        AVLogError <<  "AVDomManager::parseStartAndEndEntries: "
                "Could not find start entry in: " << current_element.toElement().tagName();
        return false;
    }

    if (current_element.toElement().tagName() != m_cfg->m_end_timestamp_entry_name &&
            current_element.toElement().attribute(m_cfg->m_end_timestamp_entry_name).isNull())
    {
        AVLogError <<  "AVDomManager::parseStartAndEndEntries: "
                "Could not find end entry in: " << current_element.toElement().tagName();
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::addEntry(const QDateTime& start, const QDateTime& end, const QString& entryname,
                            const QString& attribute, const QString& text)
{
    if (!m_dom_doc_loaded)
    {
        AVLogDebug << "AVDomManager::addEntry: adding xml header";
        QDomNode xmlNode = m_dom_doc.createProcessingInstruction("xml",
                                                                 "version=\"1.0\""
                                                                 " encoding=\"ISO-8895-1\"");
        m_dom_doc.insertBefore(xmlNode, m_dom_doc.firstChild());

        QDomElement root = m_dom_doc.createElement(m_cfg->m_root_entry_name);
        m_dom_doc.appendChild(root);

        m_dom_doc_loaded = true;
    }

    QDomElement root = m_dom_doc.documentElement();

    QDate date = start.date();
    QDomElement date_entry = getDateEntry(m_dom_doc, date);
    if (date_entry.attribute(m_cfg->m_date_attrib_name).isNull())
        root.appendChild(date_entry);

    date_entry.setAttribute(m_cfg->m_date_attrib_name, date.toString("yyyy-MM-dd"));

    QDomElement current = m_dom_doc.createElement(entryname);
    current.setAttribute(m_cfg->m_start_timestamp_entry_name,
                         start.toString("yyyy-MM-dd hh:mm:ss"));
    current.setAttribute(m_cfg->m_end_timestamp_entry_name,
                         end.toString("yyyy-MM-dd hh:mm:ss"));
    current.setAttribute(attribute, text);

    date_entry.appendChild(current);

    if (!date_entry.childNodes().isEmpty())
    {
        // check for same entry, return false if entry already available!?
    }
    else
    {

    }


    return true;
}

///////////////////////////////////////////////////////////////////////////////

QDomElement AVDomManager::getDateEntry(QDomDocument& doc, const QDate& date)
{
    AVLogDebug <<  "AVDomManager::getDateEntry";
    QDomNodeList dates = doc.elementsByTagName(m_cfg->m_date_entry_name);
    for (int date_index = 0; date_index < dates.count(); date_index++)
    {
        QDomElement element = dates.at(date_index).toElement();
        QDate file_date = QDate::fromString(element.attribute(m_cfg->m_date_attrib_name),
                                            Qt::ISODate);

        AVLogInfo << "AVDomManager::getDateEntry: D date: " << date.toString("yyyy-MM-dd") <<
                " filedate: " << file_date.toString("yyyy-MM-dd");

        if (date == file_date)
        {
            AVLogInfo << "AVDomManager::getDateEntry: found date entry.";
            return dates.at(date_index).toElement();
        }
    }
    AVLogInfo << "AVDomManager::getDateEntry: new date entry";
    return doc.createElement(m_cfg->m_date_entry_name);
}

///////////////////////////////////////////////////////////////////////////////

QStringList AVDomManager::searchForText(const QString& text, const QString& entryname,
                                        const QString& attrib)
{
    QStringList results;
    QDomNodeList entries = m_dom_doc.elementsByTagName(entryname);
    for (int entry_index = 0; entry_index < entries.count(); entry_index++)
    {
        QDomElement element = entries.at(entry_index).toElement();
        QString element_attribute = element.attribute(attrib);
        QString start = element.attribute(m_cfg->m_start_timestamp_entry_name);
        QString end = element.attribute(m_cfg->m_end_timestamp_entry_name);

        AVLogDebug << "AVDomManager::searchForText: " << text << ", start: " << start << ", end: "
                << end;

        if (element_attribute.contains(text, Qt::CaseInsensitive) ||
                text.contains(element_attribute, Qt::CaseInsensitive) ||
                start.contains(text, Qt::CaseInsensitive) ||
                end.contains(text, Qt::CaseInsensitive))
        {
            QString result(start.append(";").append(end).append(";").append(element_attribute));
            results.append(result);
        }
    }
    return results;
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::removeEntriesOlderThan(const QDateTime& timestamp)
{
    QDomNodeList dates = m_dom_doc.elementsByTagName(m_cfg->m_date_entry_name);
    AVLogInfo <<  "AVDomManager::removeEntriesOlderThan: "
               "dates size: " << QString::number(dates.size());

    for (int date_index = dates.count()-1; date_index >= 0; date_index--)
    {
        QDomElement element = dates.at(date_index).toElement();
        QDate date =
                QDate::fromString(element.attribute(m_cfg->m_date_attrib_name), Qt::ISODate);
        if (date > timestamp.date())
        {
            AVLogInfo <<  "AVDomManager::removeEntriesOlderThan: skipping: " <<
                    date.toString("yyyy-MM-dd");
            continue;
        }

        QDomNodeList subentries = element.elementsByTagName(m_cfg->m_main_sub_entry_name);
        AVLogInfo <<  "AVDomManager::removeEntriesOlderThan: "
                   "subentries size: " << QString::number(subentries.size());

        for (int sub_entry_index = subentries.count()-1; sub_entry_index >= 0; sub_entry_index--)
        {
            QDomElement sub_entry = subentries.at(sub_entry_index).toElement();
            QString endtime_string = sub_entry.attribute(m_cfg->m_end_timestamp_entry_name);
            if (!endtime_string.isEmpty())
            {
                QDateTime current_dt =
                        AVDateTime::fromString(endtime_string, "yyyy-MM-dd hh:mm:ss");
                if (current_dt < timestamp)
                {
                    AVLogInfo <<  "AVDomManager::removeEntriesOlderThan: removing: " <<
                            current_dt.toString("yyyy-MM-dd hh:mm:ss");
                    dates.at(date_index).removeChild(subentries.at(sub_entry_index));
                    //entries_to_delete.append(dates.at(date_index));
                }
            }
        }

        if (element.elementsByTagName(m_cfg->m_main_sub_entry_name).size() == 0)
        {
            AVLogInfo <<  "AVDomManager::removeEntriesOlderThan: removing "
                    << m_cfg->m_main_sub_entry_name << ": " << date.toString("yyyy-MM-dd");
            m_dom_doc.documentElement().removeChild(dates.at(date_index));
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool AVDomManager::writeDomDocToFile(const QString& filename)
{
    AVLogInfo << "AVDomManager::writeDomDocToFile: writing full domdoc to: " << filename;

    QFile outfile(filename);
    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        AVLogError <<  "AVDomManager::writeDomDocToFile: Cannot open file " <<
                outfile.fileName() << ": " << outfile.errorString();
        return false;
    }

    QTextStream out(&outfile);
    m_dom_doc.save(out, 2);
    AVLogDebug << "AVDomManager::writeDomDocToFile: file written to: " << outfile.fileName();
    return true;
}


// End of file
