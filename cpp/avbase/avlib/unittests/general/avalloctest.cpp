///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  DI Christian Muschick c.muschick@avibit.com
  \brief   Tests whether failed memory allocations generate exception or return 0. Test whether
           core dump is generated with exceptions when allocation fails.
*/


#include <QProcess>
#include <QTest>
#include <avunittestmain.h>
#include <QVector>


#include "avconfig.h" // TODO QTP-10
#include "avlog.h"
#include "avmacros.h"
#include "avdaemon.h"


/////////////////////////////////////////////////////////////////////////////

class AVAllocUnitTest : public QObject
{
    Q_OBJECT




private slots:

    //----------------------------------------

    virtual void init()
    {
    }


    //----------------------------------------

// we don't do this check under MS VC earlier than VS2008

#define DO_TESTALLOC
#if defined(_MSC_VER)
#if _MSC_VER < 1500
#undef DO_TESTALLOC
#endif
#endif

// Does not work under 64bit. TODO: leave enabled for 32 bit builds? See SWE-4168.
#undef DO_TESTALLOC

    void testAlloc()
    {
#if  defined(DO_TESTALLOC)
        qDebug("Performing stress-test memory allocation");

        // check if new really returns 0 when we're out of memory
        static const unsigned MB_CHUNK_SIZE = 1024*1024;
        unsigned char * m         = 0;
        unsigned total_allocated  = 0;
        bool throws               = false;
        QVector<unsigned char*> v;
        v.reserve(5000); // avoid dynamic resizing in loop
        try
        {
            do
            {
                m = new unsigned char[MB_CHUNK_SIZE];
                // we do *NOT* use an ASSERT here because this test should prove that
                // the system built-in "new" operator returns 0 in case of a memory alloc
                // failure, rather then throwing an exception
                //QVERIFY(m != 0);
                total_allocated++;
                v.push_back(m);
            } while (m);
        } catch (std::bad_alloc&)
        {
            throws = true;
        }

        for (int i=0; i<v.size(); ++i)
        {
            delete [] v[i];
        }

        qDebug("Total mem allocated: %d MB. Failed allocation throws exception: %d",
               total_allocated, (unsigned)throws);

        // Currently an exception is thrown.
        QVERIFY(throws);
#endif
    }

    //----------------------------------------

    /**
     *  Start a process in a temporary directory which dumps core because of an uncaught "out of
     *  mem" exception. Check whether the core file is there.
     */
    void testCoredump()
    {
        // this test confuses subsequent tests working with files, because it messes up the
        // current working directory due to an unknown bug (maybe in QT). It is temporarily
        // turned off.
#if 0
        QDir d;

        // Handle already existing tmp dir
        if (d.cd(TMP_DIR)) {
            qDebug("Cleaning up stale tmp dir");
            const QFileInfoList *fil = d.entryInfoList();
            QVERIFY(fil->count() == 2 ||
                           fil->count() == 3);
            for (QFileInfoList::const_iterator it = fil->begin();
                 it != fil->end();
                 ++it)
            {
                if ((*it)->isDir()) continue;
                d.remove((*it)->fileName());
            }
            d.cdUp();
            QVERIFY(d.rmdir(TMP_DIR));
        }

        QVERIFY(d.mkdir(TMP_DIR));
        QVERIFY(d.cd(TMP_DIR));
        d.convertToAbs();
        QVERIFY(QDir::setCurrent(d.absPath()));

        // assumes COREDUMP_EXE is in path...
        QProcess proc(COREDUMP_EXE);
        QVERIFY(proc.start());

        // quick'n'dirty
        while (proc.isRunning());

        const QFileInfoList *fil = d.entryInfoList();
        QVERIFY(fil != 0);
        QVERIFY(fil->count() == 2 ||
                       fil->count() == 3);

        bool core_file_generated = false;
        for (QFileInfoList::const_iterator it = fil->begin();
             it != fil->end();
             ++it)
        {
            if ((*it)->isDir()) continue;
            core_file_generated = true;
            d.remove((*it)->fileName());
        }

//        QVERIFY(core_file_generated);
        if (core_file_generated) {
            qDebug("Core file was generated after uncaught exception");
        } else {
            qDebug("Core file was NOT generated after uncaught exception");
        }

        d.cdUp();
        d.rmdir(TMP_DIR);
#endif
    }

private:
    static const QString COREDUMP_EXE;
    static const QString TMP_DIR; ///< Directory to receive core dump file
};


const QString AVAllocUnitTest::COREDUMP_EXE = "avcoredump";
const QString AVAllocUnitTest::TMP_DIR      = "tmp-alloctest";

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVAllocUnitTest,"avlib/unittests/config")
#include "avalloctest.moc"
// End of file
