///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2014
// QT-Version: QT4
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Alexander Randeu, a.randeu@avibit.com
  \author  QT4-PORT: Alexander Randeu, a.randeu@avibit.com
  \brief   Unittests for AVSystemTrafo class
*/

// Qt includes
#include <QTest>
#include <QList>

#include <avunittestmain.h>

// AVLib includes
#include "avmisc.h"

// AVCalculationLib
#include "avsystemtrafo.h"

//---------------------------------------------------------------------------

class AVSystemTrafoTest : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:

    void test_AVSystemTrafo();

private:

};

//---------------------------------------------------------------------------

void AVSystemTrafoTest::initTestCase()
{
}

//---------------------------------------------------------------------------

void AVSystemTrafoTest::cleanupTestCase()
{
}

//---------------------------------------------------------------------------

void AVSystemTrafoTest::init()
{
}

//---------------------------------------------------------------------------

void AVSystemTrafoTest::cleanup()
{
}

//---------------------------------------------------------------------------

void AVSystemTrafoTest::test_AVSystemTrafo()
{
    // origin_lat, origin_long, testpoint_lat, testpoint_lon
    // output (testpoint_x, testpoint_y) currently not tested, only if trafo latlong2xy -> xy2latlong results
    // in same (testpoint_lat, testpoint_lon) coordinates
    QList<double> testdata_lat_origins;
    testdata_lat_origins << -90.0 << -89.9
                         << -45.0
                         << -0.1 << 0.0 << 0.1
                         << 45.0
                         << 89.9 << 90.0;
    QList<double> testdata_lon_origins;
    testdata_lon_origins << -180.0 << -179.9
                         << -135.0
                         << -90.1 << -90.0 << -89.9
                         << -45.0
                         << -0.1 << 0.0 << 0.1
                         << 45.0
                         << 89.9 << 90.0 << 91.1
                         << 135.0
                         << 179.9 << 180.0;

    const double max_delta = 1e-6; // less than 1m for earth radius of 6378km (1m/6378000m/pi*180=8.9833e-06degs)
    const double offset_point_lat_lon = 0.2;

    QList<double>::iterator it_lat_origin;
    for (it_lat_origin = testdata_lat_origins.begin(); it_lat_origin != testdata_lat_origins.end(); ++it_lat_origin)
    {
        double origin_lat = *(it_lat_origin);

        QList<double>::iterator it_lon_origin;
        for (it_lon_origin = testdata_lon_origins.begin(); it_lon_origin != testdata_lon_origins.end(); ++it_lon_origin)
        {
            double origin_lon = *(it_lon_origin);

            // tests to exclude -> need to be checked in code!
            //if(origin_lat == -90.0 /*&& origin_lon == -180.0*/) continue;

            AVSystemTrafo trafo(origin_lat, origin_lon);

            // test around origin to make sure we also test if point is in other quadrant
            for(int lat_offset = -1; lat_offset <= 1; ++lat_offset)
            {
                for(int lon_offset = -1; lon_offset <= 1; ++lon_offset)
                {
                    double test_lat = origin_lat + lat_offset*offset_point_lat_lon;
                    double test_lon = origin_lon + lon_offset*offset_point_lat_lon;

                    // do not go over poles lat > 90 or lat < -90
                    if(test_lat > 90.0 || test_lat < -90.0) continue;
                    // TODO: this ones need to be checked
                    if(test_lon > 180.0 || test_lon < -180.0) continue;

                    double test_x = 0.0;
                    double test_y = 0.0;

                    QVERIFY(trafo.latlong2xy(test_lat, test_lon, test_x, test_y));
                    double test_lat1 = 0.0;
                    double test_lon1 = 0.0;
                    QVERIFY(trafo.xy2latlong(test_x, test_y, test_lat1, test_lon1));

                    if(fabs(test_lat - test_lat1) >= max_delta || fabs(test_lon - test_lon1) >= max_delta)
                    {
                        AVLogError << "test_lat=" << test_lat << ", test_lat1=" << test_lat1 << ", test_lon=" << test_lon
                                   << ", test_lon1="<< test_lon1 << ", test_x=" << test_x << ", test_y=" << test_y;
                    }
                    QVERIFY2(AVFLOATEQ(test_lat, test_lat1, max_delta ),
                                       qPrintable(QString("Latitude: expexted: %1, but got: %2").arg(test_lat).arg(test_lat1)));
                    QVERIFY2(AVFLOATEQ(test_lon, test_lon1, max_delta ),
                                       qPrintable(QString("Longitude: expexted: %1, but got: %2").arg(test_lon).arg(test_lon1)));
                }
            }
        }
    }
}

//---------------------------------------------------------------------------

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVSystemTrafoTest,"avcalculationlib/unittests/config")
#include "tstavsystemtrafo.moc"

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
