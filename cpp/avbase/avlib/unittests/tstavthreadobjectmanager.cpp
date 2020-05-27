//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVLIB - AviBit Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   Function level test cases for AVThreadObjectManager
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avthreadobjectmanager.h"


class TestObject : public QObject
{
    Q_OBJECT

public:
    TestObject(QString name)
    {
        setObjectName(name);
        AVLogInfo << "TestObject constructed: "<<objectName()<<" in thread "<<QThread::currentThreadId();
    }

    ~TestObject() override
    {
        AVLogInfo << "TestObject destructed: "<<objectName()<<" in thread "<<QThread::currentThreadId();
    }

public slots:
    QThread* getThread()
    {
        AVLogInfo << "TestObject::getThread "<<objectName()<<" in thread "<<QThread::currentThreadId();
        return QThread::currentThread();
    }

public:
};


class TstAVThreadObjectManager : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVThreadObjectManager_data();
    void test_AVThreadObjectManager();
    void test_moveToAvailableThread();
    void test_deleteObjectLater();

};

//-----------------------------------------------------------------------------

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(QObject)
// Q_DECLARE_METATYPE(AVThreadObjectManager)

//-----------------------------------------------------------------------------

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVThreadObjectManager::initTestCase()
{
    AVLogInfo << "main thread "<<QThread::currentThreadId();
}

//-----------------------------------------------------------------------------

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVThreadObjectManager::cleanupTestCase()
{

}

//-----------------------------------------------------------------------------

//! This will be called before each test function is executed.
void TstAVThreadObjectManager::init()
{

}

//-----------------------------------------------------------------------------

//! This will be called after every test function.
void TstAVThreadObjectManager::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVThreadObjectManager::test_AVThreadObjectManager_data()
{
    QTest::addColumn<unsigned int>("maximal_number");

    QTest::newRow("row1") << 3u;
}

void TstAVThreadObjectManager::test_AVThreadObjectManager()
{
    AVLogInfo << "---- test_AVThreadObjectManager launched (" << QTest::currentDataTag() << ") ----";

    QFETCH(unsigned int, maximal_number);

    AVThreadObjectManager instance(maximal_number);
    QCOMPARE(instance.m_maximal_number_of_threads, maximal_number);

    AVLogInfo << "---- test_AVThreadObjectManager ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVThreadObjectManager::test_moveToAvailableThread()
{
    AVLogInfo << "---- test_moveToAvailableThread launched (" << QTest::currentDataTag() << ") ----";

    AVThreadObjectManager instance(3);

    TestObject* object1 = new TestObject("object1");
    instance.moveToAvailableThread(object1);
    QCOMPARE(instance.m_active_threads.size(), 1);
    QCOMPARE(instance.m_thread_for_object.size(), 1);

    QThread* object1_thread = 0;
    QMetaObject::invokeMethod(object1, "getThread", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QThread*, object1_thread));
    QVERIFY(object1_thread != QThread::currentThreadId());
    QVERIFY(instance.m_active_threads.contains(object1_thread));

    TestObject* object2 = new TestObject("object2");
    instance.moveToAvailableThread(object2);
    QCOMPARE(instance.m_active_threads.size(), 2);
    QCOMPARE(instance.m_thread_for_object.size(), 2);

    QThread* object2_thread = 0;
    QMetaObject::invokeMethod(object2, "getThread", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QThread*, object2_thread));
    QVERIFY(object2_thread != QThread::currentThreadId());
    QVERIFY(object2_thread != object1_thread);
    QVERIFY(instance.m_active_threads.contains(object2_thread));

    TestObject* object3 = new TestObject("object3");
    instance.moveToAvailableThread(object3);
    QCOMPARE(instance.m_active_threads.size(), 3);
    QCOMPARE(instance.m_thread_for_object.size(), 3);

    QThread* object3_thread = 0;
    QMetaObject::invokeMethod(object3, "getThread", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QThread*, object3_thread));
    QVERIFY(object3_thread != QThread::currentThreadId());
    QVERIFY(object3_thread != object1_thread);
    QVERIFY(object3_thread != object2_thread);
    QVERIFY(instance.m_active_threads.contains(object3_thread));

    TestObject* object4 = new TestObject("object4");
    instance.moveToAvailableThread(object4);
    QCOMPARE(instance.m_active_threads.size(), 3);
    QCOMPARE(instance.m_thread_for_object.size(), 4);

    QThread* object4_thread = 0;
    QMetaObject::invokeMethod(object4, "getThread", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QThread*, object4_thread));
    QVERIFY(object4_thread != QThread::currentThreadId());
    QVERIFY(object4_thread == object1_thread || object4_thread == object2_thread || object4_thread == object3_thread);
    QVERIFY(instance.m_active_threads.contains(object4_thread));

    AVLogInfo << "---- test_moveToAvailableThread ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVThreadObjectManager::test_deleteObjectLater()
{
    AVLogInfo << "---- test_deleteObjectLater launched (" << QTest::currentDataTag() << ") ----";

    AVThreadObjectManager instance(3);

    TestObject* object1 = new TestObject("object1");
    instance.moveToAvailableThread(object1);
    TestObject* object2 = new TestObject("object2");
    instance.moveToAvailableThread(object2);
    TestObject* object3 = new TestObject("object3");
    instance.moveToAvailableThread(object3);
    TestObject* object4 = new TestObject("object4");
    instance.moveToAvailableThread(object4);

    QCOMPARE(instance.m_active_threads.size(), 3);
    QCOMPARE(instance.m_thread_for_object.size(), 4);

    instance.deleteObjectLater(object4);

    QCOMPARE(instance.m_active_threads.size(), 3);
    QCOMPARE(instance.m_thread_for_object.size(), 3);

    instance.deleteObjectLater(object2);

    QCOMPARE(instance.m_active_threads.size(), 2);
    QCOMPARE(instance.m_thread_for_object.size(), 2);

    instance.deleteObjectLater(object1);

    QCOMPARE(instance.m_active_threads.size(), 1);
    QCOMPARE(instance.m_thread_for_object.size(), 1);

    instance.deleteObjectLater(object3);

    QCOMPARE(instance.m_active_threads.size(), 0);
    QCOMPARE(instance.m_thread_for_object.size(), 0);

    AVLogInfo << "---- test_deleteObjectLater ended (" << QTest::currentDataTag() << ")    ----";
}

///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------

AV_QTEST_MAIN_AVCONFIG2(TstAVThreadObjectManager)
#include "tstavthreadobjectmanager.moc"


// End of file
