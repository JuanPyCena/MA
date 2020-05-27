///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVUNITTESTHELPERLIB - AVCOMMON Unit Test helper library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Gerhard Scheikl, g.scheikl@avibit.com
    \brief   AVUnittestHelpers header
 */

#ifndef AVUNITTESTHELPERS_INCLUDED
#define AVUNITTESTHELPERS_INCLUDED

// Qt includes
#include <QByteArray>

// avlib includes
#include "avmacros.h"

// local includes
#include "avunittesthelperlib_export.h"

class QTemporaryDir;


//--------------------------------------------------------------------------------------------------

#define AVLOG_ENTER_TESTCASE \
    AVLOG_INDENT.setSectionName("-------- " + AV_FUNC_INFO + \
    (QTest::currentDataTag() ? QString(" - data tag \"") + QTest::currentDataTag() + "\"" : QString("")) + \
    " --------")

//--------------------------------------------------------------------------------------------------

#define AV_DOUBLE_COMPARE(actual, expected, maxdiff)                                           \
    if(!(qAbs((expected) - (actual)) <= maxdiff))                                              \
    {                                                                                          \
        QFAIL(qPrintable("AV_DOUBLE_COMPARE: expected "+QString::number(expected)              \
               +", was actually "+QString::number(actual)));                                   \
    }

//--------------------------------------------------------------------------------------------------
//! Helper methods for use with unit tests
//! using a static-only class because we are not allowed to use namespaces
class AVUNITTESTHELPERLIB_EXPORT AVUnittestHelpers
{
    //! friend declaration for function level test case
    friend class TstAVUnittestHelpers;

public:

    /*!
     * \brief setTemporaryHome makes the <an>_HOME environment variable point to a newly created,
     *                         autodeleted temporary directory.
     *
     * Also changes the current directory to the temporary folder.
     *
     * This is useful in order not to clutter the build directory with log files and other files
     * created during test runs.
     * Should be called exactly once, and before AVApplicationPreinit.
     *
     * \param application_name Used to construct the environment variable.
     * \return The temporary directory.
     */
    static QString setTemporaryHome(const QString& application_name);

    /*!
     * \brief Writes expected/actual data to temporary files and runs kdiff3
     * \param actual actual test output
     * \param expected expected test output
     * \param blocking choose if kdiff3 is ran blocking
     */
    static void kdiff3(const QByteArray &actual, const QByteArray &expected, bool blocking = true);
    static void kdiff3(const QString &actual, const QString &expected, bool blocking = true);

private:
    AVDISABLECOPY(AVUnittestHelpers);
    AVUnittestHelpers();

    static std::unique_ptr<QTemporaryDir> m_temporary_home_folder;
};

#endif // AVUNITTESTHELPERS_INCLUDED

// End of file
