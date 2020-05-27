///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Gerhard Scheikl, g.scheikl@avibit.com
    \brief   Function level test cases for AVTwoWayHash
 */



#include <QtTest>
#include <avunittestmain.h>
#include "avtwowayhash.h"

class TstAVTwoWayHash : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
//    void test_erase_data();
//    void test_erase();
    void test_insert_data();
    void test_insert();
    void test_insert_string_int_data();
    void test_insert_string_int();
    void test_insertMulti_data();
    void test_insertMulti();
    void test_key_data();
    void test_key();
    void test_key_2_data();
    void test_key_2();
    void test_keys_data();
    void test_keys();
    void test_remove_data();
    void test_remove();
    void test_take_data();
    void test_take();
    void test_unite();
    void test_removeFromValueHash();
};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
//typedef typename QHash<Key, T>::iterator HELPER_TYPE_1;
//Q_DECLARE_METATYPE(HELPER_TYPE_1);
//Q_DECLARE_METATYPE(Key);
//Q_DECLARE_METATYPE(T);
//Q_DECLARE_METATYPE(QList<Key>);
//typedef AVTwoWayHash<Key, T> HELPER_TYPE_2;
//Q_DECLARE_METATYPE(HELPER_TYPE_2);

Q_DECLARE_METATYPE(QList<QString>)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVTwoWayHash::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVTwoWayHash::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVTwoWayHash::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVTwoWayHash::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

//void TstAVTwoWayHash::test_erase_data()
//{
//#if 0
//    QTest::addColumn<HELPER_TYPE_1>("it");
//    QTest::addColumn<HELPER_TYPE_1>("ret_val");

//    QTest::newRow("row1") << HELPER_TYPE_1() << HELPER_TYPE_1();
//#endif
//}

//void TstAVTwoWayHash::test_erase()
//{
//    AVLogInfo << "---- test_erase launched ----";

//#if 0
//    QFETCH(HELPER_TYPE_1, it);
//    QFETCH(HELPER_TYPE_1, ret_val);

//    AVTwoWayHash instance;
//    QCOMPARE(instance.erase(it), ret_val);
//#endif
//    QFAIL("Test is not implemented.");

//    AVLogInfo << "---- test_erase ended    ----";
//}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_insert_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value");

    QTest::newRow("row1") << "the_key" << "the_value";
}

void TstAVTwoWayHash::test_insert()
{
    AVLogInfo << "---- test_insert launched ----";

    QFETCH(QString, key);
    QFETCH(QString, value);

    AVTwoWayHash<QString, QString> instance;
    instance.insert(key, value);
    QVERIFY(instance.contains(key));
    QCOMPARE(instance.value(key), value);

    QVERIFY(instance.m_value_hash.contains(value));
    QCOMPARE(instance.m_value_hash.value(value), key);

    AVLogInfo << "---- test_insert ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_insert_string_int_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<int>("value");

    QTest::newRow("row1") << "the_key" << 15;
}

void TstAVTwoWayHash::test_insert_string_int()
{
    AVLogInfo << "---- test_insert_string_int launched ----";

    QFETCH(QString, key);
    QFETCH(int, value);

    AVTwoWayHash<QString, int> instance;
    instance.insert(key, value);
    QVERIFY(instance.contains(key));
    QCOMPARE(instance.value(key), value);

    QVERIFY(instance.m_value_hash.contains(value));
    QCOMPARE(instance.m_value_hash.value(value), key);

    AVLogInfo << "---- test_insert_string_int ended    ----";
}


///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_insertMulti_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value1");
    QTest::addColumn<QString>("value2");

    QTest::newRow("row1") << "the_key" << "the_value1" << "the_value2";
}

void TstAVTwoWayHash::test_insertMulti()
{
    AVLogInfo << "---- test_insertMulti launched ----";

    QFETCH(QString, key);
    QFETCH(QString, value1);
    QFETCH(QString, value2);

    AVTwoWayHash<QString, QString> instance;
    instance.insertMulti(key, value1);
    instance.insertMulti(key, value2);

    QVERIFY(instance.contains(key));
    QCOMPARE(instance.values(key).size(), 2);
    QVERIFY(instance.values(key).at(0) == value1 || instance.values(key).at(0) == value2);
    QVERIFY(instance.values(key).at(1) == value1 || instance.values(key).at(1) == value2);

    QVERIFY(instance.m_value_hash.contains(value1));
    QVERIFY(instance.m_value_hash.contains(value2));
    QCOMPARE(instance.m_value_hash.values(value1).size(), 1);
    QCOMPARE(instance.m_value_hash.values(value2).size(), 1);
    QCOMPARE(instance.m_value_hash.value(value1), key);
    QCOMPARE(instance.m_value_hash.value(value2), key);

    AVLogInfo << "---- test_insertMulti ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_key_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("ret_val");

    QTest::newRow("row1") << "key1.value3" << "key1";
    QTest::newRow("row2") << "key2.value2" << "key2";
    QTest::newRow("row3") << "key1.value1" << "key1";
}

void TstAVTwoWayHash::test_key()
{
    AVLogInfo << "---- test_key launched ----";

    QFETCH(QString, value);
    QFETCH(QString, ret_val);

    AVTwoWayHash<QString, QString> instance;

    instance.insertMulti("key1", "key1.value1");
    instance.insertMulti("key1", "key1.value2");
    instance.insertMulti("key1", "key1.value3");
    instance.insertMulti("key2", "key2.value1");
    instance.insertMulti("key2", "key2.value2");
    instance.insertMulti("key2", "key2.value3");

    QCOMPARE(instance.key(value), ret_val);

    AVLogInfo << "---- test_key ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_key_2_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("defaultKey");
    QTest::addColumn<QString>("ret_val");

    QTest::newRow("row1") << "unknown_value" << "key_for_unknown_value" << "key_for_unknown_value";
    QTest::newRow("row2") << "key2.value3" << "key_for_unknown_value" << "key2";
}

void TstAVTwoWayHash::test_key_2()
{
    AVLogInfo << "---- test_key_2 launched ----";

    QFETCH(QString, value);
    QFETCH(QString, defaultKey);
    QFETCH(QString, ret_val);

    AVTwoWayHash<QString, QString> instance;

    instance.insertMulti("key1", "key1.value1");
    instance.insertMulti("key1", "key1.value2");
    instance.insertMulti("key1", "key1.value3");
    instance.insertMulti("key2", "key2.value1");
    instance.insertMulti("key2", "key2.value2");
    instance.insertMulti("key2", "key2.value3");

    QCOMPARE(instance.key(value, defaultKey), ret_val);

    AVLogInfo << "---- test_key_2 ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_keys_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<QList<QString> >("ret_val");

    QTest::newRow("row1") << "value" << ((QList<QString>() += "key1") += "key2");
}

void TstAVTwoWayHash::test_keys()
{
    AVLogInfo << "---- test_keys launched ----";

    QFETCH(QString, value);
    QFETCH(QList<QString>, ret_val);

    AVTwoWayHash<QString, QString> instance;

    instance.insertMulti("key1", "value");
    instance.insertMulti("key2", "value");

    QList<QString> key_list = instance.keys(value);
    qSort(key_list);
    QCOMPARE(key_list, ret_val);

    AVLogInfo << "---- test_keys ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_remove_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<int>("ret_val");

    QTest::newRow("row1") << "key2" << 2;
}

void TstAVTwoWayHash::test_remove()
{
    AVLogInfo << "---- test_remove launched ----";

    QFETCH(QString, key);
    QFETCH(int, ret_val);

    AVTwoWayHash<QString, QString> instance;

    instance.insertMulti("key1", "key1.value1");
    instance.insertMulti("key1", "key1.value2");
    instance.insertMulti("key1", "key1.value3");
    instance.insertMulti("key2", "key2.value1");
    instance.insertMulti("key2", "key2.value2");

    QCOMPARE(instance.remove(key), ret_val);
    QVERIFY(!instance.m_value_hash.contains(key + ".value1"));
    QVERIFY(!instance.m_value_hash.contains(key + ".value2"));

    AVLogInfo << "---- test_remove ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_take_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("ret_val");

    QTest::newRow("row1") << "key" << "value";
}

void TstAVTwoWayHash::test_take()
{
    AVLogInfo << "---- test_take launched ----";

    QFETCH(QString, key);
    QFETCH(QString, ret_val);

    AVTwoWayHash<QString, QString> instance;
    instance.insert(key, ret_val);
    QCOMPARE(instance.take(key), ret_val);
    QVERIFY(!instance.m_value_hash.contains(ret_val));

    AVLogInfo << "---- test_take ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_unite()
{
    AVLogInfo << "---- test_unite launched ----";

    AVTwoWayHash<QString, QString> instance1;
    instance1.insertMulti("key1", "key1.value1");

    AVTwoWayHash<QString, QString> instance2;
    instance2.insertMulti("key2", "key2.value1");
    instance2.insertMulti("key2", "key2.value2");

    const AVTwoWayHash<QString, QString> &united = instance1.unite(instance2);
    QCOMPARE(united.values("key1").size(), 1);
    QCOMPARE(united.values("key2").size(), 2);
    QCOMPARE(united.m_value_hash.value("key1.value1"), QString("key1"));
    QCOMPARE(united.m_value_hash.value("key2.value1"), QString("key2"));
    QCOMPARE(united.m_value_hash.value("key2.value2"), QString("key2"));

    AVLogInfo << "---- test_unite ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVTwoWayHash::test_removeFromValueHash()
{
    AVLogInfo << "---- test_removeFromValueHash launched ----";

    AVTwoWayHash<QString, QString> instance;
    instance.insertMulti("key", "key.value1");
    instance.insertMulti("key", "key.value2");

    instance.removeFromValueHash("key");
    QCOMPARE(instance.values("key").size(), 2);
    QVERIFY(!instance.m_value_hash.contains("key.value1"));
    QVERIFY(!instance.m_value_hash.contains("key.value2"));

    AVLogInfo << "---- test_removeFromValueHash ended    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTwoWayHash,"avlib/unittests/config")
#include "tstavtwowayhash.moc"

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
