///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Stefan Kunz, s.kunz@avibit.com
    \brief   Function level test cases for AVSciConst
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avsciconst.h"

class TstAVSciConst : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_convdegTorad_data();
    void test_convdegTorad();
    void test_convradTodeg_data();
    void test_convradTodeg();
    void test_convdegToheading_data();
    void test_convdegToheading();
    void test_convheadingTodeg_data();
    void test_convheadingTodeg();
    void test_convktTomps_data();
    void test_convktTomps();
    void test_convmpsTokt_data();
    void test_convmpsTokt();
    void test_convmToft_data();
    void test_convmToft();
    void test_convftTom_data();
    void test_convftTom();
    void test_convFLTom_data();
    void test_convFLTom();
    void test_convmToFL_data();
    void test_convmToFL();
    void test_convnmTokm_data();
    void test_convnmTokm();
    void test_convnmTom_data();
    void test_convnmTom();
    void test_convmTonm_data();
    void test_convmTonm();
    void test_convkmTonm_data();
    void test_convkmTonm();
    void test_convdegTodms_data();
    void test_convdegTodms();
    void test_convdmsTodeg_data();
    void test_convdmsTodeg();
    void test_rounddms_data();
    void test_rounddms();
    void test_convdegTodmsString_data();
    void test_convdegTodmsString();
    void test_convdmsStringTodeg_data();
    void test_convdmsStringTodeg();
    void test_convdmsAIXMStringTodeg_data();
    void test_convdmsAIXMStringTodeg();
    void test_normaliseDeg_data();
    void test_normaliseDeg();
    void test_AVSciConst();

};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVSciConst::initTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVSciConst::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVSciConst::init()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVSciConst::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdegTorad_data()
{
#if 0
    QTest::addColumn<double>("deg");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convdegTorad()
{
    AVLogInfo << "---- test_convdegTorad launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, deg);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convdegTorad(deg), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convdegTorad ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convradTodeg_data()
{
#if 0
    QTest::addColumn<double>("rad");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convradTodeg()
{
    AVLogInfo << "---- test_convradTodeg launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, rad);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convradTodeg(rad), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convradTodeg ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdegToheading_data()
{
#if 0
    QTest::addColumn<double>("deg");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convdegToheading()
{
    AVLogInfo << "---- test_convdegToheading launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, deg);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convdegToheading(deg), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convdegToheading ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convheadingTodeg_data()
{
#if 0
    QTest::addColumn<double>("h");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convheadingTodeg()
{
    AVLogInfo << "---- test_convheadingTodeg launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, h);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convheadingTodeg(h), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convheadingTodeg ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convktTomps_data()
{
#if 0
    QTest::addColumn<double>("kt");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convktTomps()
{
    AVLogInfo << "---- test_convktTomps launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, kt);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convktTomps(kt), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convktTomps ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convmpsTokt_data()
{
#if 0
    QTest::addColumn<double>("mps");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convmpsTokt()
{
    AVLogInfo << "---- test_convmpsTokt launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, mps);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convmpsTokt(mps), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convmpsTokt ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convmToft_data()
{
#if 0
    QTest::addColumn<double>("m");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convmToft()
{
    AVLogInfo << "---- test_convmToft launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, m);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convmToft(m), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convmToft ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convftTom_data()
{
#if 0
    QTest::addColumn<double>("ft");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convftTom()
{
    AVLogInfo << "---- test_convftTom launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, ft);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convftTom(ft), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convftTom ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convFLTom_data()
{
#if 0
    QTest::addColumn<double>("fl");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convFLTom()
{
    AVLogInfo << "---- test_convFLTom launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, fl);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convFLTom(fl), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convFLTom ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convmToFL_data()
{
#if 0
    QTest::addColumn<double>("m");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convmToFL()
{
    AVLogInfo << "---- test_convmToFL launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, m);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convmToFL(m), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convmToFL ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convnmTokm_data()
{
#if 0
    QTest::addColumn<double>("nm");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convnmTokm()
{
    AVLogInfo << "---- test_convnmTokm launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, nm);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convnmTokm(nm), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convnmTokm ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convnmTom_data()
{
#if 0
    QTest::addColumn<double>("nm");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convnmTom()
{
    AVLogInfo << "---- test_convnmTom launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, nm);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convnmTom(nm), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convnmTom ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convmTonm_data()
{
#if 0
    QTest::addColumn<double>("m");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convmTonm()
{
    AVLogInfo << "---- test_convmTonm launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, m);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convmTonm(m), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convmTonm ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convkmTonm_data()
{
#if 0
    QTest::addColumn<double>("km");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_convkmTonm()
{
    AVLogInfo << "---- test_convkmTonm launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, km);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convkmTonm(km), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convkmTonm ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdegTodms_data()
{
#if 0
    QTest::addColumn<double>("deg");
    QTest::addColumn<bool>("signbit");
    QTest::addColumn<int>("d");
    QTest::addColumn<int>("m");
    QTest::addColumn<double>("s");

    QTest::newRow("row1") << double() << bool() << int() << int() << double();
#endif
}

void TstAVSciConst::test_convdegTodms()
{
    AVLogInfo << "---- test_convdegTodms launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, deg);
    QFETCH(bool, signbit);
    QFETCH(int, d);
    QFETCH(int, m);
    QFETCH(double, s);

    AVSciConst instance;
    instance.convdegTodms(deg, signbit, d, m, s);
    QCOMPARE(XXX, XXX);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convdegTodms ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdmsTodeg_data()
{
#if 0
    QTest::addColumn<double>("deg");
    QTest::addColumn<bool>("signbit");
    QTest::addColumn<int>("d");
    QTest::addColumn<int>("m");
    QTest::addColumn<double>("s");

    QTest::newRow("row1") << double() << bool() << int() << int() << double();
#endif
}

void TstAVSciConst::test_convdmsTodeg()
{
    AVLogInfo << "---- test_convdmsTodeg launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, deg);
    QFETCH(bool, signbit);
    QFETCH(int, d);
    QFETCH(int, m);
    QFETCH(double, s);

    AVSciConst instance;
    instance.convdmsTodeg(deg, signbit, d, m, s);
    QCOMPARE(XXX, XXX);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convdmsTodeg ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_rounddms_data()
{
#if 0
    QTest::addColumn<bool>("signbit");
    QTest::addColumn<int>("d");
    QTest::addColumn<int>("m");
    QTest::addColumn<double>("s");
    QTest::addColumn<int>("pr");

    QTest::newRow("row1") << bool() << int() << int() << double() << int();
#endif
}

void TstAVSciConst::test_rounddms()
{
    AVLogInfo << "---- test_rounddms launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(bool, signbit);
    QFETCH(int, d);
    QFETCH(int, m);
    QFETCH(double, s);
    QFETCH(int, pr);

    AVSciConst instance;
    instance.rounddms(signbit, d, m, s, pr);
    QCOMPARE(XXX, XXX);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_rounddms ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdegTodmsString_data()
{
    QTest::addColumn<double>("deg");
    QTest::addColumn<int>("pr");
    QTest::addColumn<QString>("ret_val");

    // precision
    double dec(45.231384);
    QTest::newRow("pr 0") << dec << 0 << QString("45 13 53");
    QTest::newRow("pr 1") << dec << 1 << QString("45 13 53.0");
    QTest::newRow("pr 2") << dec << 2 << QString("45 13 52.98");
    QTest::newRow("pr 3") << dec << 3 << QString("45 13 52.982");

    // some test cases
    QTest::newRow("0.0,") << double(0.0) << 1 << QString("0 00 00.0");
    QTest::newRow("90.0") << double(90.0) << 1 << QString("90 00 00.0");
    QTest::newRow("-90.0") << double(-90.0) << 1 << QString("-90 00 00.0");
    QTest::newRow("180.0") << double(180.0) << 1 << QString("180 00 00.0");
    QTest::newRow("-180.0") << double(-180.0) << 1 << QString("-180 00 00.0");
    QTest::newRow("0.00003") << double(0.00003) << 3 << QString("0 00 00.108");
    QTest::newRow("-0.00003") << double(-0.00003) << 3 << QString("-0 00 00.108");
    QTest::newRow("89.99972") << double(89.99972) << 3 << QString("89 59 58.992");
    QTest::newRow("179.99972") << double(179.99972) << 3 << QString("179 59 58.992");
    QTest::newRow("47.2292") << double(47.2292) << 3 << QString("47 13 45.120");
    QTest::newRow("-21.08602") << double(-21.08602) << 3 << QString("-21 05 09.672");

}

void TstAVSciConst::test_convdegTodmsString()
{
    AVLogInfo << "---- test_convdegTodmsString launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(double, deg);
    QFETCH(int, pr);
    QFETCH(QString, ret_val);

    QCOMPARE(AVSciConst::convdegTodmsString(deg, pr), ret_val);

    AVLogInfo << "---- test_convdegTodmsString ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdmsStringTodeg_data()
{
#if 0
    QTest::addColumn<QString>("dms");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << QString() << double();
#endif
}

void TstAVSciConst::test_convdmsStringTodeg()
{
    AVLogInfo << "---- test_convdmsStringTodeg launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(QString, dms);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.convdmsStringTodeg(dms), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_convdmsStringTodeg ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_convdmsAIXMStringTodeg_data()
{
    QTest::addColumn<QString>("dms");
    QTest::addColumn<double>("ret_val");

    // valid inputs
    QTest::newRow("000000N")  << QString("000000N")  << double(0.0);
    QTest::newRow("900000N")  << QString("900000N")  << double(90.0);
    QTest::newRow("000000S")  << QString("000000S")  << double(0.0);
    QTest::newRow("900000S")  << QString("900000S")  << double(-90.0);
    QTest::newRow("0000000E") << QString("0000000E") << double(0.0);
    QTest::newRow("1800000E") << QString("1800000E") << double(180.0);
    QTest::newRow("0000000W") << QString("0000000W") << double(0.0);
    QTest::newRow("1800000W") << QString("1800000W") << double(-180.0);

    QTest::newRow("470000.0000N")    << QString("470000.0000N")    << double(47.0);
    QTest::newRow("470000N")         << QString("470000N")         << double(47.0);
    QTest::newRow("4700N")           << QString("4700N")           << double(47.0);
    QTest::newRow("47N")             << QString("47N")             << double(47.0);

    // reference values based on http://andrew.hedges.name/experiments/convert_lat_long/
    QTest::newRow("000000.01N")      << QString("000000.01N")      << double(0.0);
    QTest::newRow("000000.1N")       << QString("000000.1N")       << double(0.00003);
    QTest::newRow("000001N")         << QString("000001N")         << double(0.00028);
    QTest::newRow("895959N")         << QString("895959N")         << double(89.99972);
    QTest::newRow("1795959E")        << QString("1795959E")        << double(179.99972);
    QTest::newRow("895959.9999N")    << QString("895959.9999N")    << double(90.0);
    QTest::newRow("1795959.9999E")   << QString("1795959.9999E")   << double(180.0);

    QTest::newRow("471345N")         << QString("471345N")         << double(47.22917);
    QTest::newRow("471345.1234N")    << QString("471345.1234N")    << double(47.2292);
    QTest::newRow("4713N")           << QString("4713N")           << double(47.21667);
    QTest::newRow("4713.12345678N")  << QString("4713.12345678N")  << double(47.21872);
    QTest::newRow("47.12345678N")    << QString("47.12345678N")    << double(47.12346);
    QTest::newRow("13.12345678S")    << QString("13.12345678S")    << double(-13.12346);
    QTest::newRow("0210509.6789W")   << QString("0210509.6789W")   << double(-21.08602);
    QTest::newRow("1210509.4567E")   << QString("1210509.4567E")   << double(121.08596);

    // invalid inputs
    QTest::newRow("N")               << QString("N")               << double(0.0); // no coords
    QTest::newRow("45")              << QString("45")              << double(0.0); // NSWE missing
    QTest::newRow("471300")          << QString("471300")          << double(0.0); // NSWE missing
    QTest::newRow("90W")             << QString("90W")             << double(0.0); // WE with latitude
    QTest::newRow("90E")             << QString("90E")             << double(0.0); // WE with latitude
    QTest::newRow("180N")            << QString("180N")            << double(0.0); // SN with longitude
    QTest::newRow("180S")            << QString("180S")            << double(0.0); // SN with longitude
    QTest::newRow("90.1N")           << QString("90.1N")           << double(0.0); // lat > 90
    QTest::newRow("90.00000001N")    << QString("90.00000001N")    << double(0.0); // lat > 90
    QTest::newRow("9000.00000001N")  << QString("9000.00000001N")  << double(0.0); // lat > 90
    QTest::newRow("900000.0001N")    << QString("900000.0001N")    << double(0.0); // lat > 90
    QTest::newRow("180.00000001W")   << QString("180.00000001W")   << double(0.0); // lon > 180
    QTest::newRow("18000.00000001W") << QString("18000.00000001W") << double(0.0); // lon > 180
    QTest::newRow("1800000.0001W")   << QString("1800000.0001W")   << double(0.0); // lon > 180
    QTest::newRow("11.000000001N")   << QString("11.000000001N")   << double(0.0); // > 8 decimals
    QTest::newRow("1100.000000001N") << QString("1100.000000001N") << double(0.0); // > 8 decimals
    QTest::newRow("110000.00001N")   << QString("110000.00001N")   << double(0.0); // > 4 decimals
}

void TstAVSciConst::test_convdmsAIXMStringTodeg()
{
    AVLogInfo << "---- test_convdmsAIXMStringTodeg launched (" << QTest::currentDataTag() <<") ----";

    QFETCH(QString, dms);
    QFETCH(double, ret_val);

    AV_DOUBLE_COMPARE(AVSciConst::convdmsAIXMStringTodeg(dms), ret_val, 0.00001);

    AVLogInfo << "---- test_convdmsAIXMStringTodeg ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_normaliseDeg_data()
{
#if 0
    QTest::addColumn<double>("angle");
    QTest::addColumn<double>("ret_val");

    QTest::newRow("row1") << double() << double();
#endif
}

void TstAVSciConst::test_normaliseDeg()
{
    AVLogInfo << "---- test_normaliseDeg launched (" << QTest::currentDataTag() <<") ----";

#if 0
    QFETCH(double, angle);
    QFETCH(double, ret_val);

    AVSciConst instance;
    QCOMPARE(instance.normaliseDeg(angle), ret_val);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_normaliseDeg ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVSciConst::test_AVSciConst()
{
    AVLogInfo << "---- test_AVSciConst launched (" << QTest::currentDataTag() <<") ----";

#if 0
    AVSciConst instance;
    QCOMPARE(XXX, XXX);
#endif
    QSKIP("Test is not implemented.");

    AVLogInfo << "---- test_AVSciConst ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVSciConst,"avcalculationlib/unittests/config")
#include "tstavsciconst.moc"

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
