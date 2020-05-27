///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIBDEBRECATED - Avibit Library Deprecated classes
//                               for compatibility with ported code from QT3
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief   AVPtrHash class header.
 */

#if !defined AVPTRHASH_H_INCLUDED
#define AVPTRHASH_H_INCLUDED

// QT includes
#include <QHash>

// local includes
#include <avlog.h>
#include <avmacros.h>

/////////////////////////////////////////////////////////////////////////////
//! Provides auto delete functionality for QHash
/*!
 * It extends QHash which is inaccessible due to security reasons.
 */

template<class KEY, class VALUE> class AVPtrHash : private QHash<KEY,VALUE*>
{
public:
    explicit AVPtrHash(uint size=0) :
        m_auto_delete(false)
    {
        if (size > 0)
            QHash<KEY,VALUE*>::reserve(size);
    }
    virtual ~AVPtrHash()
    {
        clear();
    }

    void clear()
    {
        if(m_auto_delete)
            qDeleteAll(*this);
        QHash<KEY,VALUE*>::clear();
    }

    virtual void insert(const KEY& key, VALUE* value)
    {
        if (QHash<KEY,VALUE*>::contains(key))
            if (m_auto_delete)
                qDeleteAll(QHash<KEY,VALUE*>::values(key));
        QHash<KEY,VALUE*>::insert(key,value);
    }

    //! Removes entry from the dictionary, if auto delete is set, the entry is deleted
    virtual bool remove(const KEY &key)
    {
        if (QHash<KEY,VALUE*>::contains(key))
        {
            if (m_auto_delete)
                qDeleteAll(QHash<KEY,VALUE*>::values(key));

            QHash<KEY,VALUE*>::remove(key);

            return true;
        }
        return false;
    }

    void replace(const KEY &key, VALUE* value)
    {
        insert(key,value);
    }

    virtual bool operator==(const AVPtrHash<KEY,VALUE> &other) const
    {
        if(other.m_auto_delete != m_auto_delete)
            return false;
        return QHash<KEY,VALUE*>::operator==(other);
    }

    virtual bool operator!=(const AVPtrHash<KEY,VALUE> &other) const
    {
        return !(this->operator==(other));
    }

    VALUE* value(const KEY &key)
    {
        return QHash<KEY,VALUE*>::value(key,NULL);
    }

    const VALUE* value(const KEY &key) const
    {
        return QHash<KEY,VALUE*>::value(key,NULL);
    }

    VALUE* operator[](const KEY &key) { return this->value(key); }
    const VALUE* operator[](const KEY &key) const { return this->value(key); }

    typedef typename QHash<KEY,VALUE*>::iterator iterator;
    typedef typename QHash<KEY,VALUE*>::const_iterator const_iterator;

    iterator begin() { return QHash<KEY,VALUE*>::begin(); }
    iterator end() { return QHash<KEY,VALUE*>::end(); }
    const_iterator begin() const { return QHash<KEY,VALUE*>::begin(); }
    const_iterator end() const { return QHash<KEY,VALUE*>::end(); }

    iterator find(const KEY &key ) { return QHash<KEY,VALUE*>::find(key); }
    const_iterator find(const KEY &key ) const { return QHash<KEY,VALUE*>::find(key); }

    //! Removes entry from the dictionary, if auto delete is set, the entry is deleted
    virtual iterator erase(iterator iter)
    {
        AVASSERT(iter!=this->end());
        if (m_auto_delete)
        {
            delete *iter;
            *iter = NULL;
        }
        return QHash<KEY,VALUE*>::erase(iter);
    }


    uint size() const { return QHash<KEY,VALUE*>::size(); }
    uint count() const { return this->size(); }

    bool contains(const KEY& key) const { return QHash<KEY,VALUE*>::contains(key); }

    bool autoDelete() const { return m_auto_delete; }
    void setAutoDelete ( bool auto_delete ) { m_auto_delete=auto_delete; }

    bool isEmpty() const { return QHash<KEY,VALUE*>::isEmpty(); }

    void reserve(uint size) { QHash<KEY,VALUE*>::reserve(size); }

    VALUE* take(const KEY &key) { return QHash<KEY,VALUE*>::take(key); }

private:
    bool m_auto_delete;
};

#endif

// End of file
