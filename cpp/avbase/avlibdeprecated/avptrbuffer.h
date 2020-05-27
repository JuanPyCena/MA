///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT5-PORT: Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/


#ifndef __AVPTRBUFFER_H__
#define __AVPTRBUFFER_H__

#include "avptrlist.h"

#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////

// Holds a fixed number of ptrs, where the oldest ptr will be
// removed when a new ptr is inserted (fixed length FIFO).
//! The list will have autodelete set to true by default.
template <class T> class AVPtrBuffer
{
public:

    //! Constructor
    AVPtrBuffer(unsigned int max_element_count=0) : m_max_element_count(max_element_count)
    {
        m_list.setAutoDelete(true);
    }

    //! Destructor
    virtual ~AVPtrBuffer() {}

//     //! returns true if autodelete is set, which is the default
//     inline bool autoDelete() const { return m_list.autoDelete(); }
//     //! set the autodelete flag
//     inline void setAutoDelete(bool autodelete) { m_list.setAutoDelete(autodelete); }

    //! Prepends the given ptr to the buffer.
    //! If the number of elements in the buffer exceeds the configured
    //! max. number, the last element will be removed from the buffer.
    //! Note: The first element of the buffer will always be the newest element.
    //! ATTENTION: The ptr buffer takes the ownership of the given pointer!!
    inline void prepend(T* ptr)
    {
        m_list.prepend(ptr);
        if (m_list.count() > m_max_element_count) m_list.removeLast();
    }

    //! Returns the ptr with the given index from the buffer.
    //! \warning This method will make an ASSERT when called with an invalid index!
    inline const T* operator[](unsigned int index) const
    {
        AVASSERT(index < m_list.count());
        typename AVPtrList<T>::iterator iter(m_list);
        iter += index;
        return iter.current();
        //return m_list.at(index);
    }

    //! Returns the ptr with the given index from the buffer.
    //! \warning This method will make an ASSERT when called with an invalid index!
    inline T* operator[](unsigned int index)
    {
        AVASSERT(index < m_list.count());
        return m_list.at(index);
    }

    //! Returns a pointer to the first item in the buffer, or 0 if the buffer is empty.
    inline T* getFirst ()
    {
        if(m_list.size() == 0)
            return 0;
        return m_list.first();
    }

    //! Returns a pointer to the last item in the buffer, or 0 if the buffer is empty.
    inline T* getLast ()
    {
        if(m_list.size() == 0)
            return 0;
        return m_list.last();
    }

    //! returns the max. allowed number of elements in the buffer
    inline unsigned int getMaxElementCount() const { return m_max_element_count; }

    //! returns the number of entries
    inline uint count() const { return m_list.count(); }

    //! returns wether the set is empty, i.e. count() == 0
    inline bool isEmpty() const { return m_list.isEmpty(); }

    //! clears the set
    inline void clear() { m_list.clear(); }

protected:

    //! max. number of elements inside the buffer
    unsigned int m_max_element_count;

    //! list of elements inside the buffer. the first element (index = 0) will
    //! always be the newest element.
    AVPtrList<T> m_list;

private:

    //! Equality operator
    bool operator==(const AVPtrBuffer& other) const;

    //! Inequality operator
    bool operator!=(const AVPtrBuffer& other) const;

    //! Copy Constructor
    AVPtrBuffer(const AVPtrBuffer& other_buffer);
//     {
//         m_list.setAutoDelete(true);
//         m_max_element_count = other_buffer.m_max_element_count;
//         m_list = other_buffer.m_list;
//     }

    //! Assignment operator
    AVPtrBuffer& operator=(const AVPtrBuffer& other_buffer);
//     {
//         if (&other_buffer == this) return *this;
//         m_max_element_count = other_buffer.m_max_element_count;
//         m_list = other_buffer.m_list;
//         return *this;
//     }

};

#endif /* __AVPTRBUFFER_H__ */

// End of file
