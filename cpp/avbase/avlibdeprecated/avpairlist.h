
///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Alexander Wemmer, a.wemmer@avitbit.com
  \author  QT5-PORT: Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Holds a list of string pairs (O(1) access via QDict)
*/

#if !defined(AVPAIRLIST_H_INCLUDED)
#define AVPAIRLIST_H_INCLUDED

#include <QString>
#include <QMap>

#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////
//! Holds a list of pairs of T. The pairs are stored in QDicts for
//! faster access.
template <class LEFT, class RIGHT> class AVPairList
{
public:

    //! Constructor
    AVPairList(const LEFT& left_empty_value,
               const RIGHT& right_empty_value,
               const QString& list_name = QString::null)
    {
        m_empty_left = left_empty_value;
        m_empty_right = right_empty_value;
        m_list_name = list_name;
    }

    //! Destructor
    virtual ~AVPairList() {};

    //! Adds a pair entry to the list. None of the given strings may contain
    //! the separator string. Existing enries will be overwritten. The left
    //! (right) part must be unique in respect to all other left (right) parts.
    /*!
     * \param left The left part of the pair
     * \param right The right part of the pair
     * \return True on success, false otherwise.
     */
    bool addPair(const LEFT& new_left, const RIGHT& new_right)
    {
        if (m_left_to_right_map.contains(new_left))
        {
            AVLogger->Write(LOG_DEBUG1, "AVPairList(%s):addPair: "
                            "found some right by new left - removing", m_list_name.toLocal8Bit().constData());

            m_right_to_left_map.remove(m_left_to_right_map[new_left]);
            m_left_to_right_map.remove(new_left);
        }

        if (m_right_to_left_map.contains(new_right))
        {
            AVLogger->Write(LOG_DEBUG1, "AVPairList(%s):addPair: "
                            "found some left by new right - removing", m_list_name.toLocal8Bit().constData());

            m_left_to_right_map.remove(m_right_to_left_map[new_right]);
            m_right_to_left_map.remove(new_right);
        }

        m_left_to_right_map.insert(new_left, new_right);
        m_right_to_left_map.insert(new_right, new_left);

        AVASSERT(m_right_to_left_map.count() ==  m_left_to_right_map.count());
        return true;
    };

    //! return true if a left entry with the given string exists, false otherwise.
    bool existsLeftEntry(const LEFT& left) { return m_left_to_right_map.contains(left); }

    //! return true if a right entry with the given string exists, false otherwise.
    bool existsRightEntry(const RIGHT& right) { return m_right_to_left_map.contains(right); }

    //! Returns the right part of the pair specified by the left part
    //! When no entry is found, an empty entry will be.
    const RIGHT& getRightByLeft(const LEFT& left)
    {
        if (!m_left_to_right_map.contains(left)) return m_empty_right;
        return m_left_to_right_map[left];
    }

    //! Returns the left part of the pair specified by the right part
    //! When no entry is found, an empty string is returned.
    const LEFT& getLeftByRight(const RIGHT& right)
    {
        if (!m_right_to_left_map.contains(right)) return m_empty_left;
        return m_right_to_left_map[right];
    }

    //! Removes a pair from the list, specified by the left part.
    /*!
     * \param left The left part of the pair to remove
     * \return true if the entry was found and removed, false otherwise
     */
    bool removePairByLeft(const LEFT& left)
    {
        if (!m_left_to_right_map.contains(left))
        {
            AVLogger->Write(LOG_DEBUG1, "AVPairList(%s):removePairByLeft: "
                            "No right entry found for left entry", m_list_name.toLocal8Bit().constData());
            return false;
        }

        m_right_to_left_map.remove(m_left_to_right_map[left]);
        m_left_to_right_map.remove(left);
        AVASSERT(m_right_to_left_map.count() == m_left_to_right_map.count());
        return true;
    }

    //! Removes a pair from the list, specified by the right part.
    /*!
     * \param right The right part of the pair to remove
     * \return true if the entry was found and removed, false otherwise
     */
    bool removePairByRight(const RIGHT& right)
    {
        if (!m_right_to_left_map.contains(right))
        {
            AVLogger->Write(LOG_DEBUG1, "AVPairList(%s):removePairByRight: "
                            "No left entry found for right entry", m_list_name.toLocal8Bit().constData());
            return false;
        }

        m_left_to_right_map.remove(m_right_to_left_map[right]);
        m_right_to_left_map.remove(right);
        AVASSERT(m_right_to_left_map.count() ==  m_left_to_right_map.count());
        return true;
    }

    //! Deletes all translation table entries
    void clearPairs()
    {
        m_left_to_right_map.clear();
        m_right_to_left_map.clear();
    }

    //! Returns the number of entries of the translation table
    /*!
     * \return Returns the number of pairs
     */
    unsigned int getNrPairs() const
    {
        AVASSERT(m_right_to_left_map.count() ==  m_left_to_right_map.count());
        return m_left_to_right_map.count();
    }

    //! returns a textual representation of the AVPairList
    QString toString() const
    {
        QString result;

        typename QMap<LEFT, RIGHT>::ConstIterator it = m_left_to_right_map.begin();
        for ( ; it != m_left_to_right_map.end(); ++it )
        {
            result += QString("%1: %2").arg(it.key()).arg(it.value()) + "\n";
        }

        return result;
    }

protected:

    //! holds the pairlist hashed by left entry of the pair
    QMap<LEFT, RIGHT> m_left_to_right_map;

    //! holds the pairlist hashed by right entry of the pair
    QMap<RIGHT, LEFT> m_right_to_left_map;

    //! may contain the list's name for debugging
    QString m_list_name;

    //! dummy entry
    LEFT m_empty_left;

    //! dummy entry
    RIGHT m_empty_right;

private:

    //! hidden standard constructor
    AVPairList();

    //! Dummy copy constructor
    AVPairList(const AVPairList&);

    //! Dummy assignment operator
    AVPairList& operator=(const AVPairList&);
};

#endif

/////////////////////////////////////////////////////////////////////////////

// End of file
