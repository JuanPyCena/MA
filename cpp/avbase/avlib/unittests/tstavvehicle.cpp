///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2012
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Scheikl Gerhard, g.scheikl@avibit.com
    \brief   Function level test cases for AVVehicle
 */



#include <QtTest>
#include <avunittestmain.h>
#include <avvehicle.h>

class TstAVVehicle : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVVehicle();
    void test_textFromVehicleClass_data();
    void test_textFromVehicleClass();
    void test_vehicleClassFromText_data();
    void test_vehicleClassFromText();
    void test_setupMappings();

};

//! Declaration of unknown metatypes
Q_DECLARE_METATYPE(AVVehicle::VehicleClass);

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVVehicle::initTestCase()
{

}

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVVehicle::cleanupTestCase()
{

}

//! This will be called before each test function is executed.
void TstAVVehicle::init()
{

}

//! This will be called after every test function.
void TstAVVehicle::cleanup()
{

}

void TstAVVehicle::test_AVVehicle()
{
    AVVehicle instance;
    QVERIFY(instance.m_classToTextMapping.isEmpty());
    QVERIFY(instance.m_textToClassMapping.isEmpty());
}

void TstAVVehicle::test_textFromVehicleClass_data()
{
    QTest::addColumn<AVVehicle::VehicleClass>("vehicle_class");
    QTest::addColumn<QString>("ret_val");

    QTest::newRow("INVALID")      << AVVehicle::VEH_CLASS_INVALID << QString("INVALID");
    QTest::newRow("FOLLOWME")     << AVVehicle::VEH_CLASS_FOLLOWME << QString("FOLLOWME");
    QTest::newRow("ATC_EQ_MAINTENANCE")  << AVVehicle::VEH_CLASS_ATC_EQ_MAINTENANCE
                                         << QString("ATC_EQ_MAINTENANCE");
    QTest::newRow("AIRPORT_MAINTENANCE") << AVVehicle::VEH_CLASS_AIRPORT_MAINTENANCE
                                         << QString("AIRPORT_MAINTENANCE");
    QTest::newRow("FIRE")         << AVVehicle::VEH_CLASS_FIRE << QString("FIRE");
    QTest::newRow("BIRD_SCARER")  << AVVehicle::VEH_CLASS_BIRD_SCARER << QString("BIRD_SCARER");
    QTest::newRow("SNOW_PLOUGH")  << AVVehicle::VEH_CLASS_SNOW_PLOUGH << QString("SNOW_PLOUGH");
    QTest::newRow("RWY_SWEEPER")  << AVVehicle::VEH_CLASS_RWY_SWEEPER << QString("RWY_SWEEPER");
    QTest::newRow("EMERGENCY")    << AVVehicle::VEH_CLASS_EMERGENCY << QString("EMERGENCY");
    QTest::newRow("POLICE")       << AVVehicle::VEH_CLASS_POLICE << QString("POLICE");
    QTest::newRow("BUS")          << AVVehicle::VEH_CLASS_BUS << QString("BUS");
    QTest::newRow("TUG")          << AVVehicle::VEH_CLASS_TUG << QString("TUG");
    QTest::newRow("GRASS_CUTTER") << AVVehicle::VEH_CLASS_GRASS_CUTTER << QString("GRASS_CUTTER");
    QTest::newRow("FUEL")         << AVVehicle::VEH_CLASS_FUEL << QString("FUEL");
    QTest::newRow("BAGGAGE")      << AVVehicle::VEH_CLASS_BAGGAGE << QString("BAGGAGE");
    QTest::newRow("CATERING")     << AVVehicle::VEH_CLASS_CATERING << QString("CATERING");
    QTest::newRow("ACFT_MAINTENANCE") << AVVehicle::VEH_CLASS_ACFT_MAINTENANCE
                                      << QString("ACFT_MAINTENANCE");
    QTest::newRow("UNKNOWN")      << AVVehicle::VEH_CLASS_UNKNOWN << QString("UNKNOWN");
}

void TstAVVehicle::test_textFromVehicleClass()
{
    QFETCH(AVVehicle::VehicleClass, vehicle_class);
    QFETCH(QString, ret_val);

    AVVehicle instance;
    QCOMPARE(instance.textFromVehicleClass(vehicle_class), ret_val);
}

void TstAVVehicle::test_vehicleClassFromText_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<AVVehicle::VehicleClass>("ret_val");

    QTest::newRow("INVALID")      << QString("INVALID")      << AVVehicle::VEH_CLASS_INVALID;
    QTest::newRow("FOLLOWME")     << QString("FOLLOWME")     << AVVehicle::VEH_CLASS_FOLLOWME;
    QTest::newRow("ATC_EQ_MAINTENANCE")  << QString("ATC_EQ_MAINTENANCE")
                                         << AVVehicle::VEH_CLASS_ATC_EQ_MAINTENANCE;
    QTest::newRow("AIRPORT_MAINTENANCE") << QString("AIRPORT_MAINTENANCE")
                                         << AVVehicle::VEH_CLASS_AIRPORT_MAINTENANCE;
    QTest::newRow("FIRE")         << QString("FIRE")         << AVVehicle::VEH_CLASS_FIRE;
    QTest::newRow("BIRD_SCARER")  << QString("BIRD_SCARER")  << AVVehicle::VEH_CLASS_BIRD_SCARER;
    QTest::newRow("SNOW_PLOUGH")  << QString("SNOW_PLOUGH")  << AVVehicle::VEH_CLASS_SNOW_PLOUGH;
    QTest::newRow("RWY_SWEEPER")  << QString("RWY_SWEEPER")  << AVVehicle::VEH_CLASS_RWY_SWEEPER;
    QTest::newRow("EMERGENCY")    << QString("EMERGENCY")    << AVVehicle::VEH_CLASS_EMERGENCY;
    QTest::newRow("POLICE")       << QString("POLICE")       << AVVehicle::VEH_CLASS_POLICE;
    QTest::newRow("BUS")          << QString("BUS")          << AVVehicle::VEH_CLASS_BUS;
    QTest::newRow("TUG")          << QString("TUG")          << AVVehicle::VEH_CLASS_TUG;
    QTest::newRow("GRASS_CUTTER") << QString("GRASS_CUTTER") << AVVehicle::VEH_CLASS_GRASS_CUTTER;
    QTest::newRow("FUEL")         << QString("FUEL")         << AVVehicle::VEH_CLASS_FUEL;
    QTest::newRow("BAGGAGE")      << QString("BAGGAGE")      << AVVehicle::VEH_CLASS_BAGGAGE;
    QTest::newRow("CATERING")     << QString("CATERING")     << AVVehicle::VEH_CLASS_CATERING;
    QTest::newRow("ACFT_MAINTENANCE") << QString("ACFT_MAINTENANCE")
                                      << AVVehicle::VEH_CLASS_ACFT_MAINTENANCE;
    QTest::newRow("UNKNOWN")      << QString("UNKNOWN")      << AVVehicle::VEH_CLASS_UNKNOWN;
}

void TstAVVehicle::test_vehicleClassFromText()
{
    QFETCH(QString, text);
    QFETCH(AVVehicle::VehicleClass, ret_val);

    AVVehicle instance;
    QCOMPARE(instance.vehicleClassFromText(text), ret_val);
}

void TstAVVehicle::test_setupMappings()
{
    AVVehicle instance;
    instance.setupMappings();
    QCOMPARE(instance.m_classToTextMapping.size(), 18);
    QCOMPARE(instance.m_textToClassMapping.size(), 18);
}

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVVehicle,"avlib/unittests/config")
#include "tstavvehicle.moc"

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
