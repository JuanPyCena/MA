///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Gerhard Scheikl, g.scheikl@avibit.com
    \brief   AVTwoWayHash header
 */

#ifndef AVTWOWAYHASH_INCLUDED
#define AVTWOWAYHASH_INCLUDED

// Qt includes
#include <QHash>

// avlib includes
#include "avmacros.h"

// local includes

// forward declarations

///////////////////////////////////////////////////////////////////////////////
//! Allows fast lookups from value to key
template <class Key, class T>
class AVTwoWayHash : public QHash<Key, T>
{
    //! friend declaration for function level test case
    friend class TstAVTwoWayHash;


public:
    AVTwoWayHash() : QHash<Key, T>() { }

    AVTwoWayHash(std::initializer_list<std::pair<Key,T> > list);

    typedef typename QHash<Key, T>::iterator iterator;

    iterator erase(iterator it);
    iterator insert(const Key &key, const T &value);
    iterator insertMulti(const Key &key, const T &value);

    const Key key(const T &value) const;
    const Key key(const T &value, const Key &defaultKey) const;

    using QHash<Key, T>::keys;
    QList<Key> keys(const T &value) const;

    int remove(const Key &akey);
    T take(const Key &akey);

    AVTwoWayHash<Key, T> &unite(const AVTwoWayHash<Key, T> &other);

private:

    void removeFromValueHash(const Key &akey);

    Q_DISABLE_COPY(AVTwoWayHash)
    QHash<T, Key> m_value_hash;
};

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
AVTwoWayHash<Key, T>::AVTwoWayHash(std::initializer_list<std::pair<Key, T> > list)
    : QHash<Key, T>(list)
{
    for (typename std::initializer_list<std::pair<Key,T> >::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        m_value_hash.insert(it->second, it->first);
    }
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVTwoWayHash<Key, T>::iterator AVTwoWayHash<Key, T>::erase(
        typename AVTwoWayHash<Key, T>::iterator it)
{
    m_value_hash.remove(it.value());
    return typename QHash<Key, T>::erase(it);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVTwoWayHash<Key, T>::iterator AVTwoWayHash<Key, T>::insert(const Key &akey,
                                                                     const T &avalue)
{
    m_value_hash.insert(avalue, akey);
    return QHash<Key, T>::insert(akey, avalue);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
typename AVTwoWayHash<Key, T>::iterator AVTwoWayHash<Key, T>::insertMulti(const Key &akey,
                                                                          const T &avalue)
{
    m_value_hash.insertMulti(avalue, akey);
    return QHash<Key, T>::insertMulti(akey, avalue);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
const Key AVTwoWayHash<Key, T>::key(const T &value) const
{
    return m_value_hash.value(value);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
const Key AVTwoWayHash<Key, T>::key(const T &value, const Key &defaultKey) const
{
    return m_value_hash.value(value, defaultKey);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
QList<Key> AVTwoWayHash<Key, T>::keys(const T &value) const
{
    return m_value_hash.values(value);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
int AVTwoWayHash<Key, T>::remove(const Key &akey)
{
    removeFromValueHash(akey);
    return QHash<Key, T>::remove(akey);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
T AVTwoWayHash<Key, T>::take(const Key &akey)
{
    removeFromValueHash(akey);
    return QHash<Key, T>::take(akey);
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
AVTwoWayHash<Key, T> &AVTwoWayHash<Key, T>::unite(const AVTwoWayHash<Key, T> &other)
{
    m_value_hash.unite(other.m_value_hash);
    QHash<Key, T>::unite(other);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
void AVTwoWayHash<Key, T>::removeFromValueHash(const Key &akey)
{
    typedef QHash<Key, T> basetype;
    for(const T &tmpvalue : basetype::values(akey))
    {
        m_value_hash.remove(tmpvalue);
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif // AVTWOWAYHASH_INCLUDED

// End of file
