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
    \author  QT4-PORT: Thomas Schiffer, t.schiffer@avibit.com
    \brief   AVPtrList class header.
 */

#if !defined AVPTRLIST_H_INCLUDED
#define AVPTRLIST_H_INCLUDED

// QT includes
#include <QList>

// avcommon includes
#include "avlog.h"
#include "avmacros.h"

// local includes

/////////////////////////////////////////////////////////////////////////////
//! Reimplementation of Qt3 QPtrList for compatibility of old code
/*!
 * It extends QList which is inaccessible due to security reasons.
 *
 * \attention
 * The iterator handling of AVPtrList and QPtrList is different:
 * <em>"any non-const function call performed on the QList will render
 * all existing iterators undefined"</em> \see QList::iterator documentation
 */

template <class TYPE> class AVPtrList : private QList<TYPE*>
{
public:

    typedef TYPE * value_type;

    AVPtrList() :
        m_auto_delete(false)
    {

    }
    virtual ~AVPtrList()
    {
        clear();
    }

    void clear()
    {
        if(m_auto_delete)
            qDeleteAll(*this);
        QList<TYPE*>::clear();
    }

    virtual TYPE* takeFirst()
    {
        return QList<TYPE*>::takeFirst();
    }

    //! Removes entry from the list, if auto delete is set, the entry is deleted
    virtual int removeAll(TYPE *ptr)
    {
        if (contains(ptr))
            if(m_auto_delete)
                delete ptr;
        return QList<TYPE*>::removeAll(ptr);
    }

    virtual typename QList<TYPE*>::iterator erase(typename QList<TYPE*>::iterator iter)
    {
        AVASSERT(iter>=this->begin() && iter<this->end());
        if (m_auto_delete)
        {
            delete *iter;
            *iter=NULL;
        }
        return QList<TYPE*>::erase(iter);
    }

    virtual typename QList<TYPE*>::iterator erase(typename QList<TYPE*>::iterator it_begin,
                                                  typename QList<TYPE*>::iterator it_end)
    {
        AVASSERT(it_begin >= this->begin() && it_end <= this->end() && it_begin <= it_end);
        if (m_auto_delete)
        {
            qDeleteAll(it_begin,it_end);
        }
        return QList<TYPE*>::erase(it_begin,it_end);
    }

    virtual bool removeAt(uint index)
    {
        if(static_cast<int>(index)>=QList<TYPE*>::size())
            return false;
        if(m_auto_delete)
            delete QList<TYPE*>::value(index);
        QList<TYPE*>::removeAt(index);
        return true;
    }

    bool remove(const TYPE* entry)
    {
        const int idx = indexOf(entry);
        if (idx >= 0) {
            return removeAt(idx);
        }
        return false;
    }

    bool replace(uint index, const TYPE* t)
    {
        if(static_cast<int>(index)>=QList<TYPE*>::size())
            return false;
        if(m_auto_delete)
            delete QList<TYPE*>::at(index);
        QList<TYPE*>::replace(index, const_cast<TYPE*>(t));
        return true;
    }

    bool contains(const TYPE* entry) const
    {
        return QList<TYPE*>::contains(const_cast<TYPE*>(entry)); //TODO better solution?
    }

    virtual int indexOf(const TYPE* entry)
    {
        return QList<TYPE*>::indexOf(const_cast<TYPE*>(entry));//TODO better solution?
    }

    virtual  TYPE* at(uint index) const
    {
        if(index>=static_cast<uint>(QList<TYPE*>::size()))
            return NULL;

        return QList<TYPE*>::at(index);
    }

    TYPE* operator[](uint index) const { return this->at(index); }

    bool operator==(const AVPtrList<TYPE> &other) const
    {
        if(other.m_auto_delete != m_auto_delete)
            return false;
        return QList<TYPE*>::operator==(other);
    }

    bool operator!=(const AVPtrList<TYPE> &other) const
    {
        return !(this->operator==(other));
    }

    typedef typename QList<TYPE*>::iterator iterator;
    typedef typename QList<TYPE*>::const_iterator const_iterator;

    iterator begin() { return QList<TYPE*>::begin(); }
    iterator end() { return QList<TYPE*>::end(); }
    const_iterator begin() const { return QList<TYPE*>::begin(); }
    const_iterator end() const { return QList<TYPE*>::end(); }
    const_iterator constBegin() const { return QList<TYPE*>::constBegin(); }
    const_iterator constEnd() const { return QList<TYPE*>::constEnd(); }
    QListIterator<TYPE*> javaIterator() const { return QListIterator<TYPE*>(*this); }


    uint size() const { return QList<TYPE*>::size(); }
    uint count() const { return this->size(); }
    uint count(TYPE* var) const { return QList<TYPE*>::count(var); }

    void insert(uint index, TYPE* item)
    {
        AVASSERT(index <= size());
        QList<TYPE*>::insert(index,item);
    }
    void insert(iterator it, TYPE* item)
    {
        QList<TYPE*>::insert(it,item);
    }
    void append(TYPE* item) { QList<TYPE*>::append(item); }
    void prepend(TYPE* item) { QList<TYPE*>::prepend(item); }

    TYPE* first() { return QList<TYPE*>::first(); }
    const TYPE* first() const { return QList<TYPE*>::first(); }
    TYPE* last() { return QList<TYPE*>::last(); }
    const TYPE* last() const { return QList<TYPE*>::last(); }

    virtual bool isEmpty() const  { return QList<TYPE*>::isEmpty(); }

    bool autoDelete() const { return m_auto_delete; }
    void setAutoDelete ( bool auto_delete ) { m_auto_delete=auto_delete; }

    virtual void removeFirst()
    {
        if(m_auto_delete)
            delete QList<TYPE*>::first();
        QList<TYPE*>::removeFirst();
    }

    virtual void removeLast()
    {
        if(m_auto_delete)
            delete QList<TYPE*>::last();
        QList<TYPE*>::removeLast();
    }

    TYPE* takeAt(uint index) { return QList<TYPE*>::takeAt(index); }


    /*! Removes duplicates (multiple occurrences of identical objects) in the list.
     *  The item type's equality operator== is called when checking for identity.
     *
     *  Returns the number of removed items.
     *
     *  The removed items are deleted if auto-deletion is enabled.
     */
    int removeDuplicates() {
        bool autoDel = this->autoDelete();
        int removals = 0;
        for ( uint i = 0; i < this->count(); i++ ) {
            for ( uint j = this->count()-1; j > i; j-- ) {
                if ( i != j ) {
                    if ( *this->at(i) == *this->at(j) ) {
                        if ( this->at(i) == this->at(j) )
                            this->setAutoDelete(false);
                        else
                            this->setAutoDelete(autoDel);
                        this->removeAt(j);
                        removals++;
                    }
                }
            }
        }
        this->setAutoDelete(autoDel);
        return removals;
    }

private:
    bool m_auto_delete;
};

#endif

// End of file
