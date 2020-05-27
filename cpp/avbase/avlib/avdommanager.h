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
    \brief   This class is a XML helper class, making it possible to delete XML entries from
             specially formed XML files older than a certain timestamp. This is mainly used for
             FileIndexing (avarchlib/archfileinfo), but also for descriptions for replay
             episodes in the RepMax. Could be used for much more.
*/

#if !defined AVDOMMANAGER_H_INCLUDED
#define AVDOMMANAGER_H_INCLUDED

#include <QString>
#include <QDomElement>

#include "avlib_export.h"
#include "avconfig2.h"

//! The config for the AVDomManager


class AVLIB_EXPORT AVDomManagerConfig : public AVConfig2
{
public:
    explicit AVDomManagerConfig(const QString &name = "avdommanagerconfig") :
        AVConfig2(name)
    {
        setHelpGroup(name);
        // create config entries

        registerParameter("root_entry", &m_root_entry_name,
                          "the root entry name of the XML file to handle").
                setSuggestedValue("fileindex").setCmdlineSwitch("root");

        registerParameter("date_entry", &m_date_entry_name,
                          "the date entry name of the XML file to handle").
                setSuggestedValue("dateentry").setCmdlineSwitch("date");

        registerParameter("date_attrib", &m_date_attrib_name,
                          "the date attribute name of the XML file to handle").
                setSuggestedValue("date").setCmdlineSwitch("dateattrib");

        registerParameter("sub_entry", &m_main_sub_entry_name,
                          "the main sub entry name of the XML file to handle").
                setSuggestedValue("fileentry").setCmdlineSwitch("subentry");

        registerParameter("start_entry", &m_start_timestamp_entry_name,
                          "the start timestamp entry name of the XML file to handle").
                setSuggestedValue("starttime").setCmdlineSwitch("start");

        registerParameter("end_entry", &m_end_timestamp_entry_name,
                          "the end timestamp entry name of the XML file to handle").
                setSuggestedValue("endtime").setCmdlineSwitch("end");

        AVConfig2Container::RefreshResult result;
        if (!refreshParams(&result))
        {
            AVLogInfo << "AVDomManagerConfig: failed, error: " << result.getErrorMessage();
        }
    };

    QString m_root_entry_name;
    QString m_date_entry_name;
    QString m_date_attrib_name;
    QString m_main_sub_entry_name;
    QString m_start_timestamp_entry_name;
    QString m_end_timestamp_entry_name;
};

// ---------------------------------------------------------------------------
//! QDomDocument Manager for XML Files
/*!  This manager allows to automatically delete entries from a QDomDocument of the structure
   \verbatim
    <?xml version = '1.0' encoding = 'utf-8'?>
    <root>
      <date_entry date="YYYY-MM-DD">
        <main_sub start="YYYY-MM-DDThh:mm:dd.zzz" end="YYYY-MM-DDThh:mm:dd.zzz" [...]>
          [...]
        </main_sub>
      <\date>
    <\root>
   \endverbatim
   where root can be configured by the AVDomManagerConfig's root_entry, date_entry by the config's
   date_entry, date by the config's date_attrib, main_sub by the config's sub_entry and start &
   end by the config's start_entry and end_entry respectively.

   It checks for correctness of the XML file during loading and returns false, if loading fails.

   Additionally, it is capable of writing the file to a file with a given name using the
   writeDomDocToFile(...) method.
 */

class AVLIB_EXPORT AVDomManager
{
public:
    //! constructor, takes the config as a parameter
    explicit AVDomManager(QSharedPointer<AVDomManagerConfig> config);
    //! destructor
    virtual ~AVDomManager();

    //! Main loading function. Takes a relative filename to load the given file.
    //! This method checks for consistency of the given file using the private methods
    //! checkForConsistency(...), checkForSubEntryConsistency(...) and
    //! checkForStartEndConsistency(...). Returns true upon success, false upon failure.
    bool loadDomDocument(const QString& full_filename);

    //! This method searches the previously loaded DomDoc for entries older than the given
    //! timestamp and removes those entries from the DomDoc. Removes true upon success,
    //! false upon failure.
    bool removeEntriesOlderThan(const QDateTime& timestamp);

    //! This method adds an entry with start and end timestamp attributes of a new tag "entryname".
    //! The text parameter is stored as attribute of the same tag "entryname" with the attribute
    //! name "attribute".
    bool addEntry(const QDateTime& start, const QDateTime& end, const QString& entryname,
                  const QString& attribute, const QString& text);

    //! This method writes the currently held DomDoc (maybe reduced by the removeEntries...
    //! function) to a file with the given (relative) filename. It is advised to use a full path
    //! here. This will overwrite an existing file, if available. Returns true upon success, false
    //! upon failure.
    bool writeDomDocToFile(const QString& filename);

    //! This method searches the full tree for tag "entryname" holding attributes "attrib", if the
    //! text "text" is contained. Returns a list of all entries found in the format
    //! /verbatim
    //!   starttime;endtime;text
    //! /endverbatim
    QStringList searchForText(const QString& text, const QString& entryname, const QString& attrib);

private:

    //! Dummy copy constructor
    AVDomManager(const AVDomManager&);

    //! Dummy assignment operator
    AVDomManager& operator=(const AVDomManager&);

    //! Helper function, checks for date_entry consistency
    bool checkForConsistency(const QDomNode& current_element);
    //! Helper function, checks for main_sub_entry consistency
    bool checkForSubEntryConsistency(const QDomNode& current_element);
    //! Helper function, checks for start & end timestamp consistency
    bool checkForStartEndConsistency(const QDomNode& current_element);
    //! Helper function, if an entry for a given date is already available, this entry is returned,
    //! otherwise a new date entry is returned.
    QDomElement getDateEntry(QDomDocument& doc, const QDate& date);

    //! holds the full filename of the loaded XML file
    QString m_full_filename;
    //! holds the config
    QSharedPointer<AVDomManagerConfig> m_cfg;
    //! holds the full QDomDocument
    QDomDocument m_dom_doc;
    //! whether the dom doc was loaded from file or not.
    bool m_dom_doc_loaded;
};

#endif

// End of file
