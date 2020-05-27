///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVCALCULATIONLIB - AviBit Calculation Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Madalina Hodorog, m.hodorog@avibit.com
    \brief   Function level test cases for AVCartesianToWGS84TargetTrafo
 */

#include <QtTest>
#include <avunittestmain.h>
#include "avdgstosystemtrafo.h"
#include "avsystemtrafo.h"
#include "avsciconst.h"

class TstAVDGSToSystemTrafo : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void test_AVDGSToSystemTrafo();
    void test_xyz2latlonh();
    void test_xyz2xySystemTrafo();
    void test_latlong2xySystemTrafo();
    void test_getRotationAngleFromOrdinate();
    void test_getRotationAngleFromNorth();
    void test_getRotatedCoordinates();

private:
    void convertToWgs84SystemTrafo(double point_cart[],
                                   double point_geodetic[]);
    void convertToWgs84Sensor(double point_cart[],
                              double point_geodetic[],
                              double laser_geodetic_origin[]);
    void convertCartesianFromTrafoToSensor(double origin_cart_system_trafo[],
                                           double rotation_angle,
                                           double point_cart_system_trafo[],
                                           double point_cart_sensor[]);
    QSharedPointer<AVSystemTrafo> m_system_trafo;
    QSharedPointer<AVSSRTrafo> m_sensor_trafo;
};

///////////////////////////////////////////////////////////////////////////////

//! Declaration of unknown metatypes
// Q_DECLARE_METATYPE(Point3DCartesian)

///////////////////////////////////////////////////////////////////////////////

//! This will be called before the first test function is executed.
//! It is only called once.
void TstAVDGSToSystemTrafo::initTestCase()
{
    AVLogger->setMinLevel(AVLog::LOG__DEBUG2);
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after the last test function is executed.
//! It is only called once.
void TstAVDGSToSystemTrafo::cleanupTestCase()
{

}

///////////////////////////////////////////////////////////////////////////////

//! This will be called before each test function is executed.
void TstAVDGSToSystemTrafo::init()
{
    m_system_trafo.clear();
    m_system_trafo = QSharedPointer<AVSystemTrafo>::create();

    m_sensor_trafo.clear();
    m_sensor_trafo = QSharedPointer<AVSSRTrafo>::create();
}

///////////////////////////////////////////////////////////////////////////////

//! This will be called after every test function.
void TstAVDGSToSystemTrafo::cleanup()
{

}

///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_AVDGSToSystemTrafo()
{
    AVLogInfo << "---- test_AVDGSToSystemTrafo launched (" << QTest::currentDataTag() <<") ----";

    double laser_lat        = 48.118182;
    double laser_long       = 16.56621;
    double laser_height     = 0.0;
    double reference_lat    = 48.11767;
    double reference_long   = 16.56541;
    double reference_height = 0.0;
    double reference_x      = 33.1254;
    double reference_y      = 77.1194;
    double reference_z      = 0.0;

    AVDGSToSystemTrafo instance(laser_lat, laser_long, laser_height,
                                reference_lat, reference_long, reference_height,
                                reference_x, reference_y, reference_z);
    QVERIFY(instance.m_laser_trafo != NULL);
    QVERIFY(instance.m_system_trafo != NULL);

    AVLogInfo << "---- test_AVDGSToSystemTrafo ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

namespace {

    struct GeoPos
    {
        GeoPos(const double lat, const double lon, const double h = 0) :

            m_lat(lat), m_lon(lon), m_h(h)
        {
        }

        double m_lat, m_lon, m_h;
    };
/* TODO currently unused
    QPointF latLonToLocalXY(const GeoPos& origin, const GeoPos& point_to_map)
    {
        const double EARTH_RADIUS(6378137);

        QPointF ret_point;
        ret_point.setX(deg2rad(point_to_map.m_lon - origin.m_lon) * EARTH_RADIUS *
                                                                    cos(deg2rad(origin.m_lat)));
        ret_point.setY(deg2rad(point_to_map.m_lat - origin.m_lat) * EARTH_RADIUS);
        return ret_point;
    }
    */
}

///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_xyz2latlonh()
{
    AVLogInfo << "---- test_xyz2latlonh launched (" << QTest::currentDataTag() <<") ----";

    GeoPos laser_position(48, 16);
    GeoPos reference_position_geo(0, 0);
    const double COURSE_LASER_TO_REFERENCE_POINT(45);
    const double DIST_TO_REFERENCE_POINT(30.922);
    QPointF reference_position_safegate(0, DIST_TO_REFERENCE_POINT);
    QPointF target_position_safegate(0, DIST_TO_REFERENCE_POINT);

    AVDGSToSystemTrafo instance(laser_position.m_lat,
                                laser_position.m_lon,
                                0.0,
                                reference_position_geo.m_lat,
                                reference_position_geo.m_lon,
                                0.0,
                                reference_position_safegate.x(),
                                reference_position_safegate.y(),
                                0.0);

    // creating input data for testing the conversion class
    AVAviationCalculator::pointAtCourseDist(laser_position.m_lat,
                                            laser_position.m_lon,
                                            DIST_TO_REFERENCE_POINT,
                                            COURSE_LASER_TO_REFERENCE_POINT,
                                            reference_position_geo.m_lat,
                                            reference_position_geo.m_lon);
    double target_lat;
    double target_lon;
    double target_h;

    double expected_target_lat = 48;
    double expected_target_lon = 16;

    instance.xyz2latlonh(target_position_safegate.x(),
                         target_position_safegate.y(),
                         0.0,
                         target_lat, target_lon, target_h);

    AVLogInfo << "reference_position_geo: " << reference_position_geo.m_lat << "|" << reference_position_geo.m_lon;
    AVLogInfo << "target_position: " << target_lat << "|" << target_lon;
    QCOMPARE(target_lat, expected_target_lat);
    QCOMPARE(target_lon, expected_target_lon);

    AVLogInfo << "---- test_xyz2latlonh ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::convertToWgs84SystemTrafo(double point_cart[],
                                                      double point_geodetic[3])
{
    m_system_trafo->xy2latlong(point_cart[0],
                               point_cart[1],
                               point_geodetic[0],
                               point_geodetic[1]);
}

void TstAVDGSToSystemTrafo::convertToWgs84Sensor(double point_cart[],
                                                 double point_geodetic[],
                                                 double laser_geodetic_origin[])
{
    AVASSERT(m_sensor_trafo->setRadarPos(laser_geodetic_origin[0],
                                         laser_geodetic_origin[1],
                                         laser_geodetic_origin[3]));
    m_sensor_trafo->radarCart2Geod(point_cart[0],
                                   point_cart[1],
                                   point_cart[2],
                                   point_geodetic[0],
                                   point_geodetic[1],
                                   point_geodetic[2]);
}

void TstAVDGSToSystemTrafo::convertCartesianFromTrafoToSensor(double origin_cart_system_trafo[],
                                                              double rotation_angle,
                                                              double point_cart_system_trafo[],
                                                              double point_cart_sensor[])
{
    double point_cart_translated[2] = {0.0, 0.0};
    point_cart_translated[0] = point_cart_system_trafo[0] - origin_cart_system_trafo[0];
    point_cart_translated[1] = point_cart_system_trafo[1] - origin_cart_system_trafo[1];

    rotation_angle = AVSciConst::convdegTorad(rotation_angle);
    point_cart_sensor[0] = point_cart_translated[0] * cos(rotation_angle) -
                           point_cart_translated[1] * sin(rotation_angle);
    point_cart_sensor[1] = point_cart_translated[0] * sin(rotation_angle) +
                           point_cart_translated[1] * cos(rotation_angle);
}
///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_xyz2xySystemTrafo()
{
    AVLogInfo << "---- test_xyz2xySystemTrafo launched (" << QTest::currentDataTag() <<") ----";

    // testing group data 1 in quadrant I
    AVLogInfo << "Testing the function with the group data produced in quadrant I "
                 "of the system trafo coordinate system with: Ox positive coordinate, "
                 "Oy negative coordinate";
    // 1. create the Cartesian Coordinates for laser, target, reference w.r.t. the System Trafo

    double laser_cart_system_trafo1[3]        = {0, -800, 0.0};
    double reference_cart_system_trafo1[3]    = {500, -600, 0.0};
    double target_cart_system_trafo1[3]       = {300, -1000, 0.0};
    double laser_geodetic_system_trafo1[3]    = {0.0, 0.0, 0.0};
    double reference_geodetic_system_trafo1[3]= {0.0, 0.0, 0.0};

    // 2. convert the Cartesian to WGS84 coordinates of the laser w.r.t. the System Trafo
    convertToWgs84SystemTrafo(laser_cart_system_trafo1, laser_geodetic_system_trafo1);

    // 3. create the Cartesian coordinates for the laser, target, reference w.r.t the sensor system
    double laser_cart_sensor1[3]     = {0.0, 0.0, 0.0};
    double reference_cart_sensor1[3] = {0.0, 0.0, 0.0};
    double target_cart_sensor1[3]    = {0.0, 0.0, 0.0};
    double rotation_angle_sensor1    = 125.0;
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo1, rotation_angle_sensor1,
                                      laser_cart_system_trafo1, laser_cart_sensor1);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo1, rotation_angle_sensor1,
                                      reference_cart_system_trafo1, reference_cart_sensor1);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo1, rotation_angle_sensor1,
                                      target_cart_system_trafo1, target_cart_sensor1);
    qDebug("laser Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
            laser_cart_sensor1[0], laser_cart_sensor1[1], laser_cart_sensor1[2]);
    qDebug("target Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
            target_cart_sensor1[0], target_cart_sensor1[1], target_cart_sensor1[2]);
    qDebug("reference Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
            reference_cart_sensor1[0], reference_cart_sensor1[1], reference_cart_sensor1[2]);

    // 4. create the WGS84 coordinates for the laser and the reference point for the
    //    corresponding Safegate Cartesian coordinates
    convertToWgs84SystemTrafo(reference_cart_system_trafo1, reference_geodetic_system_trafo1);
    qDebug("reference point WGS84 w.r.t system trafo = %8.6f | %8.6f",
           reference_geodetic_system_trafo1[0], reference_geodetic_system_trafo1[1]);
    convertToWgs84SystemTrafo(laser_cart_system_trafo1, laser_geodetic_system_trafo1);
    qDebug("laser point WGS84 w.r.t. system trafo = %8.6f | %8.6f",
            laser_geodetic_system_trafo1[0], laser_geodetic_system_trafo1[1] );

    // 5. compute the WGS84 coordinates for the Safegate Cartesian coordinates
    AVDGSToSystemTrafo instance_test1(laser_geodetic_system_trafo1[0],
                                      laser_geodetic_system_trafo1[1],
                                      laser_geodetic_system_trafo1[2],
                                      reference_geodetic_system_trafo1[0],
                                      reference_geodetic_system_trafo1[1],
                                      reference_geodetic_system_trafo1[2],
                                      reference_cart_sensor1[0],
                                      reference_cart_sensor1[1],
                                      reference_cart_sensor1[2]);
    double target_geodetic_computed_test1[3] = {0.0, 0.0, 0.0};

    instance_test1.xyz2latlonh(target_cart_sensor1[0],
                               target_cart_sensor1[1],
                               target_cart_sensor1[2],
                               target_geodetic_computed_test1[0],
                               target_geodetic_computed_test1[1],
                               target_geodetic_computed_test1[2]);

    // 6. compute the System Trafo WGS84 coordinates for the System Trafo Cartesian coordinates
    double target_geodetic_expected_test1[3]= {0.0, 0.0, 0.0};
    convertToWgs84SystemTrafo(target_cart_system_trafo1, target_geodetic_expected_test1);

    // 7. compare the results for the WGS84 coordinates of the target point
    double delta = 0.01;

    qDebug("target_lat_expected = %11.9f deg, target_long_expected = %11.9f deg.",
            target_geodetic_expected_test1[0], target_geodetic_expected_test1[1]);
    qDebug("target_lat_computed = %11.9f deg, target_long_computed = %11.9f deg.",
            target_geodetic_computed_test1[0],  target_geodetic_computed_test1[1]);
    AV_DOUBLE_COMPARE(target_geodetic_computed_test1[0], target_geodetic_expected_test1[0], delta);
    AV_DOUBLE_COMPARE(target_geodetic_computed_test1[1], target_geodetic_expected_test1[1], delta);

    // 8. convert the obtained results to the xy System Trafo and Sensor Cartesian Coordinate System
    double point_system_trafo_expected1[2] = {0.0 , 0.0};
    m_system_trafo->latlong2xy(target_geodetic_expected_test1[0],
                               target_geodetic_expected_test1[1],
                               point_system_trafo_expected1[0],
                               point_system_trafo_expected1[1]);
    AVLogInfo << "target Cartesian w.r.t. System Trafo from WGS84 expected = "
              << point_system_trafo_expected1[0] << " | "<<point_system_trafo_expected1[1];

    double point_system_trafo_computed1[2] = {0.0, 0.0};
    m_system_trafo->latlong2xy(target_geodetic_computed_test1[0],
                               target_geodetic_computed_test1[1],
                               point_system_trafo_computed1[0],
                               point_system_trafo_computed1[1]);
    AVLogInfo << "target Cartesian w.r.t System trafo from WGS84 computed = "
              << point_system_trafo_computed1[0] << " | "<< point_system_trafo_computed1[1];

    // 9. convert to System Trafo and compare the results
    double point_sensor_expected1[2] = {0.0, 0.0};
    instance_test1.latlong2xySystemTrafo(target_geodetic_expected_test1[0],
                                         target_geodetic_expected_test1[1],
                                         point_sensor_expected1[0],
                                         point_sensor_expected1[1]);
    AVLogInfo<< "target Cartesian w.r.t. Sensor trafo from WGS84 expected="
                  << point_sensor_expected1[0] <<" | " << point_sensor_expected1[1] <<"|";
    double point_sensor_computed1[2] = {0.0, 0.0};
    instance_test1.latlong2xySystemTrafo(target_geodetic_computed_test1[0],
                                         target_geodetic_computed_test1[1],
                                         point_sensor_computed1[0],
                                         point_sensor_computed1[1]);
    AVLogInfo<< "target Cartesian w.r.t. Sensor trafo from WGS84 computed="
             << point_sensor_computed1[0] <<" | " << point_sensor_computed1[1] <<"|";

    // testing group data 2 in quadrant II
    AVLogInfo << "Testing the function with the group data produced in quadrant II of the "
                 "system trafo coordinate system with: Ox negative coordinate, "
                 "Oy negative coordinate";
    // 1. create the Cartesian Coordinates for laser, target, reference w.r.t. the System Trafo
    double laser_cart_system_trafo2[3] = {0, -800, 0.0};
    double reference_cart_system_trafo2[3] = {-500, -600, 0.0};
    double target_cart_system_trafo2[3] = {-300, -1000, 0.0};

    double laser_geodetic_system_trafo2[3]= {0.0, 0.0, 0.0};
    double reference_geodetic_system_trafo2[3]= {0.0, 0.0, 0.0};

    // 2. convert the Cartesian to WGS84 coordinates of the laser w.r.t. the System Trafo
    convertToWgs84SystemTrafo(laser_cart_system_trafo2, laser_geodetic_system_trafo2);

    // 3. create the Cartesian coordinates for the laser, target, reference w.r.t the sensor system
    double laser_cart_sensor2[3] = {0.0, 0.0, 0.0};
    double reference_cart_sensor2[3] = {0.0, 0.0, 0.0};
    double target_cart_sensor2[3] = {0.0, 0.0, 0.0};
    double rotation_angle_sensor2 = 220.0;
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo2, rotation_angle_sensor2,
                                      laser_cart_system_trafo2, laser_cart_sensor2);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo2, rotation_angle_sensor2,
                                      reference_cart_system_trafo2, reference_cart_sensor2);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo2, rotation_angle_sensor2,
                                      target_cart_system_trafo2, target_cart_sensor2);
    qDebug("laser Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           laser_cart_sensor2[0], laser_cart_sensor2[1], laser_cart_sensor2[2]);
    qDebug("target Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           target_cart_sensor2[0], target_cart_sensor2[1], target_cart_sensor2[2]);
    qDebug("reference Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           reference_cart_sensor2[0], reference_cart_sensor2[1], reference_cart_sensor2[2]);

    // 4. create the WGS84 coordinates for the laser and the reference point for the
    //    corresponding Safegate Cartesian coordinates
    convertToWgs84SystemTrafo(reference_cart_system_trafo2, reference_geodetic_system_trafo2);
    qDebug("reference point WGS84 w.r.t system trafo = %8.6f | %8.6f",
           reference_geodetic_system_trafo2[0], reference_geodetic_system_trafo2[1]);
    convertToWgs84SystemTrafo(laser_cart_system_trafo2, laser_geodetic_system_trafo2);
    qDebug("laser point WGS84 w.r.t. system trafo = %8.6f | %8.6f",
           laser_geodetic_system_trafo2[0], laser_geodetic_system_trafo2[1] );

    // 5. compute the WGS84 coordinates for the Safegate Cartesian coordinates
    AVDGSToSystemTrafo instance_test2(laser_geodetic_system_trafo2[0],
                                      laser_geodetic_system_trafo2[1],
                                      laser_geodetic_system_trafo2[2],
                                      reference_geodetic_system_trafo2[0],
                                      reference_geodetic_system_trafo2[1],
                                      reference_geodetic_system_trafo2[2],
                                      reference_cart_sensor2[0],
                                      reference_cart_sensor2[1],
                                      reference_cart_sensor2[2]);
    double target_geodetic_computed_test2[3] = {0.0, 0.0, 0.0};

    instance_test2.xyz2latlonh(target_cart_sensor2[0],
                               target_cart_sensor2[1],
                               target_cart_sensor2[2],
                               target_geodetic_computed_test2[0],
                               target_geodetic_computed_test2[1],
                               target_geodetic_computed_test2[2]);

    // 6. compute the System Trafo WGS84 coordinates for the System Trafo Cartesian coordinates
    double target_geodetic_expected_test2[3]= {0.0, 0.0, 0.0};
    convertToWgs84SystemTrafo(target_cart_system_trafo2, target_geodetic_expected_test2);

    // 7. compare the results for the WGS84 coordinates of the target point
    AV_DOUBLE_COMPARE(target_geodetic_computed_test2[0], target_geodetic_expected_test2[0], delta);
    AV_DOUBLE_COMPARE(target_geodetic_computed_test2[1], target_geodetic_expected_test2[1], delta);

    qDebug("target_lat_expected = %11.9f deg, target_long_expected = %11.9f deg.",
            target_geodetic_expected_test2[0], target_geodetic_expected_test2[1]);
    qDebug("target_lat_computed = %11.9f deg, target_long_computed = %11.9f deg.",
            target_geodetic_computed_test2[0],  target_geodetic_computed_test2[1]);

    // 8. convert the obtained results to the xy System Trafo and Sensor Cartesian Coordinate System
    double point_system_trafo_expected2[2] = {0.0 , 0.0};
    m_system_trafo->latlong2xy(target_geodetic_expected_test2[0],
                               target_geodetic_expected_test2[1],
                               point_system_trafo_expected2[0],
                               point_system_trafo_expected2[1]);
    AVLogInfo << "target Cartesian w.r.t. System Trafo from WGS84 expected = "
              << point_system_trafo_expected2[0] << " | "<<point_system_trafo_expected2[1];

    double point_system_trafo_computed2[2] = {0.0, 0.0};
    m_system_trafo->latlong2xy(target_geodetic_computed_test2[0],
                               target_geodetic_computed_test2[1],
                               point_system_trafo_computed2[0],
                               point_system_trafo_computed2[1]);
    AVLogInfo << "target Cartesian w.r.t System trafo from WGS84 computed = "
              << point_system_trafo_computed2[0] << " | "<< point_system_trafo_computed2[1];

    // 9. convert to System Trafo and compare the results
    double point_sensor_expected2[2] = {0.0, 0.0};
    instance_test2.latlong2xySystemTrafo(target_geodetic_expected_test2[0],
                                         target_geodetic_expected_test2[1],
                                         point_sensor_expected2[0],
                                         point_sensor_expected2[1]);
    AVLogInfo << "target Cartesian w.r.t. Sensor trafo from WGS84 expected="
              << point_sensor_expected2[0] <<" | " << point_sensor_expected2[1] <<"|";
    double point_sensor_computed2[2] = {0.0, 0.0};
    instance_test2.latlong2xySystemTrafo(target_geodetic_computed_test2[0],
                                         target_geodetic_computed_test2[1],
                                         point_sensor_computed2[0],
                                         point_sensor_computed2[1]);
    AVLogInfo << "target Cartesian w.r.t. Sensor trafo from WGS84 computed="
              << point_sensor_computed2[0] <<" | " << point_sensor_computed2[1] <<"|";

    // testing group data 3 in quadrant III
    AVLogInfo << "Testing the function with the group data produced in quadrant III of the "
                 "system trafo coordinate system with: Ox negative coordinate, "
                 "Oy positive coordinate";
    // 1. create the Cartesian Coordinates for laser, target, reference w.r.t. the System Trafo
    double laser_cart_system_trafo3[3] = {0, 800, 0.0};
    double reference_cart_system_trafo3[3] = {-500, 600, 0.0};
    double target_cart_system_trafo3[3] = {-300, 1000, 0.0};

    double laser_geodetic_system_trafo3[3]= {0.0, 0.0, 0.0};
    double reference_geodetic_system_trafo3[3]= {0.0, 0.0, 0.0};

    // 2. convert the Cartesian to WGS84 coordinates of the laser w.r.t. the System Trafo
    convertToWgs84SystemTrafo(laser_cart_system_trafo3, laser_geodetic_system_trafo3);

    // 3. create the Cartesian coordinates for the laser, target, reference w.r.t the sensor system
    double laser_cart_sensor3[3] = {0.0, 0.0, 0.0};
    double reference_cart_sensor3[3] = {0.0, 0.0, 0.0};
    double target_cart_sensor3[3] = {0.0, 0.0, 0.0};
    double rotation_angle_sensor3 = 300.0;
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo3, rotation_angle_sensor3,
                                      laser_cart_system_trafo3, laser_cart_sensor3);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo3, rotation_angle_sensor3,
                                      reference_cart_system_trafo3, reference_cart_sensor3);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo3, rotation_angle_sensor3,
                                      target_cart_system_trafo3, target_cart_sensor3);
    qDebug("laser Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           laser_cart_sensor3[0], laser_cart_sensor3[1], laser_cart_sensor3[2]);
    qDebug("target Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           target_cart_sensor3[0], target_cart_sensor3[1], target_cart_sensor3[2]);
    qDebug("reference Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           reference_cart_sensor3[0], reference_cart_sensor3[1], reference_cart_sensor3[2]);

    // 4. create the WGS84 coordinates for the laser and the reference point for the
    //    corresponding Safegate Cartesian coordinates
    convertToWgs84SystemTrafo(reference_cart_system_trafo3, reference_geodetic_system_trafo3);
    qDebug("reference point WGS84 w.r.t system trafo = %8.6f | %8.6f",
           reference_geodetic_system_trafo3[0], reference_geodetic_system_trafo3[1]);
    convertToWgs84SystemTrafo(laser_cart_system_trafo3, laser_geodetic_system_trafo3);
    qDebug("laser point WGS84 w.r.t. system trafo = %8.6f | %8.6f",
           laser_geodetic_system_trafo3[0], laser_geodetic_system_trafo3[1] );

    // 5. compute the WGS84 coordinates for the Safegate Cartesian coordinates
    AVDGSToSystemTrafo instance_test3(laser_geodetic_system_trafo3[0],
                                      laser_geodetic_system_trafo3[1],
                                      laser_geodetic_system_trafo3[2],
                                      reference_geodetic_system_trafo3[0],
                                      reference_geodetic_system_trafo3[1],
                                      reference_geodetic_system_trafo3[2],
                                      reference_cart_sensor3[0],
                                      reference_cart_sensor3[1],
                                      reference_cart_sensor3[2]);
    double target_geodetic_computed_test3[3] = {0.0, 0.0, 0.0};

    instance_test3.xyz2latlonh(target_cart_sensor3[0],
                               target_cart_sensor3[1],
                               target_cart_sensor3[2],
                               target_geodetic_computed_test3[0],
                               target_geodetic_computed_test3[1],
                               target_geodetic_computed_test3[2]);

    // 6. compute the System Trafo WGS84 coordinates for the System Trafo Cartesian coordinates
    double target_geodetic_expected_test3[3]= {0.0, 0.0, 0.0};
    convertToWgs84SystemTrafo(target_cart_system_trafo3, target_geodetic_expected_test3);

    // 7. compare the results for the WGS84 coordinates of the target point
    AV_DOUBLE_COMPARE(target_geodetic_computed_test3[0], target_geodetic_expected_test3[0], delta);
    AV_DOUBLE_COMPARE(target_geodetic_computed_test3[1], target_geodetic_expected_test3[1], delta);

    qDebug("target_lat_expected = %11.9f deg, target_long_expected = %11.9f deg.",
            target_geodetic_expected_test3[0], target_geodetic_expected_test3[1]);
    qDebug("target_lat_computed = %11.9f deg, target_long_computed = %11.9f deg.",
            target_geodetic_computed_test3[0],  target_geodetic_computed_test3[1]);

    // 8. convert the obtained results to the xy System Trafo and Sensor Cartesian Coordinate System
    double point_system_trafo_expected3[2] = {0.0 , 0.0};
    m_system_trafo->latlong2xy(target_geodetic_expected_test3[0],
                               target_geodetic_expected_test3[1],
                               point_system_trafo_expected3[0],
                               point_system_trafo_expected3[1]);
    AVLogInfo << "target Cartesian w.r.t. System Trafo from WGS84 expected = "
              << point_system_trafo_expected3[0] << " | "<<point_system_trafo_expected3[1];

    double point_system_trafo_computed3[2] = {0.0, 0.0};
    m_system_trafo->latlong2xy(target_geodetic_computed_test3[0],
                               target_geodetic_computed_test3[1],
                               point_system_trafo_computed3[0],
                               point_system_trafo_computed3[1]);
    AVLogInfo << "target Cartesian w.r.t System trafo from WGS84 computed = "
              << point_system_trafo_computed3[0] << " | "<< point_system_trafo_computed3[1];

    // 9. convert to System Trafo and compare the results
    double point_sensor_expected3[2] = {0.0, 0.0};
    instance_test3.latlong2xySystemTrafo(target_geodetic_expected_test3[0],
                                         target_geodetic_expected_test3[1],
                                         point_sensor_expected3[0],
                                         point_sensor_expected3[1]);
    AVLogInfo << "target Cartesian w.r.t. Sensor trafo from WGS84 expected="
              << point_sensor_expected3[0] <<" | " << point_sensor_expected3[1] <<"|";
    double point_sensor_computed3[2] = {0.0, 0.0};
    instance_test3.latlong2xySystemTrafo(target_geodetic_computed_test3[0],
                                         target_geodetic_computed_test3[1],
                                         point_sensor_computed3[0],
                                         point_sensor_computed3[1]);
    AVLogInfo << "target Cartesian w.r.t. Sensor trafo from WGS84 computed="
              << point_sensor_computed3[0] <<" | " << point_sensor_computed3[1] <<"|";

    // testing group data 4 in quadrant IV
    AVLogInfo << "Testing the function with the group data produced in quadrant IV of the "
                 "system trafo coordinate system with: Ox positive "
                 "coordinate, and Oy positive coordinate";
    // 1. create the Cartesian Coordinates for laser, target, reference w.r.t. the System Trafo
    double laser_cart_system_trafo4[3] = {0, 800, 0.0};
    double reference_cart_system_trafo4[3] = {500, 600, 0.0};
    double target_cart_system_trafo4[3] = {300, 1000, 0.0};

    double laser_geodetic_system_trafo4[3]= {0.0, 0.0, 0.0};
    double reference_geodetic_system_trafo4[3]= {0.0, 0.0, 0.0};

    // 2. convert the Cartesian to WGS84 coordinates of the laser w.r.t. the System Trafo
    convertToWgs84SystemTrafo(laser_cart_system_trafo4, laser_geodetic_system_trafo4);

    // 3. create the Cartesian coordinates for the laser, target, reference w.r.t the sensor system
    double laser_cart_sensor4[3] = {0.0, 0.0, 0.0};
    double reference_cart_sensor4[3] = {0.0, 0.0, 0.0};
    double target_cart_sensor4[3] = {0.0, 0.0, 0.0};
    double rotation_angle_sensor4 = 45.0;
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo4, rotation_angle_sensor4,
                                      laser_cart_system_trafo4, laser_cart_sensor4);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo4, rotation_angle_sensor4,
                                      reference_cart_system_trafo4, reference_cart_sensor4);
    convertCartesianFromTrafoToSensor(laser_cart_system_trafo4, rotation_angle_sensor4,
                                      target_cart_system_trafo4, target_cart_sensor4);
    qDebug("laser Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
            laser_cart_sensor4[0], laser_cart_sensor4[1], laser_cart_sensor4[2]);
    qDebug("target Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
            target_cart_sensor4[0], target_cart_sensor4[1], target_cart_sensor4[2]);
    qDebug("reference Cartesian w.r.t sensor system = %8.6f | %8.6f | %8.6f",
           reference_cart_sensor4[0], reference_cart_sensor4[1], reference_cart_sensor4[2]);

    // 4. create the WGS84 coordinates for the laser and the reference point for the
    //    corresponding Safegate Cartesian coordinates
    convertToWgs84SystemTrafo(reference_cart_system_trafo4, reference_geodetic_system_trafo4);
    qDebug("reference point WGS84 w.r.t system trafo = %8.6f | %8.6f",
           reference_geodetic_system_trafo4[0], reference_geodetic_system_trafo4[1]);
    convertToWgs84SystemTrafo(laser_cart_system_trafo4, laser_geodetic_system_trafo4);
    qDebug("laser point WGS84 w.r.t. system trafo = %8.6f | %8.6f",
            laser_geodetic_system_trafo4[0], laser_geodetic_system_trafo4[1] );

    // 5. compute the WGS84 coordinates for the Safegate Cartesian coordinates
    AVDGSToSystemTrafo instance_test4(laser_geodetic_system_trafo4[0],
                                      laser_geodetic_system_trafo4[1],
                                      laser_geodetic_system_trafo4[2],
                                      reference_geodetic_system_trafo4[0],
                                      reference_geodetic_system_trafo4[1],
                                      reference_geodetic_system_trafo4[2],
                                      reference_cart_sensor4[0],
                                      reference_cart_sensor4[1],
                                      reference_cart_sensor4[2]);
    double target_geodetic_computed_test4[3] = {0.0, 0.0, 0.0};

    instance_test4.xyz2latlonh(target_cart_sensor4[0],
                               target_cart_sensor4[1],
                               target_cart_sensor4[2],
                               target_geodetic_computed_test4[0],
                               target_geodetic_computed_test4[1],
                               target_geodetic_computed_test4[2]);

    // 6. compute the System Trafo WGS84 coordinates for the System Trafo Cartesian coordinates
    double target_geodetic_expected_test4[3]= {0.0, 0.0, 0.0};
    convertToWgs84SystemTrafo(target_cart_system_trafo4, target_geodetic_expected_test4);

    // 7. compare the results for the WGS84 coordinates of the target point
    AV_DOUBLE_COMPARE(target_geodetic_computed_test4[0], target_geodetic_expected_test4[0], delta);
    AV_DOUBLE_COMPARE(target_geodetic_computed_test4[1], target_geodetic_expected_test4[1], delta);

    qDebug("target_lat_expected = %11.9f deg, target_long_expected = %11.9f deg.",
            target_geodetic_expected_test4[0], target_geodetic_expected_test4[1]);
    qDebug("target_lat_computed = %11.9f deg, target_long_computed = %11.9f deg.",
            target_geodetic_computed_test4[0],  target_geodetic_computed_test4[1]);

    // 8. convert the obtained results to the xy System Trafo and Sensor Cartesian Coordinate System
    double point_system_trafo_expected4[2] = {0.0 , 0.0};
    m_system_trafo->latlong2xy(target_geodetic_expected_test4[0],
                               target_geodetic_expected_test4[1],
                               point_system_trafo_expected4[0],
                               point_system_trafo_expected4[1]);
    AVLogInfo << "target Cartesian w.r.t. System Trafo from WGS84 expected = "
              << point_system_trafo_expected4[0] << " | "<<point_system_trafo_expected4[1];

    double point_system_trafo_computed4[2] = {0.0, 0.0};
    m_system_trafo->latlong2xy(target_geodetic_computed_test4[0],
                               target_geodetic_computed_test4[1],
                               point_system_trafo_computed4[0],
                               point_system_trafo_computed4[1]);
    AVLogInfo << "target Cartesian w.r.t System trafo from WGS84 computed = "
              << point_system_trafo_computed4[0] << " | "<< point_system_trafo_computed4[1];

    // 9. convert to System Trafo and compare the results
    double point_sensor_expected4[2] = {0.0, 0.0};
    instance_test4.latlong2xySystemTrafo(target_geodetic_expected_test4[0],
                                         target_geodetic_expected_test4[1],
                                         point_sensor_expected4[0],
                                         point_sensor_expected4[1]);
    AVLogInfo<< "target Cartesian w.r.t. Sensor trafo from WGS84 expected="
                  << point_sensor_expected4[0] <<" | " << point_sensor_expected4[1] <<"|";
    double point_sensor_computed4[2] = {0.0, 0.0};
    instance_test4.latlong2xySystemTrafo(target_geodetic_computed_test4[0],
                                         target_geodetic_computed_test4[1],
                                         point_sensor_computed4[0],
                                         point_sensor_computed4[1]);
    AVLogInfo<< "target Cartesian w.r.t. Sensor trafo from WGS84 computed="
             << point_sensor_computed4[0] <<" | " << point_sensor_computed4[1] <<"|";

    AVLogInfo << "---- test_xyz2xySystemTrafo ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_latlong2xySystemTrafo()
{
    AVLogInfo << "---- test_latlong2xySystemTrafo launched (" << QTest::currentDataTag() <<") ----";

    double laser_lat        = 47.492804;
    double laser_long       = 14.000000;
    double laser_height     = 0.0;
    double reference_lat    = 47.494603 ;
    double reference_long   = 14.006636;
    double reference_height = 0.0;
    double reference_x      = 494.974747;
    double reference_y      = 212.132034;
    double reference_z      = 0.0;

    AVDGSToSystemTrafo instance(laser_lat, laser_long, laser_height,
                                reference_lat, reference_long, reference_height,
                                reference_x, reference_y, reference_z);

    double target_x = 70.710678;
    double target_y = 353.553391;

    double target_lat;
    double target_lon;
    double target_h;
    double target_x_av;
    double target_y_av;

    double expected_target_lat = 47.493005;
    double expected_target_lon = 14.0009;
    double delta = 0.001;

    instance.xyz2latlonh(target_x,target_y,0.0, target_lat,target_lon, target_h);

    AV_DOUBLE_COMPARE(target_lat, expected_target_lat, delta);
    AV_DOUBLE_COMPARE(target_lon, expected_target_lon, delta);

    AVLogInfo<<" target_lat = "<<target_lat<< " target_lon = "<< target_lon <<
               " target_x_av = "<<target_x_av<<" target_y_av = "<<target_y_av;
    instance.latlong2xySystemTrafo(target_lat, target_lon, target_x_av, target_y_av);

    AVLogInfo << "---- test_latlong2xySystemTrafo ended (" << QTest::currentDataTag() <<")    ----";
}
///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_getRotationAngleFromOrdinate()
{
    AVLogInfo << "---- test_getRotationAngleFromOrdinate launched (" << QTest::currentDataTag() <<") ----";

    double laser_lat        = 47.492804;
    double laser_long       = 14.000000;
    double laser_height     = 0.0;
    double reference_lat    = 47.494603 ;
    double reference_long   = 14.006636;
    double reference_height = 0.0;
    double reference_x      = 494.974747;
    double reference_y      = 212.132034;
    double reference_z      = 0.0;

    AVDGSToSystemTrafo instance(laser_lat, laser_long, laser_height,
                                           reference_lat, reference_long, reference_height,
                                           reference_x, reference_y, reference_z);

    double returned_result = instance.getRotationAngleFromOrdinate();
    double expected_value  = 3.14159;
    double delta           = 0.00001;

    AVLogInfo<<"computed_value = "<<returned_result<<" expected_value = "<<expected_value;
    AV_DOUBLE_COMPARE(returned_result, expected_value, delta);

    AVLogInfo << "---- test_getRotationAngleFromOrdinate ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_getRotationAngleFromNorth()
{
    AVLogInfo << "---- test_getRotationAngleFromNorth launched (" << QTest::currentDataTag() <<") ----";

    double laser_lat        = 47.492804;
    double laser_long       = 14.000000;
    double laser_height     = 0.0;
    double reference_lat    = 47.494603 ;
    double reference_long   = 14.006636;
    double reference_height = 0.0;
    double reference_x      = 494.974747;
    double reference_y      = 212.132034;
    double reference_z      = 0.0;

    AVDGSToSystemTrafo instance(laser_lat, laser_long, laser_height,
                                           reference_lat, reference_long, reference_height,
                                           reference_x, reference_y, reference_z);

    double returned_result = instance.getRotationAngleFromNorth();
    double expected_value  = 1.18919;
    double delta           = 0.000001;

    AVLogInfo<<"actual value ="<<returned_result<<"expected value ="<<expected_value;
    AV_DOUBLE_COMPARE(returned_result, expected_value, delta);

    AVLogInfo << "---- test_getRotationAngleFromNorth ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

void TstAVDGSToSystemTrafo::test_getRotatedCoordinates()
{
    AVLogInfo << "---- test_getRotatedCoordinates launched (" << QTest::currentDataTag() <<") ----";

    double laser_lat        = 47.492804;
    double laser_long       = 14.000000;
    double laser_height     = 0.0;
    double reference_lat    = 47.494603 ;
    double reference_long   = 14.006636;
    double reference_height = 0.0;
    double reference_x      = 494.974747;
    double reference_y      = 212.132034;
    double reference_z      = 0.0;

    AVDGSToSystemTrafo instance(laser_lat, laser_long, laser_height,
                                reference_lat, reference_long, reference_height,
                                reference_x, reference_y, reference_z);

    double rotation_angle = 1.97569 - 1.18919;
    double x = 70.7107;
    double y = -353.553;
    double x_rotated;
    double y_rotated;

    instance.getRotatedCoordinates(rotation_angle, x, y, x_rotated, y_rotated);

    double expected_value_x = 300.219924;
    double expected_value_y = -199.669036;
    double delta            = 0.000001;

    AVLogInfo<< "x_rotated = "<<x_rotated<<" expected_value_x = "<<expected_value_x;
    AVLogInfo<< "y_rotated = "<<y_rotated<<" expected_value_y = "<<expected_value_y;
    AV_DOUBLE_COMPARE(x_rotated, expected_value_x, delta);
    AV_DOUBLE_COMPARE(y_rotated, expected_value_y, delta);

    AVLogInfo << "---- test_getRotatedCoordinates ended (" << QTest::currentDataTag() <<")    ----";
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(TstAVDGSToSystemTrafo,"avcalculationlib/unittests/config")
#include "tstavdgstosystemtrafo.moc"

// End of file
