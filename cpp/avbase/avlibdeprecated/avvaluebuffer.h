///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2008
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Alex Wemmer, a.wemmer@avibit.com
  \author  QT5-PORT: Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Holds a fixed number of values, where the oldest value will be
  removed when a new value is inserted (fixed length FIFO).
*/

#if !defined(AVVALUEBUFFER_H_INCLUDED)
#define AVVALUEBUFFER_H_INCLUDED

#include <QList>

#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////

// Holds a fixed number of values, where the oldest value will be
// removed when a new value is inserted (fixed length FIFO).
template <class T> class AVValueBuffer
{
public:

    //! Constructor
    explicit AVValueBuffer(unsigned int max_element_count=0) :
    m_max_element_count(max_element_count)
    {}

    //! Copy Constructor
    AVValueBuffer(const AVValueBuffer& other_buffer)
    {
        m_max_element_count = other_buffer.m_max_element_count;
        m_list = other_buffer.m_list;
    }

    //! Destructor
    virtual ~AVValueBuffer() {}

    //! Assignment operator
    AVValueBuffer& operator=(const AVValueBuffer& other_buffer)
    {
        if (&other_buffer == this) return *this;
        m_max_element_count = other_buffer.m_max_element_count;
        m_list = other_buffer.m_list;
        return *this;
    }

    //! Prepends the given value to the buffer.
    /*! If the number of elements in the buffer exceeds the configured
        max. number, the last element will be removed from the buffer.
        Note: The first element of the buffer will always be the newest element.
     */
    inline void prepend(const T& value)
    {
        m_list.prepend(value);
        if (m_list.count() > static_cast<int>(m_max_element_count)) m_list.pop_back();
    }

    //! Returns the value with the given index from the buffer.
    //! \warning This method will make an ASSERT when called with an invalid index!
    inline const T& getConstValueByIndex(unsigned int index)
    {
        AVASSERT(index < m_list.count());
        return m_list[index];
    }

    //! Returns the value with the given index from the buffer.
    /*! \warning This method will make an ASSERT when called with an invalid index!
     */
    inline T& getValueByIndex(unsigned int index)
    {
        AVASSERT(index < m_list.count());
        return m_list[index];
    }

    //! Returns the value with the given index from the buffer.
    /*! \warning This method will make an ASSERT when called with an invalid index!
     */
    inline const T& operator[](unsigned int index) const
    {
        AVASSERT(index < m_list.count());
        return m_list[index];
    }

    //! Returns the value with the given index from the buffer.
    /*! \warning This method will make an ASSERT when called with an invalid index!
     */
    inline T& operator[](unsigned int index)
    {
        AVASSERT(index < m_list.count());
        return m_list[index];
    }

    //! Returns the last (i.e. oldest) value from the buffer.
    /*! \warning This method will make an ASSERT when the buffer is empty!
     */
    inline T& getLastValue()
    {
        AVASSERT(m_list.count() > 0);
        unsigned int index = m_list.count() - 1;
        return m_list[index];
    }

    //! Returns the first (i.e. newest) value from the buffer.
    /*! \warning This method will make an ASSERT when the buffer is empty!
     */
    inline T& getFirstValue()
    {
        AVASSERT(m_list.count() > 0);
        return m_list[0];
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
    QList<T> m_list;

private:

    //! Equality operator
    bool operator==(const AVValueBuffer& other) const;

    //! Inequality operator
    bool operator!=(const AVValueBuffer& other) const;
};

#endif

// End of file
