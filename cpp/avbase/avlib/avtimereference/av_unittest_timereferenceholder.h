///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
*/

#if !defined(AV_UNITTEST_TIMEREFERENCEHOLDER_H)
#define AV_UNITTEST_TIMEREFERENCEHOLDER_H

#include "avtimereference/avtimereferenceholder.h"
#include "avlib_export.h"

/**
 * @brief The AVUnitTestTimeReferenceHolder class is used in unit tests to avoid race conditions. See SWE-3959.
 */
class AVLIB_EXPORT AVUnitTestTimeReferenceHolder : public AVTimeReferenceHolder
{
    Q_OBJECT

public:
    AVUnitTestTimeReferenceHolder();
    explicit AVUnitTestTimeReferenceHolder(const AVTimeReferenceHolder& other);
    ~AVUnitTestTimeReferenceHolder() override;

    //! Sets the system time override which is used by this time reference holder.
    //! Most users will want to use setAVTimeReferenceSystemTimeOverride.
    void setSystemTimeOverride(const QDateTime& override);
    void resetSystemTimeOverride();

    /**
     * @brief setAVTimeReferenceSystemTimeOverride installs a new time reference holder with an overriden system time
     *          in the AVTimeReference::singleton().
     *
     * Use this functionality to "mock" the time reference and avoid race conditions in time-critical code
     * accessing the system time via the AVTimeReference.
     */
    static void setAVTimeReferenceSystemTimeOverride(const QDateTime& override);

    /**
     * @brief resetAVTimeReferenceSystemTimeOverride undoes the effects of setAVTimeReferenceSystemTimeOverride.
     */
    static void resetAVTimeReferenceSystemTimeOverride();

private:
    QDateTime getSystemTime() const override;

    AVDateTime m_system_time_override;
};

#endif

// End of file
