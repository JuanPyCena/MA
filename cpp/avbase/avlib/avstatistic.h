///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    AVLIB - Avibit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Gerhard Scheikl, g.scheikl@avibit.com
    \brief   Class to provide statistcs like mean, min, max, std. dev.
*/

#if !defined(AVSTATISTIC_H_INCLUDED)
#define AVSTATISTIC_H_INCLUDED

// required for math symbols (sqrt etc.)
#include <iostream>
using namespace std;
#include <cmath>

// AVDLib includes

// local includes
#include "avmisc.h"

///////////////////////////////////////////////////////////////////////////////
//! Class to provide statistcs like mean, min, max, std. dev.

template<class T>
class AVStatistic
{
public:

    //! constructor
    explicit AVStatistic(const QString &name = QString::null)
        : m_name(name)
    {
        clear();
    }

    //! destructor
    ~AVStatistic() {}

    //! clears the internal state
    void clear()
    {
        m_count          = 0;
        m_ulimit         = T(0);
        m_ulimit_set     = false;
        m_llimit         = T(0);
        m_llimit_set     = false;
        m_outside_limits = 0;
        m_min            = T(0);
        m_max            = T(0);
        m_mean           = 0.0;
        m_squared_sum    = 0.0;
    }

    //! set upper limit
    void setUpperLimit(T ul)
    {
        m_ulimit      = ul;
        m_ulimit_set  = true;
    }

    //! get upper limit
    T getUpperLimit() const { return m_ulimit; }

    //! check if an upper limit has been set
    bool getUpperLimitSet() const { return m_ulimit_set; }

    //! set lower limit
    void setLowerLimit(T ll)
    {
        m_llimit      = ll;
        m_llimit_set  = true;
    }

    //! get lower limit
    T getLowerLimit() const { return m_llimit; }

    //! check if a lower limit has been set
    bool getLowerLimitSet() const { return m_llimit_set; }

    //! returns the name of the statistic
    void setName(const QString &name) { m_name = name; }

    //! returns the name of the statistic
    QString getName() const { return m_name; }

    //! adds a new value to the statistic
    void add(T value, const QString& tag = QString::null)
    {
        if (m_count == 0)
        {
            m_count = 1;
            m_min = value;
            m_max = value;
            m_mean = value;
            m_squared_sum = value*value;
        }
        else
        {
            ++m_count;
            if (value < m_min) {
                m_min = value;
                m_min_tag = tag;
            } else if (value > m_max) {
                m_max = value;
                m_max_tag = tag;
            }

            m_mean        += (value - m_mean)/static_cast<double>(m_count);
            m_squared_sum += value*value;
        }
        if ((m_ulimit_set&&value>m_ulimit) || (m_llimit_set&&value<m_llimit))
            m_outside_limits++;
    }

    //! returns the number of elements added since the last clear
    int getCount() const { return m_count; }

    //! returns the percentage of values outside the limits
    double getOutsideLimitsPerc() const
    {
        if (m_count > 0)
            return 100.0*double(m_outside_limits)/double(m_count);
        else
            return 0.0;
    }

    //! returns the percentage of values inside limits (confidence interval)
    double getInsideLimitsPerc() const { return 100.0-getOutsideLimitsPerc();  }

    //! returns the actual minimum (T(0) if count == 0)
    T getMin() const { return m_min; }

    //! returns the actual maximum (T(0) if count == 0)
    T getMax() const { return m_max; }

    //! returns the mean (0.0 if count == 0)
    double getMean() const { return m_mean; }

    //! returns the standard deviation (+/-)
    double getStdDev() const
    {
        if (m_count > 0) {
            double val = m_squared_sum/static_cast<double>(m_count) - m_mean*m_mean;
            if ( val > 0.0 )
                return sqrt(val);
            else
                return 0.0;
        }
        else
            return 0.0;
    }

    //! Returns a string representation of this.
    QString toString(int precision = 2) const
    {
        QString str, str_name, str_stats, str_limit, str_mintag, str_maxtag;

        if ( !m_min_tag.isEmpty() )
            AVsprintf(str_mintag, "(%s)", m_min_tag.toLocal8Bit().constData());
        else
            str_mintag = "";
        if ( !m_max_tag.isEmpty() )
            AVsprintf(str_maxtag, "(%s)", m_max_tag.toLocal8Bit().constData());
        else
            str_maxtag = "";

        if ( !m_name.isEmpty() )
            AVsprintf(str_name, "%s[#%d]: ", m_name.toLocal8Bit().constData(), m_count);
        else
            AVsprintf(str_name, "[#%d]: ", m_count);

        AVsprintf(str_stats, "avg=%.*f,stddev=%.*f,min=%.*f%s,max=%.*f%s",
            precision, m_mean,
            precision, getStdDev(),
            precision, double(m_min), str_mintag.toLocal8Bit().constData(),
            precision, double(m_max), str_maxtag.toLocal8Bit().constData());

        if ( m_ulimit_set && !m_llimit_set )
            AVsprintf(str_limit, ",%.2f%%<%.1f", getInsideLimitsPerc(), double(m_ulimit));
        else if ( m_llimit_set && !m_ulimit_set )
            AVsprintf(str_limit, ",%.2f%%>%.1f", getInsideLimitsPerc(), double(m_llimit));
        else if ( m_llimit_set && m_ulimit_set )
            AVsprintf(str_limit, ",%.1f<%.2f%%<%.1f",
                double(m_llimit), getInsideLimitsPerc(), double(m_ulimit));

        str = str_name+str_stats+str_limit;

        return str;
    }

protected:

    QString     m_name;           //!< name
    T           m_ulimit;         //!< upper limit
    bool        m_ulimit_set;     //!< upper limit set
    T           m_llimit;         //!< lower limit
    bool        m_llimit_set;     //!< upper limit set

    int         m_count;          //!< count
    int         m_outside_limits; //!< number of values that exceeded the limit
    T           m_min;            //!< minimum
    T           m_max;            //!< maximum
    double      m_mean;           //!< mean
    double      m_squared_sum;    //!< sum of all values squared

    QString     m_max_tag;        //!< additional info where max occurred
    QString     m_min_tag;        //!< additional info where min occurred
};

#endif

// End of file
