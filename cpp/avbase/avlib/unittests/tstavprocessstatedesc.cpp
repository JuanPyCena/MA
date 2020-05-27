///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Alexander Randeu, a.randeu@avibit.com
    \author  QT5-PORT: Alexander Randeu, a.randeu@avibit.com
    \brief   Function level test cases for AVProcessStateDesc
 */

#include <QtTest>

#include <avunittestmain.h>
#include "avprocessstate.h"
#include "avprocessstatedesc.h"

class TstAVProcessStateDesc : public QObject
{
    Q_OBJECT;

public:
    TstAVProcessStateDesc()
        : m_proc_state(0),
          m_proc_state_desc(0) {}

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:

    void test_getAndSetParameter();
    void test_getAndSetParameter_data();
    void test_getAndSetParameterAtomic();
    void test_getAndSetParameterAtomic_data();

private:
    AVProcessState*     m_proc_state;
    AVProcessStateDesc *m_proc_state_desc;
};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(AVProcessStateDesc)
Q_DECLARE_METATYPE(AVCVariant::Type);

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVProcessStateDesc::initTestCase()
{
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVProcessStateDesc::cleanupTestCase()
{
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVProcessStateDesc::init()
{
    /* not possible to to this here, why?
       QVERIFY(AVProcState != 0); -> fails
       m_proc_state_desc = AVProcState->getProcessStateDesc();
       QVERIFY(m_proc_state_desc != 0);
    */

    const QString tmp_proc_name("tmp_proc_name");
    m_proc_state = AVProcessStateFactory::getInstance()->
                       getNewProcessState(tmp_proc_name, AVProcessState::defaultShmName, false);
    QVERIFY(m_proc_state != 0);

    m_proc_state_desc = m_proc_state->getProcessStateDesc();
    QVERIFY(m_proc_state_desc != 0);

    QVERIFY(m_proc_state_desc->isValid());
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVProcessStateDesc::cleanup()
{
    m_proc_state_desc = 0;
    delete m_proc_state;
}

///////////////////////////////////////////////////////////////////////////////

void TstAVProcessStateDesc::test_getAndSetParameter_data()
{
    const QString test_key("shm_test_key");

    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("set_val"); // empty is special case to to not execute set
    QTest::addColumn<bool>("set_val_return");
    QTest::addColumn<QString>("get_val"); // empty is special case to to not test get value
    QTest::addColumn<AVCVariant::Type>("get_val_type");
    QTest::addColumn<bool>("get_val_return");

    QTest::newRow("no SHM var available")
            << test_key << "" << true << "" << AVCVariant::Undef << false;
    QTest::newRow("set to 1 and verify")
            << test_key << "1" << true << "1" << AVCVariant::String << true;
    QTest::newRow("set to 2 and verify")
            << test_key << "2" << true << "2" << AVCVariant::String << true;
}

void TstAVProcessStateDesc::test_getAndSetParameter()
{
    AVLogInfo << "---- test_getAndSetParameter launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(QString, key);
    QFETCH(QString, set_val);
    QFETCH(bool, set_val_return);
    QFETCH(QString, get_val);
    QFETCH(AVCVariant::Type, get_val_type);
    QFETCH(bool, get_val_return);

    if(!set_val.isEmpty())
    {
        AVCVariant tmp_avc(&set_val);
        QVERIFY(m_proc_state_desc->setParameter(key, tmp_avc) == set_val_return);
    }

    AVCVariant tmp_avc;
    QCOMPARE(m_proc_state_desc->getParameter(key, tmp_avc), get_val_return);
    QCOMPARE(tmp_avc.type(), get_val_type);
    QCOMPARE(m_proc_state_desc->hasParameter(key), get_val_return);
    if(!set_val.isEmpty())
    {
        QCOMPARE(tmp_avc.asString(), get_val);
    }

    AVLogInfo << "---- test_getAndSetParameter ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVProcessStateDesc::test_getAndSetParameterAtomic_data()
{
    const QString test_key("shm_test_key");

    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("initial_set_val"); // empty is special case to to not set initial value
    QTest::addColumn<QString>("set_val");
    QTest::addColumn<QString>("old_val"); // empty is special case to to not test old value
    QTest::addColumn<AVCVariant::Type>("old_val_type");
    QTest::addColumn<bool>("val_return");

    QTest::newRow("no SHM var available and set to 1")
            << test_key << "" << "1" << "" << AVCVariant::Undef << false;
    QTest::newRow("Initially set to 1 and then set to 2")
            << test_key << "1" << "2" << "1" << AVCVariant::String << true;

}

void TstAVProcessStateDesc::test_getAndSetParameterAtomic()
{
    AVLogInfo << "---- test_getAndSetParameterAtomic launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(QString, key);
    QFETCH(QString, initial_set_val);
    QFETCH(QString, set_val);
    QFETCH(QString, old_val);
    QFETCH(AVCVariant::Type, old_val_type);
    QFETCH(bool, val_return);

    if(!initial_set_val.isEmpty())
    {
        AVCVariant initial_avc(&initial_set_val);
        QCOMPARE(m_proc_state_desc->setParameter(key, initial_avc), true);
        AVCVariant check_initial_avc;
        QCOMPARE(m_proc_state_desc->getParameter(key, check_initial_avc), true);
        QCOMPARE(check_initial_avc.type(), AVCVariant::String);
        QCOMPARE(check_initial_avc.asString(), initial_set_val);
    }

    AVCVariant set_avc(&set_val);
    AVCVariant get_avc;
    QCOMPARE(m_proc_state_desc->getAndSetParameterAtomic(key, get_avc, set_avc), val_return);
    QCOMPARE(get_avc.type(), old_val_type);

    if(!old_val.isEmpty())
    {
        QCOMPARE(get_avc.asString(), old_val);
    }

    // check new value
    AVCVariant get_new_avc;
    QCOMPARE(m_proc_state_desc->getParameter(key, get_new_avc), true);
    QCOMPARE(get_new_avc.type(), AVCVariant::String);
    QCOMPARE(get_new_avc.asString(), set_val);

    AVLogInfo << "---- test_getAndSetParameterAtomic ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVProcessStateDesc,"avlib/unittests/config")
#include "tstavprocessstatedesc.moc"

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
