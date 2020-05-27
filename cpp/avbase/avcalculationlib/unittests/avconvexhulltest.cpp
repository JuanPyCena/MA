///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  DI Ulrich Feichter, u.feichter@avibit.com
  \author  QT4-PORT: Ulrich Feichter, u.feichter@avibit.com
  \brief
*/

#include <qprocess.h>
#include <QTest>
#include <avunittestmain.h>
#include <QVector>

#include "avconvexhull.h"

/////////////////////////////////////////////////////////////////////////////

class AVConvexHullTest : public QObject
{
    Q_OBJECT

private slots:

    void init()
    { }

    //----------------------------------------

    void tst_sortFunction()
    {
        qDebug("start %s", __FUNCTION__);

        // QPoint
        QVector<QPoint> points = getPoints1();
        qSort(points.begin(), points.end(), AVConvexHull<QPoint>::sortFunction);

        QVector<QPoint> points_verif = getPoints1();
        QVERIFY(points.count() == points_verif.count());
        QCOMPARE(points.count(),7);

        QVERIFY(points[0] == points_verif[3]);
        QVERIFY(points[1] == points_verif[1]);
        QVERIFY(points[2] == points_verif[0]);
        QVERIFY(points[3] == points_verif[5]);
        QVERIFY(points[4] == points_verif[2]);
        QVERIFY(points[5] == points_verif[6]);
        QVERIFY(points[6] == points_verif[4]);

        // QPointF
        QVector<QPointF> pointsf = getPoints3();
        qSort(pointsf.begin(), pointsf.end(), AVConvexHull<QPointF>::sortFunction);

        QVector<QPointF> pointsf_verif = getPoints3();
        QVERIFY(points.count() == pointsf_verif.count());
        QCOMPARE(points.count(),7);

        QVERIFY(pointsf[0] == pointsf_verif[3]);
        QVERIFY(pointsf[1] == pointsf_verif[1]);
        QVERIFY(pointsf[2] == pointsf_verif[0]);
        QVERIFY(pointsf[3] == pointsf_verif[5]);
        QVERIFY(pointsf[4] == pointsf_verif[2]);
        QVERIFY(pointsf[5] == pointsf_verif[6]);
        QVERIFY(pointsf[6] == pointsf_verif[4]);

        qDebug("end %s", __FUNCTION__);
    }

    //----------------------------------------

    void tst_convexHullQPoint()
    {
        qDebug("start %s", __FUNCTION__);

        // good points
        QVector<QPoint> points = getPoints1();

        QVector<QPoint> ret = AVConvexHull<QPoint>::convexHull(points);

        AVLogDebug << "points1:";
        for(int cnt = 0; cnt < ret.count(); cnt++)
            AVLogDebug << "(" <<  ret[cnt].x() << "/" << ret[cnt].y() << ")";

        QCOMPARE(ret.count(),6);
        QVERIFY(ret[0] == points[3]);
        QVERIFY(ret[1] == points[1]);
        QVERIFY(ret[2] == points[6]);
        QVERIFY(ret[3] == points[4]);
        QVERIFY(ret[4] == points[0]);
        QVERIFY(ret[5] == points[3]);

        // difficult points
        points = getPoints2();
        ret = AVConvexHull<QPoint>::convexHull(points);

        AVLogDebug << "points2:";
        for(int cnt = 0; cnt < ret.count(); cnt++)
            AVLogDebug << "(" <<  ret[cnt].x() << "/" << ret[cnt].y() << ")";

        QCOMPARE(ret.count(),6);
        QVERIFY(ret[0] == points[3]);
        QVERIFY(ret[1] == points[1]);
        QVERIFY(ret[2] == points[6]);
        QVERIFY(ret[3] == points[4]);
        QVERIFY(ret[4] == points[0]);
        QVERIFY(ret[5] == points[3]);

        qDebug("end %s", __FUNCTION__);
    }

    void tst_convexHullQPointF()
    {
        qDebug("start %s", __FUNCTION__);

        // good points
        QVector<QPointF> points = getPoints3();

        QVector<QPointF> ret = AVConvexHull<QPointF>::convexHull(points);

        AVLogDebug << "points3:";
        for(int cnt = 0; cnt < points.count(); cnt++)
            AVLogDebug << "(" <<  points[cnt].x() << "/" << points[cnt].y() << ")";

        AVLogDebug << "ret:";
        for(int cnt = 0; cnt < ret.count(); cnt++)
            AVLogDebug << "(" <<  ret[cnt].x() << "/" << ret[cnt].y() << ")";


        QCOMPARE(ret.count(),6);
        QVERIFY(ret[0] == points[3]);
        QVERIFY(ret[1] == points[1]);
        QVERIFY(ret[2] == points[6]);
        QVERIFY(ret[3] == points[4]);
        QVERIFY(ret[4] == points[0]);
        QVERIFY(ret[5] == points[3]);

        qDebug("end %s", __FUNCTION__);
    }

    //----------------------------------------
protected:

    //! normal points
    QVector<QPoint> getPoints1()
    {
        //      x p0
        //
        //                        x p4
        //  x p3        x p2
        //           x p5
        //                    x p6
        //      x p1

        QVector<QPoint> points;
        points.append(QPoint( 0,20));  //p0
        points.append(QPoint( 0, 0));  //p1
        points.append(QPoint( 8,10));  //p2
        points.append(QPoint(-3,10));  //p3
        points.append(QPoint(20,15));  //p4
        points.append(QPoint( 6, 6));  //p5
        points.append(QPoint(10, 5)); //p6

        return points;
    }

    //! points with big values and repetitions
    QVector<QPoint> getPoints2()
    {
        //            x p0                  x p4
        //
        //
        //
        //  x p3              x p2
        //                 x p5
        //                          x p6
        //            x p1

        QVector<QPoint> points;
        points.append(QPoint( 0,INT_MAX));  //p0
        points.append(QPoint( 0, 0));  //p1
        points.append(QPoint( 8,10));  //p2
        points.append(QPoint(-INT_MAX,10));  //p3
        points.append(QPoint(INT_MAX,INT_MAX));  //p4
        points.append(QPoint( 6, 6));  //p5
        points.append(QPoint( 10, 5)); //p6
        points.append(QPoint( 10, 5)); //p6 (twice)
        points.append(QPoint( 8,10));  //p2 (twice)

        return points;
    }

    //! getPoints1 with QPointF
    QVector<QPointF> getPoints3()
    {
        //      x p0
        //
        //                        x p4
        //  x p3        x p2
        //           x p5
        //                    x p6
        //      x p1
        QVector<QPointF> points;
        points.append(QPointF(  0.0,20.3));  //p0
        points.append(QPointF(  0.0, 0.0));  //p1
        points.append(QPointF(  8.5,10.8));  //p2
        points.append(QPointF( -3.0,10.0));  //p3
        points.append(QPointF( 20.0,15.7));  //p4
        points.append(QPointF(  6.7, 6.0));  //p5
        points.append(QPointF( 10.0, 5.6));  //p6

        return points;
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVConvexHullTest,"avcalculationlib/unittests/config")
#include "avconvexhulltest.moc"

// End of file
