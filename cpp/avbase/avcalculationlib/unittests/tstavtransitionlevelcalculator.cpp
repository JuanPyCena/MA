//-----------------------------------------------------------------------------
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2016
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
//-----------------------------------------------------------------------------

/*! \file
    \brief   Function level test cases for AVTransitionLevelCalculatorBarometric
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avtransitionlevel.h"

//-----------------------------------------------------------------------------
class TstAVTransitionLevelCalculator : public QObject
{
    Q_OBJECT

private slots:
    //! Uncommented test data is from the UK document
    //! "CAP 493 - Manual of Air Traffic Services Part 1 - Edition 5"
    //! Appendix A page 1
    void test_doComputeLevelBarometric_data();
    void test_doComputeLevelBarometric();

    void test_doComputeLevelBarometricMinimumTransitionLayer();

    void test_doComputeLevelTable_data();
    void test_doComputeLevelTable();
};

//-----------------------------------------------------------------------------

void TstAVTransitionLevelCalculator::test_doComputeLevelBarometric_data()
{
    QTest::addColumn<uint>("transition_altitude");
    QTest::addColumn<uint>("qnh");
    QTest::addColumn<int>("ret_val");

    QTest::newRow("") << 6000u <<  943u << 80;
    QTest::newRow("") << 6000u <<  959u << 80;
    QTest::newRow("") << 6000u <<  960u << 75;
    QTest::newRow("") << 6000u <<  977u << 75;
    QTest::newRow("") << 6000u <<  978u << 70;
    QTest::newRow("") << 6000u <<  995u << 70;
    QTest::newRow("") << 6000u <<  996u << 65;
    QTest::newRow("") << 6000u << 1013u << 65;
    QTest::newRow("") << 6000u << 1014u << 60;
    QTest::newRow("") << 6000u << 1031u << 60;
    QTest::newRow("") << 6000u << 1032u << 55;
    QTest::newRow("") << 6000u << 1050u << 55;

    QTest::newRow("") << 5000u <<  943u << 70;
    QTest::newRow("") << 5000u <<  959u << 70;
    QTest::newRow("") << 5000u <<  960u << 65;
    QTest::newRow("") << 5000u <<  977u << 65;
    QTest::newRow("") << 5000u <<  978u << 60;
    QTest::newRow("") << 5000u <<  995u << 60;
    QTest::newRow("") << 5000u <<  996u << 55;
    QTest::newRow("") << 5000u << 1013u << 55;
    QTest::newRow("") << 5000u << 1014u << 50;
    QTest::newRow("") << 5000u << 1031u << 50;
    QTest::newRow("") << 5000u << 1032u << 45;
    QTest::newRow("") << 5000u << 1050u << 45;

    QTest::newRow("") << 4000u <<  943u << 60;
    QTest::newRow("") << 4000u <<  959u << 60;
    QTest::newRow("") << 4000u <<  960u << 55;
    QTest::newRow("") << 4000u <<  977u << 55;
    QTest::newRow("") << 4000u <<  978u << 50;
    QTest::newRow("") << 4000u <<  995u << 50;
    QTest::newRow("") << 4000u <<  996u << 45;
    QTest::newRow("") << 4000u << 1013u << 45;
    QTest::newRow("") << 4000u << 1014u << 40;
    QTest::newRow("") << 4000u << 1031u << 40;
    QTest::newRow("") << 4000u << 1032u << 35;
    QTest::newRow("") << 4000u << 1050u << 35;

    QTest::newRow("") << 3000u <<  943u << 50;
    QTest::newRow("") << 3000u <<  959u << 50;
    QTest::newRow("") << 3000u <<  960u << 45;

    QTest::newRow("") << 3000u <<  977u << 45;
    QTest::newRow("") << 3000u <<  978u << 40;
    QTest::newRow("") << 3000u <<  995u << 40;
    QTest::newRow("") << 3000u <<  996u << 35;
    QTest::newRow("") << 3000u << 1013u << 35;
    QTest::newRow("") << 3000u << 1014u << 30;
    QTest::newRow("") << 3000u << 1031u << 30;
    QTest::newRow("") << 3000u << 1032u << 25;
    QTest::newRow("") << 3000u << 1050u << 25;

    // border case: high QNH, check that TFL become negative
    QTest::newRow("") << 1000u << 1100u << -1;

    // Invalid QNH values
    QTest::newRow("") << 6000u << 849u << -1;
    QTest::newRow("") << 6000u << 1101u << -1;
}

//-----------------------------------------------------------------------------

void TstAVTransitionLevelCalculator::test_doComputeLevelBarometric()
{
    QFETCH(uint, transition_altitude);
    QFETCH(uint, qnh);
    QFETCH(int, ret_val);

    AVTransitionLevelCalculatorBarometric calculator;
    calculator.m_granularity = 5;
    calculator.m_transition_altitude = transition_altitude;
    QCOMPARE(calculator.computeTransitionLevel(qnh), ret_val);
}

//-----------------------------------------------------------------------------

//! This tests the example from http://board.vacc-sag.org/13/55638/

void TstAVTransitionLevelCalculator::test_doComputeLevelBarometricMinimumTransitionLayer()
{
    AVTransitionLevelCalculatorBarometric calculator;

    calculator.m_transition_altitude = 5000;
    calculator.m_minimum_transition_layer = 1000;
    calculator.m_granularity = 10;

    QCOMPARE(calculator.computeTransitionLevel(1013), 70);
    QCOMPARE(calculator.computeTransitionLevel(1014), 60);
}

//-----------------------------------------------------------------------------

void TstAVTransitionLevelCalculator::test_doComputeLevelTable_data()
{
    // Test data is for transition altitude of 6000 feet
    QTest::addColumn<uint>("qnh");
    QTest::addColumn<int>("expected_flight_level");

    QTest::newRow("too high") << 1061u << -1;

    QTest::newRow("1050 - 1060: max") << 1060u << 60;
    QTest::newRow("1050 - 1060: mid") << 1055u << 60;
    QTest::newRow("1050 - 1060: min") << 1050u << 60;

    QTest::newRow("1032 - 1049: max") << 1049u << 65;
    QTest::newRow("1032 - 1049: mid") << 1040u << 65;
    QTest::newRow("1032 - 1049: min") << 1032u << 65;

    QTest::newRow("1014 - 1031: max") << 1031u << 70;
    QTest::newRow("1014 - 1031: mid") << 1022u << 70;
    QTest::newRow("1014 - 1031: min") << 1014u << 70;

    QTest::newRow(" 995 - 1013: max") << 1013u << 75;
    QTest::newRow(" 995 - 1013: mid") << 1004u << 75;
    QTest::newRow(" 995 - 1013: min") <<  995u << 75;

    QTest::newRow(" 977 -  994: max") <<  994u << 80;
    QTest::newRow(" 977 -  994: mid") <<  985u << 80;
    QTest::newRow(" 977 -  994: min") <<  977u << 80;

    QTest::newRow(" 959 -  976: max") <<  976u << 85;
    QTest::newRow(" 959 -  976: mid") <<  967u << 85;
    QTest::newRow(" 959 -  976: min") <<  959u << 85;

    QTest::newRow(" 940 -  958: max") <<  958u << 90;
    QTest::newRow(" 940 -  958: mid") <<  949u << 90;
    QTest::newRow(" 940 -  958: min") <<  940u << 90;

    QTest::newRow("too low") << 939u << -1;
}

//-----------------------------------------------------------------------------

void TstAVTransitionLevelCalculator::test_doComputeLevelTable()
{
    // Test data is for transition altitude of 6000 feet
    QFETCH(uint, qnh);
    QFETCH(int, expected_flight_level);

    QMap<QPair<uint, uint>, int> table;
    table.insert(QPair<uint, uint>(1050u, 1060u), 60);
    table.insert(QPair<uint, uint>(1032u, 1049u), 65);
    table.insert(QPair<uint, uint>(1014u, 1031u), 70);
    table.insert(QPair<uint, uint>( 995u, 1013u), 75);
    table.insert(QPair<uint, uint>( 977u,  994u), 80);
    table.insert(QPair<uint, uint>( 959u,  976u), 85);
    table.insert(QPair<uint, uint>( 940u,  958u), 90);

    AVTransitionLevelCalculatorTable calculator;
    calculator.m_table = table;
    QCOMPARE(calculator.computeTransitionLevel(qnh), expected_flight_level);
}

//-----------------------------------------------------------------------------

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVTransitionLevelCalculator,"avcalculationlib/unittests/config")
#include "tstavtransitionlevelcalculator.moc"


// End of file
