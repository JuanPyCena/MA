///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////


/*!
  \file
  \author  Alexander Wemmer, a.wemmer@avitbit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Holds a list of string pairs (O(1) access via QDict)
*/

#if !defined(AVSTRINGPAIRLIST_H_INCLUDED)
#define AVSTRINGPAIRLIST_H_INCLUDED

#include <QString>
#include <QRegExp>
#include <QFile>
#include <QTextStream>

#include "avlib_export.h"
#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////
//! Holds a list of pairs of strings. The pairs are stored in QDicts for
//! faster access. The list can be loaded and saved from/to a file.
class AVLIB_EXPORT AVStringPairList
{
public:

    //! Constructor
    /*!
     * \param filename The file holding the pairlist
     * \param separator The separator to be used for loading/saving to file
     * \param hash_size The QDict hash size to use
     */
    explicit AVStringPairList(const QString& filename = "",
                     const QString& separator = ":",
                     int hash_size = 17);

    //! Destructor
    virtual ~AVStringPairList();

    //! When enabled, only left-to-right queries are possible anymore. This
    //! allows duplictate entries on the right side. Disabled by default.
    void enableLeftToRightOnlyMode(bool enable) { m_left_to_right_only_mode = enable; }

    //! Adds a pair entry to the list. None of the given strings may contain
    //! the separator string. Existing enries will be overwritten. The left
    //! (right) part must be unique in respect to all other left (right) parts.
    /*!
     * \param left The left part of the pair
     * \param right The right part of the pair
     * \return True on success, false otherwise.
     */
    bool addPair(const QString& left, const QString& right);

    //! Adds pairs from the given stringlist. Each line of the stringlist is
    //! splitted by the given separator. The first/second field from the split
    //! is inserted as the left/right item of a pair. Return true on success,
    //! false otherwise.
    bool addPairsFromStringlist(const QStringList& stringlist, const QString& separator);

    //! return true if a left entry with the given string exists, false otherwise.
    bool existsLeftEntry(const QString& left) const;

    //! return true if a right entry with the given string exists, false otherwise.
    bool existsRightEntry(const QString& right) const;

    //! Returns the right part of the pair specified by the left part
    //! When no entry is found, an empty string is returned.
    QString getRightByLeft(const QString& left) const;

    //! Returns the left part of the pair specified by the right part
    //! When no entry is found, an empty string is returned.
    QString getLeftByRight(const QString& right) const;

    //! Searches for a left entry matching the given "value_to_match" with a
    //! regexp. If "unique_match" is true, only one left entry may match,
    //! otherwise the method returns with false, if "unique_match" is false,
    //! the first found matching entry will be returned.
    //! If successfull the matching left entry will be returned in
    //! "matching_left_entry", the corresponding right entry will be returned
    //! in "right_entry" and the method returns with true.
    //! If no matching left entry could be found, the method returns with false.
    //! ATTENTION: If only_unique_match is false, the results are not deteministic !!
    bool findMatchingLeftEntryByRegExp(const QString& value_to_match,
                                       bool only_unique_match,
                                       QString& matching_left_entry,
                                       QString& right_entry) const;

    //! Searches for a left entry matching the given "value_to_match" with a
    //! regexp.
    //! Returns only true if the corresponding right entry of such a match is
    //! equal to right_entry_to_match
    bool findMatchingLeftEntryByRegExpWithGivenRightEntry
                                      (const QString& value_to_match,
                                       const QString& right_entry_to_match) const;

    //! Removes a pair from the list, specified by the left part.
    /*!
     * \param left The left part of the pair to remove
     * \return true if the entry was found and removed, false otherwise
     */
    bool removePairByLeft(const QString& left);

    //! Removes a pair from the list, specified by the right part.
    /*!
     * \param right The right part of the pair to remove
     * \return true if the entry was found and removed, false otherwise
     */
    bool removePairByRight(const QString& right);

    //! Saves the pairs to the file
    /*!
     * \return true when successful, false on error
     */
    bool savePairs() const;

    //! Loads the pairs from the file. If any error occurs, the current
    //! list will not be replaced by the loaded list. If no error occurs,
    //! the current list will be deleted and replaced by the loaded list.
    /*!
     * \return true when successful, false on error
     */
    bool loadPairs();

    //! Returns a list of all pairs (format: left separator right)
    /*!
     * \param Returns the list of all pair entries separated
     *  by the specified separator string.
     */
    void getPairs(QStringList& entry_list) const;

    //! Deletes all translation table entries
    void clearPairs();

    //! Returns the number of entries of the translation table
    /*!
     * \return Returns the number of pairs
     */
    unsigned int getNrPairs() const;

    //! Returns a list of all left elements
    void getLefts(QStringList& left_list) const;

    //! Set a new filename for storing the pairs
    bool setFileName(const QString& filename);

    //! Get the filename of the pairs
    QString getFileName() { return m_filename; };

    //! Set a new separator string. Checks and returns false, if any
    //! of the pairs contains the new separator string. Returns true
    //! on success.
    bool setSeparator(const QString& separator);

    //! set the list's name for debugging
    void setListName(const QString& list_name)
    {
        m_list_name = list_name;
    }

    //! Joins the string pair list into a single string
    /*! Two separators are used:
     *  1. separator 1 which can be set with the constructor and the setSeparator() method
     *     separates the left from the right string; default: ":"
     *  2. separator 2 which can be specified as argument of the toString() method separates
     *     the string pairs from each other; default: "\n" (line feed)
     */
    QString toString(const QString & sep = "\n") const;

    //! Joins the string pair list into a text stream
    /*! Two separators are used:
     *  1. separator 1 which can be set with the constructor and the setSeparator() method
     *     separates the left from the right string; default: ":"
     *  2. separator 2 which can be specified as argument of the toString() method separates
     *     the string pairs from each other; default: "\n" (line feed)
     */
    void toTextStream(QTextStream &s, const QString & sep = "\n") const;

    //! Return the QHash
    const QHash<QString,QString> &getHash() const { return m_left_to_right_dict; }

protected:

    //! holds the separator used when storing the pairs to a file
    QString m_separator;

    //! name of the file holding the callsign data
    QString m_filename;

    //! holds the pairlist hashed by left entry of the pair
    QHash<QString, QString> m_left_to_right_dict;

    //! holds the pairlist hashed by right entry of the pair
    QHash<QString, QString> m_right_to_left_dict;

    //! when true, only left-to-right operations are permitted
    bool m_left_to_right_only_mode;

    //! may contain the list's name for debugging
    QString m_list_name;

private:

    //! Dummy copy constructor
    AVStringPairList(const AVStringPairList&);

    //! Dummy assignment operator
    AVStringPairList& operator=(const AVStringPairList&);
};

#endif

/////////////////////////////////////////////////////////////////////////////

// End of file
