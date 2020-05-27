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
    \brief   Running mean class
*/

#ifndef __AVRUNNINGMEAN_H__
#define __AVRUNNINGMEAN_H__

//////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <QTextStream>
using namespace std;

#include "avmacros.h"
#include "avlog.h"

///////////////////////////////////////////////////////////////////////////////
//! class to maintain a running mean

template <class T> class AVRunningMean
{
public:
    //! construct the running mean class with the specified length
    /*!
     * \param max_length The desired length of the running mean. This value has to be > 0
     * \param zero_value Used for initialization.
     */
    AVRunningMean(unsigned int max_length = 4, T zero_value = 0, bool enable_undo = false) :
        m_max_length(max_length), m_current_index(0), m_current_length(0), m_values(0),
        m_zero_value(zero_value), m_undo_value(zero_value), m_undo_enable(enable_undo),
        m_undoable(false)
    {
        // Important, as we do not allocate any mem in this case but access it!
        AVASSERT(m_max_length > 0);

        m_values = new T[max_length];
        AVASSERT(m_values != 0);
        //for (unsigned int index = 0; index < max_length; ++index) m_values[index] = m_zero_value;
    }

    //! copy-constructor
    AVRunningMean(const AVRunningMean<T>& rm)
     : AVRunningMean(1)
    {
        if (&rm == this) return;
        copyContents(rm);
    }

    //! assignment operator
    const AVRunningMean& operator= (const AVRunningMean<T>& rm)
    {
        if (&rm == this) return *this;
        copyContents(rm);
        return *this;
    }

    //! destroy the running mean class
    ~AVRunningMean()
    {
        delete[] m_values;
    }

    //! Resizes this running mean (clears all contents)
    void resetLength(unsigned int max_length)
    {
        AVASSERT(max_length>0);
        clear();
        delete[] m_values;
        m_max_length = max_length;
        m_values = new T[max_length];
    }

    //! resets the running mean
    void clear() {
        m_current_index = 0;
        m_current_length = 0;
        if (m_undo_enable) {
            m_undoable = false;
        }
    }

    //! returns the number of elements in the running mean array
    unsigned int count() const { return m_current_length; }

    //! returns the max. length of the running mean array
    unsigned int maxLen() const { return m_max_length; }

    //! add the specified value to the running mean list
    void add(const T& val)
    {
        if (m_undo_enable) {
            m_undo_value = m_values[m_current_index];
            m_undoable = true;
        }
        m_values[m_current_index] = val;
        m_current_index++;
        if (m_current_length <  m_max_length) m_current_length++;
        if (m_current_index  >= m_max_length) m_current_index = 0;
    }

    //! Undo last add, if undo is enabled and has not been performed yet.
    bool undo()
    {
        if (m_undo_enable && m_undoable)
        {
            int index = int(m_current_index) - 1;
            if (index < 0) { index = m_max_length-1; }
            m_current_index = index;
            m_values[m_current_index] = m_undo_value;
            if (m_current_length < m_max_length) m_current_length--;
            m_undoable = false;
            return true;
        }

        return false;
    }

    //! return the last value added to this running mean; returns 0 if no values were added yet.
    T lastElem() const
    {
        int index = m_current_index-1;
        if (index < 0) { index = m_max_length-1; }
        if (m_current_length >= 1) return m_values[index];
        else return m_zero_value;
    }

    //! calculate the current running arithmetic mean, returns 0 if no values were added yet.
    T mean() const
    {
        T sum = m_zero_value;
        for (unsigned int index = 0; index < m_current_length; index++)
            sum += m_values[index];
        if (m_current_length >= 1) return sum/m_current_length;
        else return m_zero_value;
    }

    //! calculate the current running arithmetic mean, returns 0 if no values were added yet.
    T mean(unsigned int len) const
    {
        if (m_current_length == 0)
            return m_zero_value;

        T sum = m_zero_value;
        if (len < m_current_length)
        {
            int index = m_current_index;
            for (unsigned int counter = 0; counter < len; counter++)
            {
                --index;
                if (index < 0)
                    index = m_max_length-1;
                sum += m_values[index];
            }
            return sum/T(len);
        }
        else
            return mean();
    }

    //! calculate the current running arithmetic mean, returns 0 if no values were added yet.
    T weightedMean() const
    {
        T sum = m_zero_value;
        double w = m_current_length;
        for (unsigned int index = 0; index < m_current_length; index++) {
            sum += m_values[index] * w--;
        }
        if (m_current_length >= 1)
            return (sum * T(2) / T((m_current_length*(m_current_length+1))));
        else
            return m_zero_value;
    }

    //! calculate the current running arithmetic mean of absolute values.
    /*! returns 0 if no values were added yet.
     */
    T meanAbs() const
    {
        T sum = m_zero_value;
        for (unsigned int index = 0; index < m_current_length; index++)
            if (m_values[index] > m_zero_value)
                sum += m_values[index];
            else
                sum -= m_values[index];
        if (m_current_length >= 1) return sum/m_current_length;
        else return m_zero_value;
    }

    //! calculate the current running arithmetic mean of absolute values.
    /*! returns 0 if no values were added yet.
     */
    T meanAbs(unsigned int len) const
    {
        if (m_current_length == 0)
            return m_zero_value;

        T sum = m_zero_value;
        if (len < m_current_length)
        {
            int index = m_current_index;
            for (unsigned int counter = 0; counter < len; counter++)
            {
                --index;
                if (index < 0)
                    index = m_max_length-1;
                if (m_values[index] > m_zero_value)
                    sum += m_values[index];
                else
                    sum -= m_values[index];
            }
            return sum/len;
        }
        else
            return meanAbs();
    }

    //! calculate the trend over \p len elements, returns 0 if no values were added yet.
    T trend(unsigned int len) const
    {
        if (m_current_length <= 2)
            return m_zero_value;

        T avg1 = m_zero_value;
        T avg2 = m_zero_value;

        if (len > m_current_length)
            len = m_current_length;

        unsigned int len_half = len/2;
        int index = m_current_index;
        for (unsigned int counter = 0; counter < len; counter++)
        {
            --index;
            if (index < 0)
                index = m_max_length-1;

            if (counter < len_half)
                avg2 += m_values[index];
            else
                avg1 += m_values[index];
        }
        avg1 /= T(len-len_half);
        avg2 /= T(len_half);
        double td = 2.0*(avg2-avg1)/T(len);

        return td;
    }

    //! Calculates the current standard deviation s(N-1).
    /*! Returns 0 if less than 2 values were added yet
     */
    T stdDev() const
    {
        if (m_current_length <= 1) return 0;

        T sum = m_zero_value;
        T current_mean = mean();
        T e;

        for (unsigned int index = 0; index < m_current_length; index++)
        {
            e  = m_values[index] - current_mean;
            sum += e*e;
        }

        return sqrt(sum / static_cast<T>(m_current_length-1));
    }

    //! Calculates the current standard deviation of the last len added elements s(N-1).
    /*! Returns 0 if less than 2 values were added yet
     */
    T stdDev(unsigned int len) const
    {
        if (m_current_length <= 1) return 0;

        T sum = m_zero_value;
        T current_mean = mean(len);
        T e;

        if (len < m_current_length)
        {
            int index = m_current_index;
            for (unsigned int counter = 0; counter < len; counter++)
            {
                --index;
                if (index < 0)
                    index = m_max_length-1;
                e  = m_values[index] - current_mean;
                sum += e*e;
            }
            return sqrt(sum/(len-1));
        }
        else
            return stdDev();
    }

    //! Calculates the current standard deviation s(N-1) of absolute values.
    /*! Returns 0 if less than 2 values were added yet
     */
    T stdDevAbs() const
    {
        if (m_current_length <= 1) return 0;

        T sum = m_zero_value;
        T current_mean = meanAbs();
        T e;

        for (unsigned int index = 0; index < m_current_length; index++)
        {
            if (m_values[index] > m_zero_value)
                e  =  m_values[index] - current_mean;
            else
                e  = -m_values[index] - current_mean;
            sum += e*e;
        }

        return sqrt(sum / (m_current_length-1));
    }

    //! determine the current max value
    T maxValue() const
    {
        T max = m_values[0];
        for (unsigned int index = 1; index < m_current_length; index++)
            if (m_values[index] > max) max = m_values[index];
        return max;
    }

    //! determine the current min value
    T minValue() const
    {
        T min = m_values[0];
        for (unsigned int index = 1; index < m_current_length; index++)
            if (m_values[index]<min) min = m_values[index];
        return min;
    }

    //! Returns a string representation of this AVRunningMean.
    QString toString () const
    {
        QString str;
        QTextStream ts( &str, QIODevice::WriteOnly );

        for (unsigned int index = 0; index < m_max_length; index++) {
            if (index==m_current_index) ts << ">";
            ts << m_values[index];
            if (index==m_current_length) ts << "|";
            ts << " ";
        }

        ts << "meanAbs=" << meanAbs();

        if (m_undoable && m_undo_enable)
            ts << " undoVal=" << m_undo_value;

        return str;
    }

protected:

    //! copy array
    void copyContents(const AVRunningMean<T>& other)
    {
        if (m_max_length != other.m_max_length)
        {
            delete[] m_values;
            m_values = 0;
            m_max_length = other.m_max_length;
            m_values = new T[m_max_length];
            AVASSERT(m_values != 0);
        }

        m_current_index  = other.m_current_index;
        m_current_length = other.m_current_length;
        m_undo_value     = other.m_undo_value;
        m_undo_enable    = other.m_undo_enable;
        m_undoable       = other.m_undoable;

        for (unsigned int index=0; index<other.m_current_length; index++)
            m_values[index] = other.m_values[index];
    }

protected:

    unsigned int m_max_length;
    unsigned int m_current_index;
    unsigned int m_current_length;
    T *m_values;
    T m_zero_value;
    T m_undo_value;
    bool m_undo_enable;
    bool m_undoable;
};

#endif /* __AVRUNNINGMEAN_H__ */

// End of file
