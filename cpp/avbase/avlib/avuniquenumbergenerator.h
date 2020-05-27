///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVUNIQUENUMBERGENERATOR_H__
#define __AVUNIQUENUMBERGENERATOR_H__

#include <QString>
#include <QSet>

#include "avlib_export.h"
#include "avmisc.h"

/////////////////////////////////////////////////////////////////////////////

// *** DEPRECATED, don't use in new code ***
// Refer to QUuid instead.

//! unique number generator
//! ATTENTION: Make sure to call setMinMax() before use!
class AVLIB_EXPORT AVUniqueNumberGenerator
{
public:

    //! Standard Constructor
    AVUniqueNumberGenerator() : m_minimum(0), m_maximum(0), m_last_issued_number(0) {};

    //! Destructor
    virtual ~AVUniqueNumberGenerator() {};

    //! sets the minimum and maximum numbers to use
    //! ATTENTION: this will reset the number generator completely!
    inline void setMinMax(uint minimum, uint maximum)
    {
        AVLogger->Write(LOG_INFO, "AVUniqueNumberGenerator(%p):setMinMax: "
                        "min=%u, max=%u", this, minimum, maximum);

        m_issued_number_set.clear();
        m_minimum = m_last_issued_number = minimum;
        m_maximum = maximum;
    }

    //! returns the configured minimum track number
    inline uint getMin() const { return m_minimum; }

    //! returns the configured maximum track number
    inline uint getMax() const { return m_maximum; }

    //! Return the number of free numbers
    inline uint getNFreeNumbers()
    {
        return m_maximum - m_minimum +1 - m_issued_number_set.count();
    }

    //! Return the number of acquired/taken numbers
    inline uint getNAcquiredNumbers() { return m_issued_number_set.count(); }

    //! Returns a new free number as a string with the given width.
    //! Preceeding digits will be filled up with zeros.
    //! On overrun, "ok" will be set to false.
    //! ATTENTION: Make sure to call setMinMax() before the first call.
    inline QString acquireNumberAsFixedWidthString(uint width, bool& ok)
    {
        QString result;
        return AVsprintf(result, "%0*u", width, acquireNumber(ok));
    }

    //! Returns a new free number. On overrun, "ok" will be set to false.
    //! ATTENTION: Make sure to call setMinMax() before the first call.
    inline uint acquireNumber(bool& ok)
    {
        if (m_maximum == 0)
        {
            AVLogger->Write(LOG_ERROR, "AVUniqueNumberGenerator(%p):acquireNumber: "
                            "setMinMax() was not called", this);
            ok = false;
            return 0;
        }

        ok = true;
        uint search_number = m_last_issued_number;

        do
        {
            ++search_number;
            if (search_number > m_maximum) search_number = m_minimum;

            if (!m_issued_number_set.contains(search_number))
            {
                m_last_issued_number = search_number;
                m_issued_number_set.insert(m_last_issued_number);
                return m_last_issued_number;
            }
        }
        while(search_number != m_last_issued_number);

        AVLogger->Write(LOG_ERROR, "AVUniqueNumberGenerator(%p):acquireNumber: "
                        "overrun - no free number found", this);
        ok = false;
        return 0;
    }

    //! Removes the given number from the set of issued numbers.
    //! Returns true if the given number was issued.
    inline bool freeNumber(uint number) { return m_issued_number_set.remove(number); }

protected:

    uint m_minimum;
    uint m_maximum;
    uint m_last_issued_number;
    QSet<uint> m_issued_number_set;

private:

    //! Hidden copy-constructor
    AVUniqueNumberGenerator(const AVUniqueNumberGenerator&);
    //! Hidden assignment operator
    const AVUniqueNumberGenerator& operator = (const AVUniqueNumberGenerator&);
};

#endif /* __AVUNIQUENUMBERGENERATOR_H__ */

// End of file
