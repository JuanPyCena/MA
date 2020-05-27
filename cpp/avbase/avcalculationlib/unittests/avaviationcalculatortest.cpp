///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////
/*! \file
    \author  DI Alexander Randeu, a.randeu@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Test the AVAViationCalculator class
*/





// Qt includes
#include <QString>
#include <QTest>
#include <avunittestmain.h>
#include <QPointF>


// AVLib includes
#include "avaviationcalculator.h"

///////////////////////////////////////////////////////////////////////////////

class AVAviationCalculatorTest : public QObject
{
    Q_OBJECT

public:

    AVAviationCalculatorTest() : m_double_eps(0.0) {}

private slots:

    void init()
    {
        m_double_eps = 1e-10;
    }

    void cleanup()
    {
    }

    void testNormalDistancePointToLine()
    {
        // normalDistancePointToLine

        // vertical line
        // point inside line segment
        double linex1 = 3.0;
        double liney1 = 1.0;
        double linex2 = 3.0;
        double liney2 = 5.0;
        double pointx = 2.0;
        double pointy = 4.0;

        double dist = AVPlanarCalculator::normalDistancePointToLine(pointx, pointy,
                                                                    linex1, liney1,
                                                                    linex2, liney2);
        QVERIFY2(1.0 == dist, "vertical line/point inside line segment");

        // point outside line segment
        pointy = 8.0;
        dist = AVPlanarCalculator::normalDistancePointToLine(pointx, pointy,
                                                             linex1, liney1,
                                                             linex2, liney2);
        QVERIFY2(1.0 == dist, "vertical line/point outside line segment");

        // horizontal line
        // point inside line segment
        linex1 = 3.0;
        liney1 = 1.0;
        linex2 = 5.0;
        liney2 = 1.0;
        pointx = 4.0;
        pointy = 5.0;

        dist = AVPlanarCalculator::normalDistancePointToLine(pointx, pointy,
                                                             linex1, liney1,
                                                             linex2, liney2);
        QVERIFY2(4.0 == dist, "horizontal line/point inside line segment");

        // point outside line segment
        pointx = 6.0;
        dist = AVPlanarCalculator::normalDistancePointToLine(pointx, pointy,
                                                             linex1, liney1,
                                                             linex2, liney2);
        QVERIFY2(4.0 == dist, "horizontal line/point outside line segment");

        // general line
        // point inside line segment
        linex1 = 1.0;
        liney1 = 1.0;
        linex2 = 4.0;
        liney2 = 3.0;
        pointx = 4.0;
        pointy = 1.0;

        dist = AVPlanarCalculator::normalDistancePointToLine(pointx, pointy,
                                                             linex1, liney1,
                                                             linex2, liney2);
        double alpha = atan((liney2-liney1)/(linex2-linex1));
        AV_DOUBLE_COMPARE(dist, sin(alpha)*(pointx-linex1), m_double_eps);

        // point outside line segment
        pointx = 7.0;
        alpha = atan((liney2-liney1)/(linex2-linex1));
        dist = AVPlanarCalculator::normalDistancePointToLine(pointx, pointy,
                                                             linex1, liney1,
                                                             linex2, liney2);
        AV_DOUBLE_COMPARE(dist, sin(alpha)*(pointx-linex1), m_double_eps);
    }

    void testIntersectLineCircle()
    {
        // vertical line

        // no intersection
        double linex1 = 3.0;
        double liney1 = 1.0;
        double linex2 = 3.0;
        double liney2 = 5.0;
        double centerx = 7.0;
        double centery = 4.0;
        double radius = 3.0;
        double intersectx1;
        double intersecty1;
        double intersectx2;
        double intersecty2;
        bool res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                           linex2, liney2,
                                                           centerx, centery, radius,
                                                           intersectx1, intersecty1,
                                                           intersectx2, intersecty2);
        QVERIFY2( res == false, "vertical line/circle no intersection");


        // 2 intersections
        radius = 5.0;
        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == true, "vertical line/circle 2 intersections (a)");
        AV_DOUBLE_COMPARE(linex1, intersectx1, m_double_eps);
        AV_DOUBLE_COMPARE(linex1, intersectx2, m_double_eps);
        double c = sqrt(radius*radius - 4.0*4.0);
        AV_DOUBLE_COMPARE(centery + c, intersecty1, m_double_eps);
        AV_DOUBLE_COMPARE(centery - c, intersecty2, m_double_eps);

        // 1 tangential intersection
        radius = 4.0;

        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == true, "vertical line/circle 1 intersection (a)");
        AV_DOUBLE_COMPARE(linex1, intersectx1, m_double_eps);
        AV_DOUBLE_COMPARE(linex1, intersectx2, m_double_eps);
        AV_DOUBLE_COMPARE(centery, intersecty1, m_double_eps);
        AV_DOUBLE_COMPARE(centery, intersecty2, m_double_eps);

        // horizontal line
        // no intersection
        linex1 = 1.0;
        liney1 = 1.0;
        linex2 = 5.0;
        liney2 = 1.0;
        centerx = 7.0;
        centery = 5.0;
        radius = 3.0;
        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == false, "horizontal line/circle no intersection");


        // 2 intersections
        radius = 5.0;
        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == true, "horizontal line/circle 2 intersections (a)");
        AV_DOUBLE_COMPARE(liney1, intersecty1, m_double_eps);
        AV_DOUBLE_COMPARE(liney1, intersecty2, m_double_eps);
        c = sqrt(radius*radius - 4.0*4.0);
        AV_DOUBLE_COMPARE(centerx + c, intersectx1, m_double_eps);
        AV_DOUBLE_COMPARE(centerx - c, intersectx2, m_double_eps);

        // 1 tangential intersection
        radius = 4.0;

        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == true, "horizontal line/circle 1 intersection (a)");
        AV_DOUBLE_COMPARE(liney1, intersecty1, m_double_eps);
        AV_DOUBLE_COMPARE(liney1, intersecty2, m_double_eps);
        AV_DOUBLE_COMPARE(centerx, intersectx1, m_double_eps);
        AV_DOUBLE_COMPARE(centerx, intersectx2, m_double_eps);

        // general line
        // no intersection
        linex1 = 1.0;
        liney1 = 1.0;
        linex2 = 5.0;
        liney2 = 5.0;
        centerx = 5.0;
        centery = 0.0;
        radius = 2.0;
        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == false, "general line/circle no intersection");


        // 2 intersections
        radius = 5.0;
        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == true, "general line/circle 2 intersections (a)");
        AV_DOUBLE_COMPARE(5.0, intersectx1, m_double_eps);
        AV_DOUBLE_COMPARE(0.0, intersectx2, m_double_eps);
        c = sqrt(radius*radius - 4.0*4.0);
        AV_DOUBLE_COMPARE(5.0, intersecty1, m_double_eps);
        AV_DOUBLE_COMPARE(0.0, intersecty2, m_double_eps);

        // 1 tangential intersection
        radius = 5.0/sqrt(2.0) + 5e-16;

        res = AVPlanarCalculator::intersectLineCircle(linex1, liney1,
                                                      linex2, liney2,
                                                      centerx, centery, radius,
                                                      intersectx1, intersecty1,
                                                      intersectx2, intersecty2);
        QVERIFY2( res == true, "general line/circle 1 intersection (a)");
        AV_DOUBLE_COMPARE(2.5, intersectx1, 1e-6);
        AV_DOUBLE_COMPARE(2.5, intersectx2, 1e-6);
        AV_DOUBLE_COMPARE(2.5, intersecty1, 1e-6);
        AV_DOUBLE_COMPARE(2.5, intersecty2, 1e-6);
    }

    void testIntersectionHeading() {
        double p1_lat    = 47.0;
        double p1_lon    = 16.0;
        double c12       = 45.0;
        double p3_lat    = 47.5;
        double p3_lon    = 16.0;
        double p4_lat    = 47.0;
        double p4_lon    = 16.5;
        double deviation = 0.0;
        QVERIFY(AVAviationCalculator::intersectionHeading(p1_lat, p1_lon,
                                                          c12,
                                                          p3_lat, p3_lon,
                                                          p4_lat, p4_lon,
                                                          deviation));
        AV_DOUBLE_COMPARE(100.64747, deviation, 1e-5);

        p1_lat    = 0.0;
        p1_lon    = 0.0;
        c12       = 0.0;
        p3_lat    = 17.5;
        p3_lon    = -1.0;
        p4_lat    = 17.5;
        p4_lon    = 1.0;
        QVERIFY(AVAviationCalculator::intersectionHeading(p1_lat, p1_lon,
                                                          c12,
                                                          p3_lat, p3_lon,
                                                          p4_lat, p4_lon,
                                                          deviation));
        AV_DOUBLE_COMPARE(89.69927, deviation, 1e-5);

        p1_lat    = 47.0;
        p1_lon    = 16.0;
        c12       = 225.0;
        p3_lat    = 47.5;
        p3_lon    = 16.0;
        p4_lat    = 47.0;
        p4_lon    = 16.5;
        QVERIFY(!AVAviationCalculator::intersectionHeading(p1_lat, p1_lon,
                                                           c12,
                                                           p3_lat, p3_lon,
                                                           p4_lat, p4_lon,
                                                           deviation));

        p1_lat    = 47.0;
        p1_lon    = 16.0;
        c12       = 90.0;
        p3_lat    = 47.5;
        p3_lon    = 16.0;
        p4_lat    = 47.5;
        p4_lon    = 16.5;
        QVERIFY(!AVAviationCalculator::intersectionHeading(p1_lat, p1_lon,
                                                           c12,
                                                           p3_lat, p3_lon,
                                                           p4_lat, p4_lon,
                                                           deviation));
    }

    void testIntersection2SegmentsSpherical() {
        double p1_lat    = 47.0;
        double p1_lon    = 16.0;
        double p2_lat    = 47.5;
        double p2_lon    = 16.5;
        double p3_lat    = 47.5;
        double p3_lon    = 16.0;
        double p4_lat    = 47.0;
        double p4_lon    = 16.5;

        QVERIFY(AVAviationCalculator::intersection2SegmentsSpherical(p1_lat, p1_lon,
                                                                     p2_lat, p2_lon,
                                                                     p3_lat, p3_lon,
                                                                     p4_lat, p4_lon));
        p1_lat    = 0.0;
        p1_lon    = 0.0;
        p2_lat    = 10.0;
        p2_lon    = 0.0;
        p3_lat    = 5.0;
        p3_lon    = -1.0;
        p4_lat    = 5.0;
        p4_lon    = 1.0;

        QVERIFY(AVAviationCalculator::intersection2SegmentsSpherical(p1_lat, p1_lon,
                                                                     p2_lat, p2_lon,
                                                                     p3_lat, p3_lon,
                                                                     p4_lat, p4_lon));

        p1_lat    = 47.0;
        p1_lon    = 16.0;
        p2_lat    = 47.0;
        p2_lon    = 16.5;
        p3_lat    = 47.5;
        p3_lon    = 16.0;
        p4_lat    = 47.5;
        p4_lon    = 16.5;

        QVERIFY(!AVAviationCalculator::intersection2SegmentsSpherical(p1_lat, p1_lon,
                                                                      p2_lat, p2_lon,
                                                                      p3_lat, p3_lon,
                                                                      p4_lat, p4_lon));
    }

    void testIntersection2SegmentsSpherical1() {
        double p1_lat    = 47.0;
        double p1_lon    = 16.0;
        double p2_lat    = 47.5;
        double p2_lon    = 16.5;
        double p3_lat    = 47.5;
        double p3_lon    = 16.0;
        double p4_lat    = 47.0;
        double p4_lon    = 16.5;
        double p_int_lat, p_int_lon;

        QVERIFY(AVAviationCalculator::intersection2SegmentsSpherical(p1_lat, p1_lon,
                                                                     p2_lat, p2_lon,
                                                                     p3_lat, p3_lon,
                                                                     p4_lat, p4_lon,
                                                                     p_int_lat, p_int_lon));
        AV_DOUBLE_COMPARE(47.25145, p_int_lat, 1e-5);
        AV_DOUBLE_COMPARE(16.25000, p_int_lon, 1e-5);

        p1_lat    = 1.0;
        p1_lon    = 1.0;
        p2_lat    = 10.0;
        p2_lon    = 1.0;
        p3_lat    = 5.0;
        p3_lon    = -10.0;
        p4_lat    = 5.0;
        p4_lon    = 10.0;

        QVERIFY(AVAviationCalculator::intersection2SegmentsSpherical(p1_lat, p1_lon,
                                                                     p2_lat, p2_lon,
                                                                     p3_lat, p3_lon,
                                                                     p4_lat, p4_lon,
                                                                     p_int_lat, p_int_lon));
        AV_DOUBLE_COMPARE(5.07596, p_int_lat, 1e-5);
        AV_DOUBLE_COMPARE(1.00000, p_int_lon, 1e-5);

        p1_lat    = 47.0;
        p1_lon    = 16.0;
        p2_lat    = 47.0;
        p2_lon    = 16.5;
        p3_lat    = 47.5;
        p3_lon    = 16.0;
        p4_lat    = 47.5;
        p4_lon    = 16.5;

        QVERIFY(!AVAviationCalculator::intersection2SegmentsSpherical(p1_lat, p1_lon,
                                                                      p2_lat, p2_lon,
                                                                      p3_lat, p3_lon,
                                                                      p4_lat, p4_lon,
                                                                      p_int_lat, p_int_lon));
    }

    void testIntersectionLineWithLine()
    {
        QPointF s11;
        QPointF s12;
        QPointF l11;
        QPointF l12;
        QPointF intersection_point;
        double eps = 1e-20;

        // case point and line
        s11=QPointF(0.0, 0.0);
        s12=QPointF(0.0, 0.0);
        l11=QPointF(2.0, 0.0);
        l12=QPointF(2.0, 2.0);

        bool intersect = AVPlanarCalculator::intersectLineWithLine(s11, s12, l11, l12,
                                                                   &intersection_point);
        QVERIFY(intersect == false);

        // case parallel
        s11=QPointF(0.0, 0.0);
        s12=QPointF(0.0, 2.0);
        l11=QPointF(2.0, 0.0);
        l12=QPointF(2.0, 2.0);

        intersect = AVPlanarCalculator::intersectLineWithLine(s11, s12, l11, l12,
                                                              &intersection_point);
        QVERIFY(intersect == false);

        // case 1 line horizontal
        s11=QPointF(0.0, 0.0);
        s12=QPointF(2.0, 0.0);
        l11=QPointF(2.0, 0.0);
        l12=QPointF(3.0, 3.0);

        intersect = AVPlanarCalculator::intersectLineWithLine(s11, s12, l11, l12,
                                                              &intersection_point);
        QVERIFY(intersect == true);
        AV_DOUBLE_COMPARE(2.0, intersection_point.x(), eps);
        AV_DOUBLE_COMPARE(0.0, intersection_point.y(), eps);

        // case 1 line vertical
        s11=QPointF(1.0, 1.0);
        s12=QPointF(2.0, 0.0);
        l11=QPointF(2.0, 0.0);
        l12=QPointF(2.0, 2.0);

        intersect = AVPlanarCalculator::intersectLineWithLine(s11, s12, l11, l12,
                                                              &intersection_point);
        QVERIFY(intersect == true);
        AV_DOUBLE_COMPARE(2.0, intersection_point.x(), eps);
        AV_DOUBLE_COMPARE(0.0, intersection_point.y(), eps);

        // case general
        s11=QPointF(0.0, 0.0);
        s12=QPointF(4.0, 4.0);
        l11=QPointF(1.0, 4.0);
        l12=QPointF(3.0, 0.0);

        intersect = AVPlanarCalculator::intersectLineWithLine(s11, s12, l11, l12,
                                                              &intersection_point);
        QVERIFY(intersect == true);
        AV_DOUBLE_COMPARE(2.0, intersection_point.x(), eps);
        AV_DOUBLE_COMPARE(2.0, intersection_point.y(), eps);
    }

protected:

    double m_double_eps;
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVAviationCalculatorTest,"avcalculationlib/unittests/config")
#include "avaviationcalculatortest.moc"


// End of file
