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
  \author  DI Alexander Wemmer, a.wemmer@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for the AVShare class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>

#include <QString>


#include "avlog.h"
#include "avmisc.h"
#include "avshare.h"


///////////////////////////////////////////////////////////////////////////////

//! used to detect whether an object has been deleted of not
class AVShareUnitTestClass
{

public:

    explicit AVShareUnitTestClass(int* test)
    {
        QVERIFY(test);
        QVERIFY(*test != 0);
        m_test = test;
    }

    ~AVShareUnitTestClass()
    {
        *m_test = 0;
    }

    void setValue(int val) { *m_test = val; }
    int getValue() const { return *m_test; }

protected:

    int* m_test;
};


///////////////////////////////////////////////////////////////////////////////


class AVShareUnitTest : public QObject
{

    Q_OBJECT

protected:


private slots:

    void init()
    {
        AVLogger->setMinLevel(AVLog::LOG__INFO);
    }

    //----------------------------------------

    void cleanup()
    {
    }

    //----------------------------------------

    void testWithoutShare()
    {
        AVLogger->Write(LOG_INFO, "----- AVShareTest:testWithoutShare -----");
        AVLogger->setMinLevel(AVLog::LOG__FATAL);

        int* int_ptr = new (LOG_HERE) int(1);
        AVShare<int> share1(int_ptr, "share1");

        QVERIFY(share1.getRefCount() == 1);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 1);

        share1.getShare() = 2;

        QVERIFY(share1.getRefCount() == 1);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 2);

        //-----

        int testint = 2;
        AVShareUnitTestClass* test = new AVShareUnitTestClass(&testint);
        QVERIFY(test);
        QVERIFY(testint == 2);

        AVShare<AVShareUnitTestClass> *share2 = new AVShare<AVShareUnitTestClass>(test, "share2");
        QVERIFY(share2);

        QVERIFY(share2->getRefCount() == 1);
        QVERIFY(!share2->getLocking());
        QVERIFY(share2->getShare().getValue() == 2);

        share2->getShare().setValue(3);
        QVERIFY(share2->getShare().getValue() == 3);
        QVERIFY(testint == 3);

        delete share2;
        share2 = 0;
        QVERIFY(testint == 0);

        QSharedPointer<int> shPtr1;
        QVERIFY(shPtr1.isNull() == true);

        QSharedPointer<int> shPtr2(shPtr1);
        QVERIFY(shPtr2.isNull() == true);

        QSharedPointer<AVShareUnitTestClass> shPtr3;
        QVERIFY(shPtr3.isNull() == true);

        int ti2 = 5;
        {
            AVShareUnitTestClass* test_class = new (LOG_HERE) AVShareUnitTestClass(&ti2);
            QSharedPointer<AVShareUnitTestClass> shPtr4(test_class);
            QVERIFY(ti2 == 5);
        }
        QVERIFY(ti2 == 0);
    }

    //----------------------------------------

    void testSharedOnce()
    {
        AVLogger->Write(LOG_INFO, "----- AVShareTest:testSharedOnce -----");
        AVLogger->setMinLevel(AVLog::LOG__FATAL);

        int* int_ptr_1 = new (LOG_HERE) int(1);
        AVShare<int> share1(int_ptr_1, "share1");
        int* int_ptr_2 = new (LOG_HERE) int(2);
        AVShare<int> share2(int_ptr_2, "share2");

        QVERIFY(share1.getRefCount() == 1);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 1);

        QVERIFY(share2.getRefCount() == 1);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 2);

        share2 = share1;

        QVERIFY(share1.getRefCount() == 2);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 1);

        QVERIFY(share2.getRefCount() == 2);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        share1.setLocking(true);
        share1.lock();

        QVERIFY(share1.getRefCount() == 2);
        QVERIFY(share1.getLocking());
        QVERIFY(share1.getShare() == 1);

        QVERIFY(share2.getRefCount() == 2);
        QVERIFY(share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        share1.unlock();
        share2.setLocking(false);

        QVERIFY(share1.getRefCount() == 2);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 1);

        QVERIFY(share2.getRefCount() == 2);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        int* int_ptr_4 = new (LOG_HERE) int(4);
        share1 = AVShare<int>(int_ptr_4, "share1clear");

        QVERIFY(share1.getRefCount() == 1);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 4);

        QVERIFY(share2.getRefCount() == 1);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        share2 = share2;

        QVERIFY(share2.getRefCount() == 1);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        int ti2 = 4;
        {
            AVShareUnitTestClass* test_class = new (LOG_HERE) AVShareUnitTestClass(&ti2);
            QSharedPointer<AVShareUnitTestClass> shPtr5(test_class);
            {
                QSharedPointer<AVShareUnitTestClass> shPtr5Copy(shPtr5);
                QVERIFY(ti2 == 4);
            }
            QVERIFY(ti2 == 4);
        }
        QVERIFY(ti2 == 0);
    }

    //----------------------------------------

    void testSharedTwice()
    {
        AVLogger->Write(LOG_INFO, "----- AVShareTest:testSharedTwice -----");
        AVLogger->setMinLevel(AVLog::LOG__FATAL);

        int* int_ptr_1 = new (LOG_HERE) int(1);
        AVShare<int> share1(int_ptr_1, "share1");
        int* int_ptr_2 = new (LOG_HERE) int(2);
        AVShare<int> share2(int_ptr_2, "share2");
        int* int_ptr_3 = new (LOG_HERE) int(3);
        AVShare<int> share3(int_ptr_3, "share3");

        share2 = share1;

        QVERIFY(share1.getRefCount() == 2);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 1);

        QVERIFY(share2.getRefCount() == 2);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        QVERIFY(share3.getRefCount() == 1);
        QVERIFY(!share3.getLocking());
        QVERIFY(share3.getShare() == 3);

        share1 = share3;

        QVERIFY(share1.getRefCount() == 2);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 3);

        QVERIFY(share2.getRefCount() == 1);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        QVERIFY(share3.getRefCount() == 2);
        QVERIFY(!share3.getLocking());
        QVERIFY(share3.getShare() == 3);

        int* int_ptr_5 = new (LOG_HERE) int(5);
        share1 = AVShare<int>(int_ptr_5, "share1clear");

        QVERIFY(share1.getRefCount() == 1);
        QVERIFY(!share1.getLocking());
        QVERIFY(share1.getShare() == 5);

        QVERIFY(share2.getRefCount() == 1);
        QVERIFY(!share2.getLocking());
        QVERIFY(share2.getShare() == 1);

        QVERIFY(share3.getRefCount() == 1);
        QVERIFY(!share3.getLocking());
        QVERIFY(share3.getShare() == 3);

        int ti3 = 4;
        {
            AVShareUnitTestClass* test_class = new (LOG_HERE) AVShareUnitTestClass(&ti3);
            QSharedPointer<AVShareUnitTestClass> shPtr5(test_class);
            {
                QSharedPointer<AVShareUnitTestClass> shPtr5Copy(shPtr5);
                QSharedPointer<AVShareUnitTestClass> shPtr5CopyCopy(shPtr5Copy);
                QVERIFY(ti3 == 4);
            }
            QVERIFY(ti3 == 4);
        }
        QVERIFY(ti3 == 0);
    }


};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVShareUnitTest,"avlib/unittests/config")
#include "avsharetest.moc"

// End of file
