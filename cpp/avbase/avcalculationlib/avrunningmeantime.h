///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dietmar Goesseringer, d.goesseringer@avibit.com
    \author  QT5-PORT: Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Running mean of time stamped data items
*/

#ifndef __AVRUNNINGMEANTIME_H__
#define __AVRUNNINGMEANTIME_H__

//////////////////////////////////////////////////////////////////////////////

#include <cmath>
using namespace std;

#include "qmap.h"
#include "qpair.h"

#include "avmisc.h"
#include "avlog.h"
#include "avdatetime.h"

///////////////////////////////////////////////////////////////////////////////
//! class to maintain a running mean

template <class T> class AVRunningMeanTime
{

// ------  CONSTRUCTORS, INITIALIZERS, OPERATORS, DESTRUCTOR

public:

    //! construct the running mean class with the specified length
    /*!
     * \param max_length The desired length of the running mean
     * \param zero_value Used for initialization.
     */
    AVRunningMeanTime(unsigned int max_length = 4, T zero_value = 0, bool enable_undo = false) :
        m_max_length(max_length),
        m_adds(0),
        m_zero_value(zero_value),
        m_undo_enable(enable_undo),
        m_undoable(false)
    {
        if (m_max_length <= 0)
            m_max_length = 1;
    }

    //! copy-constructor
    AVRunningMeanTime(const AVRunningMeanTime& rm)
    {
        if (&rm == this) return;
        copyContents(rm);
    }

    //! assignment operator
    const AVRunningMeanTime& operator= (const AVRunningMeanTime& rm)
    {
        if (&rm == this) return *this;
        copyContents(rm);
        return *this;
    }

    //! destroy the running mean class
    ~AVRunningMeanTime() {}

    //! resets the running mean
    void clear()
    {
        m_items.clear();
        m_adds     = 0;
        m_undoable = false;
    }

protected:

    //! copy array
    void copyContents(const AVRunningMeanTime& other)
    {
        m_max_length  = other.m_max_length;
        m_adds        = other.m_adds;
        m_items       = other.m_items;
        m_zero_value  = other.m_zero_value;
        m_undo_item   = other.m_undo_item;
        m_undo_enable = other.m_undo_enable;
        m_undoable    = other.m_undoable;
    }

// ------  DATA MEMBERS

protected:

    unsigned int        m_max_length;
    unsigned int        m_adds;
    QMap<AVDateTime,T>  m_items;
    T                   m_zero_value;
    QPair<AVDateTime,T> m_undo_item;
    bool                m_undo_enable;
    bool                m_undoable;

// ------  METHODS

public:

    //! returns the number of elements in the running mean array
    unsigned int count() const { return m_items.count(); }

    //! returns the max. length of the running mean array
    unsigned int maxLen() const { return m_max_length; }

    //! return the last value added to this running mean; returns 0 if no values were added yet.
    T lastElem() const
    {
        if (m_items.count() >= 1) {
            typename QMap<AVDateTime,T>::const_iterator it = m_items.constEnd();
            --it;
            return it.value();
        } else {
            return m_zero_value;
        }
    }

    //! return the last value added to this running mean; returns 0 if no values were added yet.
    QPair<AVDateTime, T> lastItem() const
    {
        if (m_items.count() >= 1) {
            typename QMap<AVDateTime,T>::const_iterator it = m_items.constEnd();
            --it;
            return qMakePair(it.key(), it.value());
        } else {
            return qMakePair(AVDateTime(), m_zero_value);
        }
    }

    //! add the specified value to the running mean list
    void add(const AVDateTime& ts, const T& val)
    {
        if (m_undo_enable)
        {
            m_undo_item = lastItem();
            m_undoable = true;
        }

        m_items.insert(ts, val);
        if (m_items.count() > static_cast<int>(m_max_length))
            m_items.erase(m_items.begin());
        ++m_adds;
    }

    //! Undo last add, if undo is enabled and has not been performed yet.
    bool undo()
    {
        if (m_undo_enable && m_undoable && (m_items.count() >= 1))
        {
            // Remove last added item
            typename QMap<AVDateTime,T>::iterator it = m_items.end();
            --it;
            m_items.erase(it);
            // Add undo item
            if (m_adds > m_max_length)
                m_items.insert(m_undo_item.first, m_undo_item.second);
            --m_adds;
            // Set undo state
            m_undoable = false;
            return true;
        }
        return false;
    }


    //! calculate the current running arithmetic mean, returns 0 if no values were added yet.
    T mean() const
    {
        T sum = m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it;
        for (it = m_items.constBegin(); it != m_items.constEnd(); ++it)
            sum += it.value();

        if (m_items.count() >= 1)
            return sum/T(m_items.count());
        else
            return m_zero_value;
    }

    //! calculate the current running arithmetic mean, returns 0 if no values were added yet.
    T mean(unsigned int len) const
    {
        if ((m_items.count() == 0) || (len <= 0))
            return m_zero_value;

        T sum = m_zero_value;

        if (len < m_items.count())
        {
            typename QMap<AVDateTime,T>::const_iterator it = m_items.constEnd();
            --it;
            for (unsigned int counter = 0; counter < len; counter++)
            {
                sum += it.value();
                --it;
            }
            return sum/T(len);
        }
        else
            return mean();
    }

    //! calculate the current running arithmetic mean of absolute values.
    /*! returns 0 if no values were added yet.
     */
    T meanAbs() const
    {
        T sum = m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it;
        for (it = m_items.constBegin(); it != m_items.constEnd(); ++it)
            if (it.value() > m_zero_value)
                sum += it.value();
            else
                sum -= it.value();

        if (m_items.count() >= 1)
            return sum/T(m_items.count());
        else
            return m_zero_value;
    }

    //! calculate the current running arithmetic mean of absolute values.
    /*! returns 0 if no values were added yet.
     */
    T meanAbs(unsigned int len) const
    {
        if ((m_items.count() == 0) || (len <= 0))
            return m_zero_value;

        T sum = m_zero_value;

        if (len < m_items.count())
        {
            typename QMap<AVDateTime,T>::const_iterator it = m_items.constEnd();
            --it;
            for (unsigned int counter = 0; counter < len; counter++)
            {
                if (it.value() > m_zero_value)
                    sum += it.value();
                else
                    sum -= it.value();
                --it;
            }
            return sum/T(len);
        }
        else
            return mean();
    }

    //! calculates the slope between most current and \p len last element
    T slope(unsigned int len) const
    {
        if ((static_cast<int>(len) > m_items.count()) || (len <= 0))
            return m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it_t2 = m_items.constEnd();
        --it_t2;
        typename QMap<AVDateTime,T>::const_iterator it_t1 = it_t2;
        for (unsigned int counter = 0; counter < len-1; counter++)
            --it_t1;

        // slope, or trend resp. = d / dt
        T      d     = it_t2.value() - it_t1.value();
        double dt    = double( it_t1.key().msecsTo(it_t2.key()) ) / 1000.0;
        T      slope = d/T(dt);

        return slope;
    }

    //! calculate the trend over \p len elements
    /*! returns 0 if
        * \p len > number of values added yet
        * \p not monotonically increasing/decreasing or contains extremes
     */
    T trendMonotone(unsigned int len, bool allow_zero_crossing = true) const
    {
        if ((len > m_items.count()) || (len <= 0))
            return m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it_t2 = m_items.constEnd();
        --it_t2;
        typename QMap<AVDateTime,T>::const_iterator it_t1 = it_t2;
        for (unsigned int counter = 0; counter < len-1; counter++)
            --it_t1;

        // slope, or trend resp. = d / dt
        T      d     = it_t2.value() - it_t1.value();
        double dt    = double( it_t1.key().msecsTo(it_t2.key()) ) / 1000.0;
        T      slope = d/T(dt);

        // eventually check zero crossing
        if (!allow_zero_crossing)
            if ( AVsign(it_t2.value()) != AVsign(it_t1.value()) )
                return m_zero_value;

        // check that there are no extremes
        it_t2 = it_t1; ++it_t2;
        for (unsigned int counter = 0; counter < len-1; counter++)
        {
            if ( AVsign(it_t2.value()-it_t1.value()) != AVsign(d) )
            {
                return m_zero_value;
            }
            ++it_t1; ++it_t2;
        }

        return slope;
    }

    //! calculate the trend over \p len elements
    /*! returns 0 if
        * \p len > number of values added yet
        * \p not quasi-monotonically increasing/decreasing (i.e. exceeding the tolerance limit
              given as proportion of the max. deviation between min & max)
     */
    T trendQuasiMonotone(
        unsigned int len,
        double tolerance_ratio = 0.2,
        bool allow_zero_crossing = true) const
    {
        if ((static_cast<int>(len) > m_items.count()) || (len <= 0))
            return m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it_t2 = m_items.constEnd();
        --it_t2;
        typename QMap<AVDateTime,T>::const_iterator it_t1 = it_t2;
        for (unsigned int counter = 0; counter < len-1; counter++)
            --it_t1;

        // slope, or trend resp. = d / dt
        T      d     = it_t2.value() - it_t1.value();
        double toler = d * tolerance_ratio;
        double dt    = double( it_t1.key().msecsTo(it_t2.key()) ) / 1000.0;
        T      slope = d/T(dt);

        // eventually check zero crossing
        if (!allow_zero_crossing)
            if ( AVsign(it_t2.value()) != AVsign(it_t1.value()) )
                return m_zero_value;

        // check that there are no extremes
        it_t2 = it_t1; ++it_t2;
        double diff = 0;
        for (unsigned int counter = 0; counter < len-1; counter++)
        {
            diff = it_t2.value()-it_t1.value();
            if ( AVsign(diff) != AVsign(d) )
            {
                if ( diff > toler )
                    return m_zero_value;
            }
            ++it_t1; ++it_t2;
        }

        return slope;
    }

    //! calculate the trend over \p len elements
    /*! returns 0 if
        * \p len > number of values added yet
        * \p any intermediate element's deviation from the linear trendline exceeds \p max_dev
     */
    T trendLinear(unsigned int len, T max_dev = -1.0) const
    {
        if ((len > m_items.count()) || (len <= 0))
            return m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it_t2 = m_items.constEnd();
        --it_t2;
        typename QMap<AVDateTime,T>::const_iterator it_t1 = it_t2;
        for (unsigned int counter = 0; counter < len-1; counter++)
            --it_t1;

        // slope, or trend resp. = d / dt
        T      d     = it_t2.value() - it_t1.value();
        double dt    = double( it_t1.key().msecsTo(it_t2.key()) ) / 1000.0;
        T      slope = d/T(dt);

        // check if measurements support trend (i.e. do not deviate more than stdDev)
        if (max_dev > 0.0)
        {
            it_t2 = it_t1; ++it_t2;
            double act_dt;
            T req_val, act_val;
            for (unsigned int counter = 0; counter < len-1; counter++)
            {
                act_dt   = double( it_t1.key().msecsTo(it_t2.key()) ) / 1000.0;
                req_val  = it_t1.value() + act_dt*slope;
                act_val  = it_t2.value();
                if (fabs(double(req_val)-double(act_val)) > max_dev)
                {
                    //AVLogger->Write(LOG_INFO,
                    //    "AVRunningMeanTime::trend\tabs(%.2f-%.2f)>%.2f => 0.0",
                    //    double(req_val), double(act_val), double(max_dev));
                    return m_zero_value;
                }
                ++it_t2;
            }
        }

        return slope;
    }

    //! Calculates the current standard deviation s(N-1).
    /*! Returns 0 if less than 2 values were added yet
     */
    T stdDev() const
    {
        if (m_items.count() <= 1)
            return m_zero_value;

        T sum = m_zero_value;
        T current_mean = mean();
        T e;

        typename QMap<AVDateTime,T>::const_iterator it;
        for (it = m_items.constBegin(); it != m_items.constEnd(); ++it)
        {
            e = it.value() - current_mean;
            sum += e*e;
        }

        return sqrt(sum / (m_items.count()-1));
    }

    //! Calculates the current standard deviation s(N-1) of absolute values.
    /*! Returns 0 if less than 2 values were added yet
     */
    T stdDevAbs() const
    {
        if (m_items.count() <= 1)
            return m_zero_value;

        T sum = m_zero_value;
        T current_mean = mean();
        T e;

        typename QMap<AVDateTime,T>::const_iterator it;
        for (it = m_items.constBegin(); it != m_items.constEnd(); ++it)
        {
            if (it.value() > m_zero_value)
                e = it.value() - current_mean;
            else
                e = -it.value() - current_mean;
            sum += e*e;
        }

        return sqrt(sum / (m_items.count()-1));
    }

    //! determine the current max value
    T maxValue() const
    {
        if (m_items.count() <= 0)
            return m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it = m_items.constBegin();
        T max = it.value();
        for (; it != m_items.constEnd(); ++it)
            if (it.value() > max)
                max = it.value();
        return max;
    }

    //! determine the current min value
    T minValue() const
    {
        if (m_items.count() <= 0)
            return m_zero_value;

        typename QMap<AVDateTime,T>::const_iterator it = m_items.constBegin();
        T min = it.value();
        for (; it != m_items.constEnd(); ++it)
            if (it.value() < min)
                min = it.value();
        return min;
    }

/*    //! Returns a string representation of this AVRunningMeanTime.
    QString toString () const
    {
        QString str, elem;
        QTextStream ts( &str, IO_WriteOnly );

        for (unsigned int index = 0; index < m_max_length; index++) {
            AVsprintf(elem, "%.2f", m_values[index]);
            if (index==m_current_index) ts << ">";
            ts << elem;
            if (index==m_current_length) ts << "|";
            ts << " ";
        }

        AVsprintf(elem, "meanAbs=%.2f", meanAbs());
        ts << elem;
        if (m_undoable && m_undo_enable) {
            AVsprintf(elem, " undoVal=%.2f", m_undo_value);
            ts << elem;
        }

        return str;
    }*/
};

#endif /* __AVRUNNINGMEANTIME_H__ */

// End of file
