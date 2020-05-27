///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2010
//
// Module:     TESTS - AVCOMMON Unit Tests
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
 \author  Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
 \brief
 */


// system includes
#include <cmath>
#include <QTest>
#include <avunittestmain.h>

// local includes
#include <avplanarcalculator.h>



///////////////////////////////////////////////////////////////////////////////

class AVPlanarCalculatorTest: public QObject
{
    Q_OBJECT

private:

private slots:

    void init()
    {
    }

    void cleanup()
    {
    }

    void testTwoLineSegmentsIntersect()
    {
        AVLogInfo <<  "----testTwoLineSegmentsIntersect launched----";

        //crossing
        QPoint A1(0, 0);
        QPoint A2(10, 2);
        QPoint B1(1, 0);
        QPoint B2(0, 1);

        QVERIFY(AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        //not crossing
        B2 = QPoint(10, 0);
        QVERIFY(!AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        //touching
        B2 = QPoint(10, 2);
        QVERIFY(AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        //parallel
        B1 = QPoint(5, 0);
        B2 = QPoint(15, 2);
        QVERIFY(!AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        //parallel, in-line
        B1 = QPoint(20, 4);
        B2 = QPoint(30, 6);
        QVERIFY(!AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        //parallel touching
        B1 = QPoint(10, 2);
        B2 = QPoint(30, 6);
        QVERIFY(AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        //parallel crossing
        B1 = QPoint(5, 1);
        B2 = QPoint(20, 4);
        QVERIFY(AVPlanarCalculator::twoSegsHaveIntersection(A1, A2, B1, B2));

        AVLogInfo <<  "----testTwoLineSegmentsIntersect finished----";
    }

    void testVectorHeading()
    {
        AVLogInfo <<  "----testVectorHeading launched----";

        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(0.0, -1.0)),0.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(1.0, -1.0)),45.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(1.0, 0.0)),90.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(1.0, 1.0)),135.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(0.0, 1.0)),180.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(-1.0, 1.0)),225.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(-1.0, 0.0)),270.0);
        QCOMPARE(AVPlanarCalculator::getVectorHeading(QPointF(-1.0, -1.0)),315.0);

        AVLogInfo <<  "----testVectorHeading finished----";
    }

    void testUnitVectorForHeading()
    {
        AVLogInfo <<  "----testUnitVectorForHeading launched----";

        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(0.0),   QPointF(0.0, -1.0));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(45.0),  QPointF(1.0, -1.0)/sqrt(2.));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(90.0),  QPointF(1.0, 0.0));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(135.0), QPointF(1.0, 1.0)/sqrt(2.));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(180.0), QPointF(0.0, 1.0));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(225.0), QPointF(-1.0, 1.0)/sqrt(2.));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(270.0), QPointF(-1.0, 0.0));
        QCOMPARE(AVPlanarCalculator::getUnitVectorForHeading(315.0), QPointF(-1.0, -1.0)/sqrt(2.));

        AVLogInfo <<  "----testUnitVectorForHeading finished----";
    }

    void testAngleDifference()
    {
        AVLogInfo <<  "----testAngleDifference launched----";

        QCOMPARE(AVPlanarCalculator::getAngleDifference(143.3, 143.3),0.0);

        QVERIFY(fabs(56.7-AVPlanarCalculator::getAngleDifference(143.3, 200))<1E-10);
        QVERIFY(fabs(6.1- AVPlanarCalculator::getAngleDifference(1.1, 355))<1E-10);
        QVERIFY(fabs(15.5- AVPlanarCalculator::getAngleDifference(10.5, 355))<1E-10);
        QVERIFY(fabs(6.1- AVPlanarCalculator::getAngleDifference(355, 1.1))<1E-10);
        QVERIFY(fabs(15.5- AVPlanarCalculator::getAngleDifference(355, 10.5))<1E-10);
        QVERIFY(fabs(0.0- AVPlanarCalculator::getAngleDifference(360, 0))<1E-10);
        QVERIFY(fabs(180.0- AVPlanarCalculator::getAngleDifference(270, 90))<1E-10);

        AVLogInfo <<  "----testAngleDifference finished----";
    }

    void testDistancePointToSegment()
    {
        AVLogInfo <<  "----testDistancePointToSegment launched----";

        QPointF p1(0, 0);
        QPointF p2(1, 1);

        QVERIFY(fabs(
                0.0-
                AVPlanarCalculator::getMinDistanceOfPointToSegment(QPointF(0, 0),p1,p2))<1E-10);
        QVERIFY(fabs(
                sqrt(2.0)/2-
                AVPlanarCalculator::getMinDistanceOfPointToSegment(QPointF(0, 1),p1,p2))<1E-10);
        QVERIFY(fabs(
                0.0-
                AVPlanarCalculator::getMinDistanceOfPointToSegment(QPointF(1, 1),p1,p2))<1E-10);
        QVERIFY(fabs(
                1.0-
                AVPlanarCalculator::getMinDistanceOfPointToSegment(QPointF(-1, 0),p1,p2))<1E-10);
        QVERIFY(fabs(
                1.0-
                AVPlanarCalculator::getMinDistanceOfPointToSegment(QPointF(1, 2),p1,p2))<1E-10);
        QVERIFY(fabs(
                1.0/sqrt(8.0)-
                AVPlanarCalculator::getMinDistanceOfPointToSegment(QPointF(0.5, 0),p1,p2))<1E-10);


        AVLogInfo <<  "----testDistancePointToSegment finished----";
    }

    void testContains1()
    {
        AVLogInfo <<  "----testContains1 launched----";

        // case contains(const QPolygon &points, const QPoint &p)
        QPolygon polygon(4);
        polygon.setPoint(0, QPoint( 2, 2));
        polygon.setPoint(1, QPoint(-2, 2));
        polygon.setPoint(2, QPoint(-2,-2));
        polygon.setPoint(3, QPoint( 2,-2));

        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint( 0, 0)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint( 1, 1)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint(-1, 1)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint(-1,-1)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint( 1,-1)));

        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint( 3, 3)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint(-3, 3)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint(-3,-3)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint( 3,-3)));

        AVLogInfo <<  "----testContains1 finished----";
    }

    void testContains2()
    {
        AVLogInfo <<  "----testContains2 launched----";

        // case contains(const QVector<QPoint> &polygon, const QPoint &p)
        QVector<QPoint> polygon(4);
        polygon[0] = QPoint( 2, 2);
        polygon[1] = QPoint(-2, 2);
        polygon[2] = QPoint(-2,-2);
        polygon[3] = QPoint( 2,-2);

        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint( 0, 0)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint( 1, 1)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint(-1, 1)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint(-1,-1)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPoint( 1,-1)));

        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint( 3, 3)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint(-3, 3)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint(-3,-3)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPoint( 3,-3)));

        AVLogInfo <<  "----testContains2 finished----";
    }

    void testContains3()
    {
        AVLogInfo <<  "----testContains3 launched----";

        // case contains(const QVector<QPointF> &polygon, const QPointF &p)
        QVector<QPointF> polygon(4);
        polygon[0] = QPointF( 2.0, 2.0);
        polygon[1] = QPointF(-2.0, 2.0);
        polygon[2] = QPointF(-2.0,-2.0);
        polygon[3] = QPointF( 2.0,-2.0);

        QVERIFY(AVPlanarCalculator::contains(polygon, QPointF( 0.0, 0.0)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPointF( 1.0, 1.0)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPointF(-1.0, 1.0)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPointF(-1.0,-1.0)));
        QVERIFY(AVPlanarCalculator::contains(polygon, QPointF( 1.0,-1.0)));

        QVERIFY(!AVPlanarCalculator::contains(polygon, QPointF( 3.0, 3.0)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPointF(-3.0, 3.0)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPointF(-3.0,-3.0)));
        QVERIFY(!AVPlanarCalculator::contains(polygon, QPointF( 3.0,-3.0)));

        AVLogInfo <<  "----testContains3 finished----";
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVPlanarCalculatorTest,"avcalculationlib/unittests/config")
#include "avplanarcalculatortest.moc"

// End of file
