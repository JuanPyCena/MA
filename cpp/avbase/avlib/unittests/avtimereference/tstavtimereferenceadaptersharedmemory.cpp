///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    UNITTESTS
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Function level test cases for AVTimeReferenceAdapterSharedMemory
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtimereference/avtimereferenceadaptersharedmemory.h"
#include "avsharedmemory.h"
#include "avshm_varnames.h"
#include "avdatastream.h"
#include "avtimereference.h"

#include "avtimereference/av_unittest_timereferenceholder.h"

class TstAVTimeReferenceAdapterSharedMemory : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVTimeReferenceAdapterSharedMemory();
    void test_initialize();
    void test_setSpeed();
    void test_setCurrentDateTime();
    void test_setCurrentDateTimeSpeed();
    void test_writeParameters();
    void test_readParameters();
    void test_slotCheckTimeReference();

    void test_polling();

private:
    void writeTestDatatoSHM(AVSharedMemory& shm, QDateTime reference_time, double speed, int offset_s, int offset_ms);
    bool checkParameters(QDateTime reference_time, double speed, qint64 offset, ulong global_resync_counter);
};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(long long)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTimeReferenceAdapterSharedMemory::initTestCase()
{
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTimeReferenceAdapterSharedMemory::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTimeReferenceAdapterSharedMemory::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTimeReferenceAdapterSharedMemory::cleanup()
{
    AVTimeReferenceAdapterSharedMemory instance;
    instance.resetProcess();
    AVUnitTestTimeReferenceHolder::resetAVTimeReferenceSystemTimeOverride();
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_AVTimeReferenceAdapterSharedMemory()
{
    AVLogInfo << "---- test_AVTimeReferenceAdapterSharedMemory launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterSharedMemory instance;
    QCOMPARE(instance.m_last_read_resync_counter, 1ul);
    QVERIFY(!instance.m_shared_memory.isNull());
    QVERIFY(instance.m_polling_timer.isActive());
    QCOMPARE(instance.m_polling_timer.interval(), static_cast<int>(AVTimeReferenceAdapterSharedMemory::DEFAULT_POLLING_INTERVAL));

    AVLogInfo << "---- test_AVTimeReferenceAdapterSharedMemory ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_initialize()
{
    AVLogInfo << "---- test_initialize launched (" << QTest::currentDataTag() << ") ----";

    AVTimeReferenceAdapterSharedMemory instance;

    QVERIFY(instance.m_shared_memory->pointer(0) != 0);

    AVLogInfo << "---- test_initialize ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_setSpeed()
{
    AVLogInfo << "---- test_setSpeed launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    AVTimeReferenceAdapterSharedMemory instance;

    instance.setCurrentDateTime(system_time);
    instance.setSpeed(2.1);
    QDateTime reference_time = instance.getLocalTimeReferenceHolder().getReferenceDateTime();

    QVERIFY(checkParameters(reference_time, 2.1, 1, 3)); // offset calculation takes 1ms difference in m_reference_date_time into account

    AVLogInfo << "---- test_setSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_setCurrentDateTime()
{
    AVLogInfo << "---- test_setCurrentDateTime launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    AVTimeReferenceAdapterSharedMemory instance;

    instance.setCurrentDateTime(system_time.addSecs(-10));
    QDateTime reference_time = instance.getLocalTimeReferenceHolder().getReferenceDateTime();

    QVERIFY(checkParameters(reference_time, 1., -10000, 2));

    AVLogInfo << "---- test_setCurrentDateTime ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_setCurrentDateTimeSpeed()
{
    AVLogInfo << "---- test_setCurrentDateTimeSpeed launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    AVTimeReferenceAdapterSharedMemory instance;

    instance.setCurrentDateTimeSpeed(system_time.addSecs(-10), 2.7);
    QDateTime reference_time = instance.getLocalTimeReferenceHolder().getReferenceDateTime();

    QVERIFY(checkParameters(reference_time, 2.7, -9999, 2)); // 1 ms offset results from AVTimeReferenceHolder::updateReferenceDateTime(...)

    AVLogInfo << "---- test_setCurrentDateTimeSpeed ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_writeParameters()
{
    AVLogInfo << "---- test_writeParameters launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    AVTimeReferenceAdapterSharedMemory instance;

    instance.writeParameters(system_time, 3.1, 1134);

    QVERIFY(checkParameters(system_time, 3.1, 1134, 2));

    AVLogInfo << "---- test_writeParameters ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_readParameters()
{
    AVLogInfo << "---- test_readParameters launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVTimeReferenceAdapterSharedMemory instance;

    AVSharedMemory shm(AVEnvironment::getApplicationName()+AV_SHM_TIMEREF_POSTFIX, 1024);
    writeTestDatatoSHM(shm, system_time, 0.8, 3132, 924);

    AVDateTime reference_time;
    double speed;
    qint64 offset;

    QVERIFY(instance.readParameters(reference_time, speed, offset));

    QCOMPARE(QDateTime(reference_time), system_time);
    QCOMPARE(speed, 0.8);
    QCOMPARE(offset, 3132924);

    AVLogInfo << "---- test_readParameters ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_slotCheckTimeReference()
{
    AVLogInfo << "---- test_slotCheckTimeReference launched (" << QTest::currentDataTag() << ") ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);

    AVSharedMemory shm(AVEnvironment::getApplicationName()+AV_SHM_TIMEREF_POSTFIX, 1024);
    writeTestDatatoSHM(shm, system_time.addSecs(-10), 0.8, 3132, 924);

    AVTimeReferenceAdapterSharedMemory instance;

    instance.slotCheckTimeReference();

    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time.addMSecs(3132924-2000)), 0);

    AVLogInfo << "---- test_slotCheckTimeReference ended (" << QTest::currentDataTag() << ")    ----";
}


///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::test_polling()
{
    AVLogInfo << "---- test_polling launched ----";

    QDateTime system_time = AVDateTime::currentDateTimeUtc();

    AVTimeReferenceAdapterSharedMemory instance;
    instance.m_polling_timer.setInterval(20);

    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);
    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time), 0);

    system_time = system_time.addMSecs(30);
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);
    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time), 0);

    // use the signal spy to wait for polling of SHM
    QSignalSpy spy_jump(&AVTimeReference::singleton(), SIGNAL(signalTimeJumped(const QDateTime&, const QDateTime&)));
    AVSharedMemory shm(AVEnvironment::getApplicationName()+AV_SHM_TIMEREF_POSTFIX, 1024);
    writeTestDatatoSHM(shm, system_time, 1., -13, 450);
    QVERIFY(spy_jump.wait());

    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time.addMSecs(-12550)), 0);

    system_time = system_time.addMSecs(30);
    AVUnitTestTimeReferenceHolder::setAVTimeReferenceSystemTimeOverride(system_time);
    QCOMPARE(AVTimeReference::currentDateTime().msecsTo(system_time.addMSecs(-12550)), 0);

    AVLogInfo << "---- test_polling ended ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTimeReferenceAdapterSharedMemory::writeTestDatatoSHM(AVSharedMemory& shm,
                                                               QDateTime reference_time,
                                                               double speed,
                                                               int offset_s,
                                                               int offset_ms)
{
    shm.lock();
    char* data = shm.getSegment();

    int* time_offset_s_ptr = reinterpret_cast<int*>(data);
    data += sizeof(int);
    double* speed_ptr = reinterpret_cast<double*>(data);
    data += sizeof(double);
    ulong* global_resync_counter_ptr = reinterpret_cast<ulong*>(data);
    data += sizeof(ulong);
    int* time_offset_ms_ptr = reinterpret_cast<int*>(data);
    data += sizeof(int);
    QByteArray dt_data;
    AVDataStream dt_stream(&dt_data, QIODevice::WriteOnly);
    dt_stream << reference_time;
    memcpy(data, dt_data.data(), dt_data.size());

    *time_offset_s_ptr = offset_s;
    *speed_ptr =  speed;
    *global_resync_counter_ptr = 132ul;
    *time_offset_ms_ptr = offset_ms;
    shm.setInitialized();
    shm.unlock();
}

///////////////////////////////////////////////////////////////////////////////

bool TstAVTimeReferenceAdapterSharedMemory::checkParameters(QDateTime reference_time, double speed, qint64 offset, ulong global_resync_counter)
{
    AVSharedMemory shm(AVEnvironment::getApplicationName()+AV_SHM_TIMEREF_POSTFIX, 1024);

    const char* data = shm.getSegment();

    const int* time_offset_s_ptr = reinterpret_cast<const int*>(data);
    data += sizeof(int);
    const double* speed_ptr = reinterpret_cast<const double*>(data);
    data += sizeof(double);
    const ulong* global_resync_counter_ptr = reinterpret_cast<const ulong*>(data);
    data += sizeof(ulong);
    const int* time_offset_ms_ptr = reinterpret_cast<const int*>(data);
    data += sizeof(int);
    QByteArray dt_data(data, 8);
    AVDataStream dt_stream(dt_data);
    AVDateTime reference_time_read;
    dt_stream >> reference_time_read;

    if(reference_time_read.msecsTo(reference_time) != 0)
    {
        AVLogError << "TstAVTimeReferenceAdapterSharedMemory::checkParameters: reference time was "<<reference_time_read
                <<", should be "<<reference_time;
        return false;
    }
    if(speed != *speed_ptr)
    {
        AVLogError << "TstAVTimeReferenceAdapterSharedMemory::checkParameters: speed was "<<*speed_ptr<<", should be "<<speed;
        return false;
    }
    if(*time_offset_s_ptr * 1000 + *time_offset_ms_ptr - offset != 0)
    {
        AVLogError << "TstAVTimeReferenceAdapterSharedMemory::checkParameters: time_offset was "<<*time_offset_s_ptr * 1000 + *time_offset_ms_ptr
                <<", should be "<<offset;
        return false;
    }
    if(*time_offset_ms_ptr >=1000 || *time_offset_ms_ptr <= -1000)
    {
        AVLogError << "TstAVTimeReferenceAdapterSharedMemory::checkParameters: ms time offset wrong: "<<*time_offset_ms_ptr;
        return false;
    }
    if(*global_resync_counter_ptr != global_resync_counter)
    {
        AVLogError << "TstAVTimeReferenceAdapterSharedMemory::checkParameters: global_resync_counter was "<<*global_resync_counter_ptr
                <<", should be "<<global_resync_counter;
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTimeReferenceAdapterSharedMemory,"avlib/unittests/config")
#include "tstavtimereferenceadaptersharedmemory.moc"


// End of file
