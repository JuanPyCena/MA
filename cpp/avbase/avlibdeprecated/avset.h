///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIBDEPRECATED - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Stefan Kunz, s.kunz@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Holds a set of arbitrary values
*/

#if !defined(AVSET_H_INCLUDED)
#define AVSET_H_INCLUDED

// QT includes
#include <QMap>
#include <QList>
#include <QTextStream>

#include "avfromtostring.h"
#include "avdeprecate.h"

/////////////////////////////////////////////////////////////////////////////

//! A const iterator for the AVSet class.
//! Also see SWE-4535 - sanitize AVSet implementation.
template <class T> class AVSetConstIterator : public QMap<T, bool>::const_iterator
{
public:
    AVSetConstIterator<T>() : QMap<T, bool>::const_iterator() { m_false = false; }
    explicit AVSetConstIterator(QMapNode<T, bool> *p) : QMap<T, bool>::const_iterator(p) { m_false = false; }
    explicit AVSetConstIterator(const typename QMap<T, bool>::const_iterator &it) :
        QMap<T, bool>::const_iterator(it) { m_false = false; }
    explicit AVSetConstIterator(const QMapIterator<T, bool> &it) : QMap<T, bool>::const_iterator(it)
            { m_false = false; }

    bool operator== ( const typename QMap<T, bool>::const_iterator &it) const
    {
        return QMap<T, bool>::const_iterator::operator==(it);
    }

    bool operator!= ( const typename QMap<T, bool>::const_iterator &it) const
    {
        return QMap<T, bool>::const_iterator::operator!=(it);
    }

    //! Returns the value of this set entry.
    const T& value() const { return QMap<T, bool>::const_iterator::key(); }

    //! Returns the value of this set entry.
    /*! For compatability with standard iterator syntax.
        Basically the same as value()
    */
    const T& operator*() const { return value(); }

private:
    //! only the keys of the internal AVSet map shall be visible.
    bool m_false;
    const bool & data () const { return m_false; }
};

///////////////////////////////////////////////////////////////////////////////
/// This class is considered DEPRECATED. std::set should be used instead.
//! The AVSet class is a template class that provides a set with a strict weak ordering
/*! The set is traversed in the order defined by operator<(T, T).
 *  AVSet's value data type \c T must be an assignable data type, meaning that it has to provide:
 *      - A copy constructor
 *      - An assignment operator
 *      - A default constructor, i.e. a constructor that does not take any arguments.
 *
 *   In addition the type must implement the operator< to define ordering of the items.
 *
 *   \attention Be aware of the difference between AVSet<T> and QSet<T>. The former
 *              specifies a strict weak ordering for its item, whereas the latter stores its values
 *              in an unspecified order.
 *
 *  Also see SWE-4535 - sanitize AVSet implementation.
 *
 */
template <class T> class AVSet
{
public:
    typedef AVSetConstIterator<T> const_iterator;

    //! Constructor
    AVSet() {
        AVDEPRECATE(AVSet);
    }

    //! Copy Constructor
    AVSet(const AVSet& other)
    {
        m_set_dict = other.m_set_dict;
        AVDEPRECATE(AVSet);
    }

    //! Destructor
    virtual ~AVSet() {}

    //! Assignment operator
    AVSet& operator=(const AVSet& other)
    {
        if (&other == this) return *this;

        m_set_dict = other.m_set_dict;
        return *this;
    }

    //! Equality operator
    bool operator==(const AVSet& other) const
    {
        if (&other == this) return true;
        if (count() != other.count()) return false;

        typename QMap<T, bool>::const_iterator it    = m_set_dict.begin();
        typename QMap<T, bool>::const_iterator endIt = m_set_dict.end();
        for (; it != endIt; ++it)
        {
            if (!other.m_set_dict.contains(it.key())) return false;
        }

        return true;
    }

    //! Inequality operator
    bool operator!=(const AVSet& other) const
    {
        return !operator==(other);
    }

    //! Adds the given value to the set, returns true if the value already
    //! existed, false otherwise.
    //!  This method should be used only for primitive types (e.g. int). Non-
    //! primitive types should use addByRef() instead.
    inline bool add(T value)
    {
        bool existed = m_set_dict.contains(value);
        m_set_dict.insert(value, true);
        return existed;
    }

    //! Adds the given value to the set.
    /*! This method should be used only for primitive types (e.g. int). Non-
        primitive types should use addByRef() instead.
     */
    inline void addUnchecked(T value) { m_set_dict.insert(value, true); }

    //! Adds the given value to the set, returns true if the value already
    //! existed, false otherwise.
    /*! This method should be used for Non-primitive types. Primitive types
        such as ints should use add() instead.
     */
    inline bool addByRef(const T& value)
    {
        bool existed = m_set_dict.contains(value);
        m_set_dict.insert(value, true);
        return existed;
    }

    //! Adds the given value to the set.
    /*! This method should be used for Non-primitive types. Primitive types
        such as ints should use add() instead.
     */
    inline void addByRefUnchecked(const T& value) { m_set_dict.insert(value, true); }

    //! Adds the entries from the given list (for example AVIntList) to the set.
    /*! If an entry already existed, the new entry will overwrite the old one.
        Returns true if an entry already existed, false otherwise.
    */
    bool addFromList(const QList<T>& list)
    {
        bool ret = false;
        typename QList<T>::const_iterator iter = list.begin();
        for(; iter != list.end(); ++iter)
        {
            if (add(*iter)) ret = true;
        }
        return ret;
    }

    //! returns true if the given value exists in the set, false otherwise.
    /*! This method should be used only for primitive types (e.g. int). Non-
        primitive types should use existsByRef() instead.
     */
    // TODO CM remove
    inline bool exists(T value) const
    {
        return m_set_dict.contains(value);
    }

    //! returns true if the given value exists in the set, false otherwise.
    /*! This method should be used only for primitive types (e.g. int). Non-
        primitive types should use existsByRef() instead.
     */
    inline bool contains(T value) const
    {
        return m_set_dict.contains(value);
    }

    //! returns true if all elements in other take part in this set
    inline bool containsSet(const AVSet& other) const
    {
        if(&other == this)
            return true;

        AVSetConstIterator<T> it    = other.begin();
        AVSetConstIterator<T> endIt = other.end();
        for (; it != endIt; ++it)
        {
            if(m_set_dict.contains(it.key()) == false)
                return false;
        }

        return true;
    }

    //! returns true if the given value exists in the set, false otherwise.
    /*! This method should be used for Non-primitive types. Primitive types
        such as ints should use exists() instead.
     */
    // TODO UF rename to containsByRef
    inline bool existsByRef(const T& value) const
    {
        return m_set_dict.contains(value);
    }

    //! deletes the given value from the set, returns true if the value
    //! existed, false otherwise.
    /*! This method should be used only for primitive types (e.g. int). Non-
        primitive types should use delByRef() instead.
     */
    // TODO CM remove
    inline bool del(T value)
    {
        bool existed = m_set_dict.contains(value);
        m_set_dict.remove(value);
        return existed;
    }

    //! deletes the given value from the set, returns true if the value
    //! existed, false otherwise.
    /*! This method should be used only for primitive types (e.g. int). Non-
        primitive types should use delByRef() instead.
     */
    inline bool remove(T value)
    {
        bool existed = m_set_dict.contains(value);
        m_set_dict.remove(value);
        return existed;
    }

    //! deletes the given value from the set, returns true if the value
    //! existed, false otherwise.
    /*! This method should be used for Non-primitive types. Primitive types
        such as ints should use del() instead.
     */
    inline bool delByRef(const T& value)
    {
        bool existed = m_set_dict.contains(value);
        m_set_dict.remove(value);
        return existed;
    }

    //! Returns the union of this set and \c other set.
    inline AVSet<T> unite(const AVSet<T>& other) const
    {
        AVSet<T> set = *this;
        const_iterator it  = other.begin();
        const_iterator it_end = other.end();
        for (; it != it_end; ++it) set.addByRef(*it);
        return set;
    }

    //! Returns the intersection of this set and \c other set.
    inline AVSet<T> intersect(const AVSet<T>& other) const
    {
        AVSet<T> set;
        const_iterator it  = other.begin();
        const_iterator it_end = other.end();
        for (; it != it_end; ++it)
        {
            if (existsByRef(*it)) set.addByRef(*it);
        }
        return set;
    }

    //! Return a set of all elements which are members of this set but not members of \c other set.
    // TODO UF typo, rename it to subtract
    inline AVSet<T> substract(const AVSet<T>& other) const
    {
        AVSet<T> ret;
        for (typename QMap<T, bool>::const_iterator it = m_set_dict.cbegin(); it != m_set_dict.cend(); ++it)
        {
            if (it.value() && !other.existsByRef(it.key()))
            {
                ret.addByRefUnchecked(it.key());
            }
        }
        return ret;
    }

    //! returns the number of entries
    inline uint count() const
    {
        return m_set_dict.count();
    }

    //! returns wether the set is empty, i.e. count() == 0
    inline bool isEmpty() const
    {
        return m_set_dict.isEmpty();
    }

    //! clears the set
    inline void clear()
    {
        m_set_dict.clear();
    }

    //! returns a list with all values of the set, in order
    inline QList<T> getSet() const
    {
        return m_set_dict.keys();
    }

    //! Returns the content of the set as a comma separated list
    inline QString toString() const
    {
        return join();
    }

    //! Returns the content of the set as a separated list
    inline QString join(const QString& separator = ",") const
    {
        QString text;
        QTextStream ds(&text);

        typename QMap<T, bool>::const_iterator iter = m_set_dict.begin();
        for(; iter != m_set_dict.end();)
        {
            ds << iter.key();
            ++iter;
            if (iter != m_set_dict.end()) ds << separator;
        }

        return text;
    }

    //! const iterator begin
    AVSetConstIterator<T> begin() const { return static_cast<AVSetConstIterator<T> >(m_set_dict.begin()); }

    //! const iterator end
    AVSetConstIterator<T> end() const { return static_cast<AVSetConstIterator<T> >(m_set_dict.end()); }

    AVSet<T>& operator<<(const T& item)
    {
        add(item);
        return *this;
    }

protected:

    //! dict holding the values of the set as keys
    QMap<T, bool> m_set_dict;
};

/////////////////////////////////////////////////////////////////////////////

typedef AVSet<QString> AVStringSet;
typedef AVSetConstIterator<QString> AVStringSetConstIterator;
typedef AVSet<int> AVIntSet;
typedef AVSetConstIterator<int> AVIntSetConstIterator;

/////////////////////////////////////////////////////////////////////////////

template<typename T>
QString AVToString(const AVSet<T>& arg, bool enable_escape = false)
{
    QStringList entries;
    for (typename AVSet<T>::const_iterator it = arg.begin();
         it != arg.end();
         ++it)
    {
        entries << AVToString(*it, enable_escape);
    }
    return avfromtostring::DELIMITER_START +
           entries.join(QString(avfromtostring::SEPARATOR) + " ") +
           avfromtostring::DELIMITER_END;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVFromString(const QString& str, AVSet<T>& arg)
{
    arg.clear();
    QStringList entries;
    if (!avfromtostring::nestedSplit(entries, avfromtostring::SEPARATOR, str, true)) return false;
    for (int e=0; e<entries.size(); ++e)
    {
        QString value = entries[e].trimmed();

        T val;

        if (!AVFromString(value, val)) return false;

        if (arg.add(val)) return false;
    }

    return true;
}

#endif // AVSET_H_INCLUDED

// End of file
