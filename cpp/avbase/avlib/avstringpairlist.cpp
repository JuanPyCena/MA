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


#include <QRegExp>
#include <QTextStream>

#include "avstringpairlist.h"


///////////////////////////////////////////////////////////////////////////////

AVStringPairList::AVStringPairList(const QString& filename,
                                   const QString& separator,
                                   int hash_size)
{
    AVASSERT(!separator.isEmpty());
    m_separator = separator;
    m_filename = filename;
    m_left_to_right_only_mode = false;

    m_left_to_right_dict.reserve(hash_size);
    m_right_to_left_dict.reserve(hash_size);
}

/////////////////////////////////////////////////////////////////////////////

AVStringPairList::~AVStringPairList()
{
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::addPair(const QString& new_left,
                               const QString& new_right)
{
    if (new_left.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPair: "
                        "Left entry is empty", qPrintable(m_list_name));
        return false;
    }

    if (new_right.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPair: "
                        "Right entry is empty", qPrintable(m_list_name));
        return false;
    }

    if (new_left.contains(m_separator))
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPair: "
                        "Left entry %s contains the separator string %s",
                        qPrintable(m_list_name), qPrintable(new_left), qPrintable(m_separator));
        return false;
    }

    if (new_right.contains(m_separator))
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPair: "
                        "Right entry %s contains the separator string %s",
                        qPrintable(m_list_name), qPrintable(new_right), qPrintable(m_separator));
        return false;
    }

    if (m_left_to_right_dict.contains(new_left))
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1, "AVStringPairList(%s):addPair: "
                            "found same right by new left - removing", qPrintable(m_list_name));

        m_right_to_left_dict.remove(m_left_to_right_dict[new_left]);
        m_left_to_right_dict.remove(new_left);
    }

    if (!m_left_to_right_only_mode)
    {
        if (m_right_to_left_dict.contains(new_right))
        {
            if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
                AVLogger->Write(LOG_DEBUG1, "AVStringPairList(%s):addPair: "
                                "found same left by new right - removing", qPrintable(m_list_name));

            m_left_to_right_dict.remove(m_right_to_left_dict[new_right]);
            m_right_to_left_dict.remove(new_right);
        }
    }

    m_left_to_right_dict.insert(new_left, new_right);

    if (!m_left_to_right_only_mode)
    {
        m_right_to_left_dict.insert(new_right, new_left);
        AVASSERT(m_right_to_left_dict.count() ==  m_left_to_right_dict.count());
    }

    return true;
};

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::addPairsFromStringlist(
    const QStringList& stringlist, const QString& separator)
{
    bool ret = true;

    QStringList::const_iterator iter = stringlist.begin();
    for(; iter != stringlist.end(); ++iter)
    {
        QString line = *iter;
        if (!line.contains(separator))
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPairsFromStringlist: "
                            "line (%s) did not contain separator (%s) - skipping",
                            qPrintable(m_list_name), qPrintable(line), qPrintable(separator));
            ret = false;
            continue;
        }

        QString left_part = line.section(separator, 0,0);
        QString right_part = line.section(separator, 1,1);

        AVLogger->Write(LOG_DEBUG, "AVStringPairList(%s):addPairsFromStringlist: "
                        "left/right: (%s)/(%s)",
                        qPrintable(m_list_name), qPrintable(left_part), qPrintable(right_part));

        if (left_part.isEmpty())
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPairsFromStringlist: "
                            "Left part of (%s) was empty - skipping",
                            qPrintable(m_list_name), qPrintable(line));
            ret = false;
            continue;
        }

        if (right_part.isEmpty())
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPairsFromStringlist: "
                            "Right part of (%s) was empty - skipping",
                            qPrintable(m_list_name), qPrintable(line));
            ret = false;
            continue;
        }

        if (!addPair(left_part, right_part))
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):addPairsFromStringlist: "
                            "Could not insert pairs of (%s) - skipping",
                            qPrintable(m_list_name), qPrintable(line));
            ret = false;
            continue;
        }
    }

    return ret;
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::existsLeftEntry(const QString& left) const
{
    return m_left_to_right_dict.contains(left);
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::existsRightEntry(const QString& right) const
{
    if (m_left_to_right_only_mode) return false;
    return m_right_to_left_dict.contains(right);
}

/////////////////////////////////////////////////////////////////////////////

QString AVStringPairList::getRightByLeft(const QString& left) const
{
    if (left.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):getRightByLeft: "
                        "Left entry is empty", qPrintable(m_list_name));
        return QString::null;
    }

    if (!m_left_to_right_dict.contains(left))
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1, "AVStringPairList(%s):getRightByLeft: "
                            "No right entry found for left entry %s",
                            qPrintable(m_list_name), qPrintable(left));
        return QString::null;
    }

    return m_left_to_right_dict[left];
}

/////////////////////////////////////////////////////////////////////////////

QString AVStringPairList::getLeftByRight(const QString& right) const
{
    if (m_left_to_right_only_mode) return QString::null;

    if (right.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):getLeftByRight: "
                        "Right entry is empty", qPrintable(m_list_name));
        return QString::null;
    }

    if (!m_right_to_left_dict.contains(right))
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1, "AVStringPairList(%s):getLeftByRight: "
                            "No left entry found for right entry %s",
                            qPrintable(m_list_name), qPrintable(right));
        return QString::null;
    }

    return m_right_to_left_dict[right];
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::removePairByLeft(const QString& left)
{
    if (left.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):removePairByLeft: "
                        "Left entry is empty", qPrintable(m_list_name));
        return false;
    }

    if (!m_left_to_right_dict.contains(left))
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1, "AVStringPairList(%s):removePairByLeft: "
                            "No right entry found for left entry %s",
                            qPrintable(m_list_name), qPrintable(left));
        return false;
    }

    m_right_to_left_dict.remove(m_left_to_right_dict[left]);
    m_left_to_right_dict.remove(left);

    AVASSERT(m_right_to_left_dict.count() ==
             m_left_to_right_dict.count());

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::removePairByRight(const QString& right)
{
    if (m_left_to_right_only_mode) return false;

    if (right.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):removePairByRight: "
                        "Right entry is empty", qPrintable(m_list_name));
        return false;
    }

    if (!m_right_to_left_dict.contains(right))
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1, "AVStringPairList(%s):removePairByRight: "
                            "No left entry found for right entry %s",
                            qPrintable(m_list_name), qPrintable(right));
        return false;
    }

    m_left_to_right_dict.remove(m_right_to_left_dict[right]);
    m_right_to_left_dict.remove(right);

    AVASSERT(m_right_to_left_dict.count() ==
             m_left_to_right_dict.count());

    return true;
}

/////////////////////////////////////////////////////////////////////////////

QString AVStringPairList::toString(const QString & sep) const
{
    QString str;
    QTextStream stream(&str, QIODevice::WriteOnly);
    toTextStream(stream, sep);
    return str;
}

/////////////////////////////////////////////////////////////////////////////

void AVStringPairList::toTextStream(QTextStream &s, const QString & sep) const
{
    for(QHash<QString,QString>::const_iterator iter=m_left_to_right_dict.begin();
            iter!=m_left_to_right_dict.end(); ++iter )
    {
        s << iter.key() << m_separator
          << qPrintable(iter.value()) << sep;
    }
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::savePairs() const
{
    QFile outfile(m_filename);

    if (! outfile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):savePairs:"
                        "Could not open the data file %s.",
                        qPrintable(m_list_name), qPrintable(m_filename));
        return false;
    }

    QTextStream outstream(&outfile);
    toTextStream(outstream, "\n");

    outfile.flush();
    outfile.close();

    AVLogger->Write(LOG_INFO,
                    "AVStringPairList(%s):savePairs: "
                    "Saved %d pairs to %s",
                    qPrintable(m_list_name), m_left_to_right_dict.count(), qPrintable(m_filename));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::loadPairs()
{
    QFile infile(m_filename);

    QHash<QString,QString> new_left_to_right_dict;
    QHash<QString,QString> new_right_to_left_dict ;

    if (! infile.open(QIODevice::ReadOnly))
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):loadPairs:"
                        "Could not open file %s.", qPrintable(m_list_name), qPrintable(m_filename));

        return false;
    }

    unsigned long linenr = 0;

    QTextStream instream(&infile);

    while(! instream.atEnd())
    {
        QString line = instream.readLine(10000);
        linenr++;

        if (line.isNull())
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):loadPairs: "
                            "Could not read from file %s, line %lu.",
                            qPrintable(m_list_name), qPrintable(m_filename), linenr);

            return false;
        }

        line = line.trimmed();
        if (line.length() < 1 || line.startsWith("#")) continue;

        QString left = line.section(m_separator, 0, 0);
        QString right = line.section(m_separator, 1, 1);

        if (left.isEmpty() || right.isEmpty())
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):loadPairs: "
                            "Could not parse line %lu: \"%s\"",
                            qPrintable(m_list_name), linenr, qPrintable(line));
            return false;
        }

        new_left_to_right_dict.insertMulti(left, right);
        new_right_to_left_dict.insertMulti(right, left);
    }

    if (!m_left_to_right_only_mode)
        AVASSERT(new_right_to_left_dict.count() == new_left_to_right_dict.count());

    infile.close();

    // delete the old lists and set the new ones
    m_left_to_right_dict = new_left_to_right_dict;
    m_right_to_left_dict = new_right_to_left_dict;

    AVLogger->Write(LOG_INFO, "AVStringPairList(%s):loadPairs: "
                    "Loaded %d pairs from %s",
                    qPrintable(m_list_name), m_left_to_right_dict.count(), qPrintable(m_filename));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void AVStringPairList::getLefts(QStringList& left_list) const
{
    left_list.clear();

    for(QHash<QString,QString>::const_iterator it=m_left_to_right_dict.begin();
            it!=m_left_to_right_dict.end(); ++it)
        left_list.append(it.key());
}

/////////////////////////////////////////////////////////////////////////////

void AVStringPairList::getPairs(QStringList& entry_list) const
{
    entry_list.clear();
    for(QHash<QString,QString>::const_iterator it=m_left_to_right_dict.begin();
            it!=m_left_to_right_dict.end(); ++it)
        entry_list.append(it.key()+m_separator+(it.value()));
}

/////////////////////////////////////////////////////////////////////////////

void AVStringPairList::clearPairs()
{
    m_left_to_right_dict.clear();
    m_right_to_left_dict.clear();
}

/////////////////////////////////////////////////////////////////////////////

unsigned int AVStringPairList::getNrPairs() const
{
    return m_left_to_right_dict.count();
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::setFileName(const QString& filename)
{
    if (filename.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):setFileName: "
                        "The given filename is empty", qPrintable(m_list_name));
        return false;
    }

    m_filename = filename;
    return true;
};

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::setSeparator(const QString& separator)
{
    if (separator.isEmpty())
    {
        AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):setSeparator: "
                        "The given separator is empty", qPrintable(m_list_name));
        return false;
    }

    for(QHash<QString,QString>::const_iterator it=m_left_to_right_dict.begin();
            it!=m_left_to_right_dict.end(); ++it)
    {
        const QString &left = it.key();
        const QString &right = it.value();

        if (left.contains(separator) || right.contains(separator))
        {
            AVLogger->Write(LOG_ERROR, "AVStringPairList(%s):setSeparator: "
                            "The entry %s%s%s contains the separator string "
                            "%s - new separator not set",
                            qPrintable(m_list_name),
                            qPrintable(left), qPrintable(m_separator),
                            qPrintable(right), qPrintable(separator));
            return false;
        }
    }

    m_separator = separator;
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::findMatchingLeftEntryByRegExp(const QString& value_to_match,
                                                     bool only_unique_match,
                                                     QString& matching_left_entry,
                                                     QString& right_entry) const
{
    matching_left_entry = right_entry = QString::null;
    bool found_entry = false;

    for(QHash<QString,QString>::const_iterator iter=m_left_to_right_dict.begin();
            iter!=m_left_to_right_dict.end(); ++iter)
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1, "AVStringPairList:findMatchingLeftEntryByRegExp: "
                            "Searching for (%s) in left entry (%s)",
                            qPrintable(value_to_match), qPrintable(iter.key()));

        QRegExp search_expr = QRegExp(iter.key());

        if (search_expr.exactMatch(value_to_match))
        {
            AVLogger->Write(LOG_DEBUG, "AVStringPairList:findMatchingLeftEntryByRegExp: "
                            "Left entry (%s) matches (%s)",
                            qPrintable(iter.key()), qPrintable(value_to_match));

            if (only_unique_match && found_entry)
            {
                AVLogger->Write(LOG_WARNING, "AVStringPairList:findMatchingLeftEntryByRegExp: "
                                "Found another matching entry (%s) for (%s) but "
                                "\"only_unique_match\" was set - aborting",
                                qPrintable(iter.key()), qPrintable(value_to_match));

                matching_left_entry = right_entry = QString::null;
                return false;
            }

            found_entry = true;
            matching_left_entry = iter.key();
            right_entry = iter.value();

            if (!only_unique_match) return true;
        }
    }

    if (found_entry) return true;

    AVLogger->Write(LOG_INFO, "AVStringPairList:findMatchingLeftEntryByRegExp: "
                    "No matching left entry found for (%s)",
                    qPrintable(value_to_match));
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool AVStringPairList::findMatchingLeftEntryByRegExpWithGivenRightEntry
                                                (const QString& value_to_match,
                                                 const QString& right_entry_to_match) const
{
    for(QHash<QString,QString>::const_iterator iter=m_left_to_right_dict.begin();
            iter!=m_left_to_right_dict.end(); ++iter)
    {
        if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
            AVLogger->Write(LOG_DEBUG1,
                            "AVStringPairList:findMatchingLeftEntryByRegExpWithGivenRightEntry: "
                            "Searching for (%s) in left entry (%s)",
                            qPrintable(value_to_match), qPrintable(iter.key()));

        QRegExp search_expr = QRegExp(iter.key());

        if (search_expr.exactMatch(value_to_match))
        {
            AVLogger->Write(LOG_DEBUG,
                            "AVStringPairList:findMatchingLeftEntryByRegExpWithGivenRightEntry: "
                            "Left entry (%s) matches (%s)",
                            qPrintable(iter.key()),
                            qPrintable(value_to_match));

            QString right_entry = getRightByLeft(iter.key());

            if (right_entry == right_entry_to_match)
            {
                AVLogger->Write(LOG_DEBUG,
                                "AVStringPairList:"
                                "findMatchingLeftEntryByRegExpWithGivenRightEntry: "
                                "Left entry (%s) matches (%s) and right entry is %s. Found match!",
                                qPrintable(iter.key()),
                                qPrintable(value_to_match),
                                qPrintable(right_entry_to_match));
                return true;
            }
            else
                AVLogger->Write(LOG_DEBUG,
                                "AVStringPairList:"
                                "findMatchingLeftEntryByRegExpWithGivenRightEntry: "
                                "Left entry (%s) matches (%s) BUT right entry is %s. No match! ",
                                qPrintable(iter.key()),
                                qPrintable(value_to_match),
                                qPrintable(right_entry));
        }
    }

    if (!AVLogger->isLevelSuppressed(AVLog::LOG__DEBUG1))
        AVLogger->Write(LOG_DEBUG1,
                        "AVStringPairList:findMatchingLeftEntryByRegExpWithGivenRightEntry: "
                        "No matching left entry found for (%s) with matching right entry %s",
                        qPrintable(value_to_match),
                        qPrintable(right_entry_to_match));
    return false;
}

// End Of File
