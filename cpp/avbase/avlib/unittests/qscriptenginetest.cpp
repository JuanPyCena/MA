///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2011
//
// Module:    UnitTests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Bernhard Roth, b.roth@avibit.com
    \brief
*/



// QT includes
#include <QtScript/QScriptEngine>
#include <QTest>
#include <avunittestmain.h>


#if defined(Q_OS_LINUX)
#include <fenv.h>
#include <signal.h>
#include <setjmp.h>
#endif

// local includes


///////////////////////////////////////////////////////////////////////////////

class QScriptEngineTest : public QObject
{
    Q_OBJECT



    #if defined(Q_OS_LINUX)
    static volatile sig_atomic_t m_sigFpeRaised;
    static          sigjmp_buf   m_buf;
    #endif
private:
    static void handleSigFpe(int signo)
    {
        #if defined(Q_OS_LINUX)
        m_sigFpeRaised = 1;
        siglongjmp(m_buf, signo);
        #endif
    }


private slots:

    void init()
    {
    }

    void cleanup()
    {
    }

    void testQScriptEngineConstructor()
    {
        #if defined(Q_OS_LINUX)
        AVLogInfo << "----testQScriptEngineConstructor launched----";

        // setting up new sigaction
        struct sigaction oldSig, testSig;
        testSig.sa_handler = QScriptEngineTest::handleSigFpe;
        sigemptyset(&testSig.sa_mask);
        testSig.sa_flags = SA_RESTART;
        sigaction(SIGFPE, &testSig, &oldSig);

        // setting up test exception behaviour
        fexcept_t fpeFlags;
        fegetexceptflag(&fpeFlags, FE_INVALID);
        feenableexcept(FE_INVALID);

        // this should not raise an exception
        if (sigsetjmp(m_buf, 1) == 0) {
            QScriptEngine engine;
        }

        if (m_sigFpeRaised) {
            AVLogError << "WARNING: QScriptEngine Ctor still raises FP Invalid Exception (SIGFPE)";
        }

        // restoring original exception behaviour
        fesetexceptflag(&fpeFlags, FE_INVALID);
        // restoring original sigaction
        sigaction(SIGFPE, &oldSig, 0);

        // reverse check: unit test should fail if behaviour in Qt is fixed
        // so we can remove the exception suppression from AVThread::fpinit
        QVERIFY2(m_sigFpeRaised, "TEST OBSOLETE: QScriptEngine got fixed, remove "
                                "fedisableexcept(FE_INVALID); from AVThread::fpinit");

        AVLogInfo << "----testQDataStream ended----";
        #endif
    }
};


#if defined(Q_OS_LINUX)
volatile sig_atomic_t QScriptEngineTest::m_sigFpeRaised = 0;
         sigjmp_buf   QScriptEngineTest::m_buf;
#endif


AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(QScriptEngineTest,"avlib/unittests/config")
#include "qscriptenginetest.moc"
// End of file
