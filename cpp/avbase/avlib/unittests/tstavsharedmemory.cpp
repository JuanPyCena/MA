//-----------------------------------------------------------------------------
//
// Package:   TODO: PLEASE ADAPT PACKAGE NAME
// Copyright: AviBit data processing GmbH, 2001-2015
//
// Module:    TODO: PLEASE ADAPT MODULE NAME
//
//-----------------------------------------------------------------------------

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AVSharedMemory
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avsharedmemory.h"

class TstAVSharedMemory : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:

    void test();

    // SWE-2023
    void testStaleSegmentWithSmallerSize();
    void testStaleSegmentWithLargerSize();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVSharedMemory::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVSharedMemory::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVSharedMemory::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVSharedMemory::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVSharedMemory::test()
{
    AVLogInfo << "---- test launched ----";

    int level = AVLogger->minLevel();
    AVLogger->setMinLevel(0);

    AVLogInfo << "TstAVSharedMemory::test: run "<<AVEnvironment::getApplicationRun();

    {
        AVSharedMemory mem1("seg1", 100);

        AVSharedMemory mem2("seg1", 100);

        QCOMPARE(mem1.getSize(), 100);
        QCOMPARE(mem2.getSize(), 100);

        QVERIFY(!mem1.isInitialized());
        QVERIFY(!mem2.isInitialized());

        QVERIFY(mem1.lock());
        mem1.setInitialized();

        QByteArray data;
        QDataStream write_stream(&data, QIODevice::WriteOnly);
        QString msg("Hello World!");
        write_stream << msg;
        QDataStream raw_stream(data);
        raw_stream.readRawData(mem1.getSegment(), mem1.getSize());

        QVERIFY(mem1.unlock());

        QVERIFY(mem1.lock());
        QVERIFY(mem1.isInitialized());
        QVERIFY(mem1.unlock());

        QVERIFY(mem2.lock());
        QVERIFY(mem2.isInitialized());
        QVERIFY(mem2.unlock());

        QVERIFY(mem2.lock());

        QByteArray mem2_block = QByteArray::fromRawData(mem2.getSegment(), mem2.getSize());
        QDataStream read_stream(&mem2_block, QIODevice::ReadOnly);
        QString result;
        read_stream >> result;
        QCOMPARE(result, msg);

        QVERIFY(mem2.unlock());
    }

    AVLogger->setMinLevel(level);

    AVLogInfo << "---- test ended ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSharedMemory::testStaleSegmentWithSmallerSize()
{
    AVLOG_ENTER(TstAVSharedMemory::testStaleSegmentWithSmallerSize);

    {
        AVLOG_ENTER(create initial segment);
        AVSharedMemory original_mem("test", 1024);
        QVERIFY(original_mem.pointer(0) != 0);
        QVERIFY(original_mem.getSize() == 1024);

        {
            AVLOG_ENTER(attach with smaller size);
            AVSharedMemory smaller_mem("test", 512);
            // Regardless of whether the existing SHM is smaller or larger, this
            // indicates a configuration error
            QVERIFY(smaller_mem.pointer(0) == 0);

        }
        {
            AVLOG_ENTER(attach with larger size);
            AVSharedMemory larger_mem("test", 2048);
            QVERIFY(larger_mem.pointer(0) == 0);

        }
        {
            AVLOG_ENTER(attach with identical size);
            AVSharedMemory identical_mem("test", 1024);
            AVLogInfo << (void*)identical_mem.pointer(0) << " vs " << (void*)original_mem.pointer(0);
            QVERIFY(identical_mem.pointer(0) != 0);
            // This is just a very crude check - the pointers themselves are not identical because the segment is mapped to different locations in RAM...
            QVERIFY(*identical_mem.pointer(0) == *original_mem.pointer(0));
            QVERIFY(identical_mem.getSize() == 1024);
        }

        // Leave a stale segment for the next test
        original_mem.detachWithoutCleanup();
    }

    // this can only work in Unix, because there is no stale segment detection in windows
    #if !defined(_OS_WIN32_) && !defined(Q_OS_WIN32)
    {
        AVLOG_ENTER(attach to stale with larger size);
        AVSharedMemory mem("test", 2048);
        QVERIFY(mem.pointer(0) != 0);
        QVERIFY(mem.getSize() == 2048);
    }
    #endif
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSharedMemory::testStaleSegmentWithLargerSize()
{
    AVLOG_ENTER(TstAVSharedMemory::testStaleSegmentWithLargerSize);

    {
        AVLOG_ENTER(create initial segment);
        AVSharedMemory original_mem("test", 1024);
        QVERIFY(original_mem.pointer(0) != 0);
        original_mem.detachWithoutCleanup();
    }

    // this can only work in Unix, because there is no stale segment detection in windows
    #if !defined(_OS_WIN32_) && !defined(Q_OS_WIN32)
    {
        // make sure stale segment is recreated with correct size
        AVLOG_ENTER(create smaller segment);
        AVSharedMemory smaller_mem("test", 512);
        QVERIFY(smaller_mem.pointer(0) != 0);
        QVERIFY(smaller_mem.getSize() == 512);
    }
    #endif
}

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVSharedMemory,"avlib/unittests/config")
#include "tstavsharedmemory.moc"

// Example for extracting parameters of QSignalSpy:
//
// qRegisterMetaType<XXXType>("XXXType");
// QSignalSpy spy(&instance, SIGNAL(signalReceivedPacket(const XXXType &)));
// instance.doXXX();
// QCOMPARE(spy.count(), 1);
// QList<QVariant> arguments = spy.takeFirst();
// QVariant v_XXX = arguments.at(0);
// QCOMPARE(v_XXX.canConvert<XXXType>(), true);
// XXXType YYY = v_XXX.value<XXXType>();
// QCOMPARE(YYY.XXX, ZZZ);
//
// --- if metatype is known:
// QSignalSpy spy(&instance, SIGNAL(signalReceivedPacket(const XXXType &)));
// instance.doXXX();
// QCOMPARE(spy.count(), 1);
// QList<QVariant> arguments = spy.takeFirst();
// QVariant v_XXX = arguments.at(0);
// XXXType YYY = v_XXX.toXXX();
// QCOMPARE(YYY.XXX, ZZZ);

// End of file
