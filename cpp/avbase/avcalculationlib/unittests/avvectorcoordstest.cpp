///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Dietmar Goesseringer, d.goesseringer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   AVVectorCoords tests.
*/



// Qt includes
#include <QObject>
#include <QTest>
#include <avunittestmain.h>
#include <QApplication>
#include <QFile>
#include <QTextStream>

// AVLib includes
#include "avlog.h"
#include "avmisc.h"
#include "avvectorcoords.h"



const double delta = 0.0000000001;

///////////////////////////////////////////////////////////////////////////////

class AVVectorCoordsTest : public QObject
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

    void testAVVectorCoords()
    {
        // create AVVectorCoords objects
        AVVectorCoords *pvec0 = 0;
        AVVectorCoords *pvec1 = 0;
        AVVectorCoords *pvec2 = 0;
        AVVectorCoords *pvec3 = 0;
        pvec0 = new AVVectorCoords;
        QVERIFY(pvec0 != 0);
        pvec1 = new AVVectorCoords;
        QVERIFY(pvec1 != 0);
        pvec2 = new AVVectorCoords;
        QVERIFY(pvec2 != 0);
        pvec3 = new AVVectorCoords;
        QVERIFY(pvec3 != 0);

        AVVectorCoords vec0 = AVVectorCoords::Cartesian();
        AVVectorCoords vec1 = AVVectorCoords::Cartesian(0, 0);
        AVVectorCoords vec2 = AVVectorCoords::Cartesian(2.0, 3.0);
        AVVectorCoords vec3 = AVVectorCoords::Cartesian(5, 7);
        //AVVectorCoords vec4();
        // does not construct an object vec4 of type AVVectorCoords.
        // Instead, it is the prototype for a function vec4 with no arguments
        // and return type AVVectorCoords.

        // test isNull()
        {
            pvec2->setCartesian(2.0, 3.0);
            QVERIFY(  (vec0.isNull()) );
            QVERIFY(  (vec1.isNull()) );
            QVERIFY( !(vec2.isNull()) );
            QVERIFY(  (pvec0->isNull()) );
            QVERIFY(  (pvec1->isNull()) );
            QVERIFY( !(pvec2->isNull()) );
        }

        // test operator==
        {
            pvec1->setCartesian(0, 0);
            pvec2->setCartesian(2.0, 3.0);
            pvec3->setCartesian(2, 3);
            QVERIFY( (*pvec0 == *pvec1) );
            QVERIFY( (*pvec2 == *pvec3) );
            QVERIFY( !(*pvec0 == *pvec2) );
            QVERIFY( (vec0 == vec1) );
            QVERIFY( !(vec2 == vec3) );
            QVERIFY( !(vec0 == vec3) );
            QVERIFY( (*pvec0 == vec0) );
            QVERIFY( (*pvec1 == vec1) );
            QVERIFY( (*pvec2 == vec2) );
            QVERIFY( (vec2 == *pvec2) );
            QVERIFY( !(*pvec3 == vec3) );
        }

        // test setCartesian & operator== <cont'd>
        {
            pvec2->setCartesian(5, 7);
            QVERIFY( (*pvec2 == vec3) );
            vec2.setCartesian(0, 0);
            QVERIFY( (vec2 == vec1) );
            vec2.setCartesian(5.0, 7.0);
            QVERIFY( (vec2 == *pvec2) );
            vec3.setCartesian(5.0001, 7.0);
            QVERIFY( !(vec2 == vec3) );
            vec3.setCartesian(5.00000000001, 7.0);
            QVERIFY( (vec2 == vec3) );
        }

        // test x() & y()
        {
            vec1.setCartesian(0, 0);
            vec1.x() = 2.0;
            vec1.y() = 3.0;
            QVERIFY( (vec1 == AVVectorCoords::Cartesian(2.0, 3.0)) );
        }

        // test setPolarDeg & setPolarRad & setVelocity
        {
            vec1.setCartesian(1, 0);
            vec2.setVelocity(90, 1);
            QVERIFY( (vec2 == vec1) );
            vec2.setVelocity(0, 2);
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(0, 2)) );
            vec1.setCartesian(0, -4);
            vec2.setVelocity(180, 4);
            QVERIFY( (vec2 == vec1) );
            vec1.setCartesian(-2.2, 0.0);
            vec2.setVelocity(270, 2.2);
            QVERIFY( (vec2 == vec1) );
            vec1.setCartesian(4.0*cos(AV_PI/4.0), 4.0*sin(AV_PI/4.0));
            vec2.setVelocity(45, 4);
            QVERIFY( (vec2 == vec1) );
            vec1.setPolarDeg(4, -90);
            vec2.setVelocity(180, 4);
            vec3.setPolarDeg(4, 270);
            QVERIFY( (vec1 == vec2) );
            QVERIFY( (vec2 == vec3) );
            vec1.setPolarRad(5, AV_PI);
            vec2.setPolarDeg(5, 180);
            QVERIFY( (vec1 == vec2) );
            vec1.setPolarRad(0, 0);
            vec2.setPolarDeg(0, 0);
            QVERIFY( (vec1 == vec2) );
            vec1.setPolarRad(7.1, AV_PI/2.0);
            vec2.setPolarDeg(7.1, 90.0);
            QVERIFY( (vec1 == vec2) );
        }

        // test getCartesian*
        {
            qint32 ix = 0;
            double  fx = 0.0;
            qint32 iy = 0;
            double  fy = 0.0;
            vec1.setPolarDeg(7.1, 90.0);
            AV_DOUBLE_COMPARE(   0, vec1.getCartesianX(), delta );
            AV_DOUBLE_COMPARE( 7.1, vec1.getCartesianY(), delta );
            vec1.getCartesian(ix, iy);
            QVERIFY( (ix == 0) );
            QVERIFY( (iy == 7) );
            vec1.setCartesian(-2.2, 1.0);
            QVERIFY( (vec1.getCartesianX() == -2.2) );
            QVERIFY( (vec1.getCartesianY() == 1) );
            vec1.getCartesian(ix, iy);
            QVERIFY( (ix == -2) );
            QVERIFY( (iy ==  1) );
            vec1.getCartesian(fx, fy);
            QVERIFY( (fx == -2.2) );
            QVERIFY( (fy ==  1.0) );
        }

        // test getPolar*
        {
            double  fr   = 0.0;
            double  fphi = 0.0;
            vec1.setPolarDeg(7.1, 90.0);
            AV_DOUBLE_COMPARE(      7.1, vec1.getPolarRadius(),delta);
            AV_DOUBLE_COMPARE(     90.0, vec1.getPolarAngleDeg(),delta);
            AV_DOUBLE_COMPARE(AV_PI/2.0, vec1.getPolarAngleRad(),delta);
            vec1.getPolarDeg(fr, fphi);
            AV_DOUBLE_COMPARE(  7.1, fr, delta );
            AV_DOUBLE_COMPARE( 90.0, fphi, delta );
            vec1.setCartesian(5, 5);
            AV_DOUBLE_COMPARE(sqrt(50.0), vec1.getPolarRadius(),delta);
            AV_DOUBLE_COMPARE(      45.0, vec1.getPolarAngleDeg(),delta);
            AV_DOUBLE_COMPARE( AV_PI/4.0, vec1.getPolarAngleRad(),delta);
            vec1.getPolarDeg(fr, fphi);
            AV_DOUBLE_COMPARE(  sqrt(50.0), fr, delta );
            AV_DOUBLE_COMPARE(        45.0, fphi, delta );
            vec1.getPolarRad(fr, fphi);
            AV_DOUBLE_COMPARE(  sqrt(50.0), fr, delta );
            AV_DOUBLE_COMPARE(   AV_PI/4.0, fphi, delta );
            vec2.setPolarDeg(0, 0);
            vec2.getPolarDeg(fr, fphi);
            QVERIFY(   (fr == 0.0) );
            QVERIFY( (fphi == 0.0) );
            vec2.getPolarRad(fr, fphi);
            QVERIFY(   (fr == 0.0) );
            QVERIFY( (fphi == 0.0) );
        }

        // test getVelocity, speed, heading
        {
            double  fs = 0.0;
            double  fh = 0.0;
            vec1.setPolarDeg(0, 0);
            AV_DOUBLE_COMPARE(  0.0, vec1.speed(), delta );
            AV_DOUBLE_COMPARE(  0.0, vec1.heading(), delta );
            vec1.setPolarDeg(3, 30);
            AV_DOUBLE_COMPARE(  3.0, vec1.speed(), delta );
            AV_DOUBLE_COMPARE( 60.0, vec1.heading(), delta );
            vec1.getVelocity(fh, fs);
            AV_DOUBLE_COMPARE(  3.0, fs, delta );
            AV_DOUBLE_COMPARE( 60.0, fh, delta );
            vec1.setCartesian(-1, 1);
            AV_DOUBLE_COMPARE( sqrt(2.0), vec1.speed(), delta );
            AV_DOUBLE_COMPARE(     315.0, vec1.heading(), delta );
            vec1.getVelocity(fh, fs);
            AV_DOUBLE_COMPARE( sqrt(2.0), fs, delta );
            AV_DOUBLE_COMPARE(     315.0, fh, delta );
        }

        // test distance
        {
            vec1.setCartesian(2, 5);
            vec2.setCartesian(6, 1);
            AV_DOUBLE_COMPARE(        0.0, vec1.distance(vec1), delta );
            AV_DOUBLE_COMPARE( sqrt(32.0), vec1.distance(vec2), delta );
            AV_DOUBLE_COMPARE( sqrt(32.0), vec2.distance(vec1), delta );
        }

        // test perpendicularity
        {
            vec1.setCartesian(0.0, 10.0);
            vec2.setCartesian(10.0, 0.0);
            AV_DOUBLE_COMPARE(0.0, vec1 * vec2, delta);
            QVERIFY(vec1.isPerpendicular(vec2));
            QVERIFY(vec2.isPerpendicular(vec1));
            vec2.setCartesian(15.0, 0.0);
            AV_DOUBLE_COMPARE(0.0, vec1 * vec2, delta);
            QVERIFY(vec1.isPerpendicular(vec2));
            QVERIFY(vec2.isPerpendicular(vec1));
            vec2.setCartesian(15.0, 0.0001);
            QVERIFY(! vec1.isPerpendicular(vec2));
            QVERIFY(! vec2.isPerpendicular(vec1));
        }

        // test directions
        {
            vec1.setCartesian(5.0, 5.0);
            vec2.setCartesian(5.0, 1.0);
    /*
            qDebug("### angle(%s)=%.02f angle(%s)=%.02f", qPrintable(vec1.toString(false)),
                            vec1.getPolarAngleDeg(),
                                             qPrintable(vec2.toString(false)),
                                                              vec2.getPolarAngleDeg());
            qDebug("### heading(%s)=%.02f heading(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.heading(),
                                              qPrintable(vec2.toString(false)),
                                              vec2.heading());

            vec1.setCartesian(-5.0, 5.0);
            qDebug("### angle(%s)=%.02f angle(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.getPolarAngleDeg(),
                                              qPrintable(vec2.toString(false)),
                                                               vec2.getPolarAngleDeg());
            qDebug("### heading(%s)=%.02f heading(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.heading(),
                                              qPrintable(vec2.toString(false)),
                                                               vec2.heading());

            vec1.setCartesian(-5.0, -5.0);
            qDebug("### angle(%s)=%.02f angle(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.getPolarAngleDeg(),
                                              qPrintable(vec2.toString(false)),
                                                               vec2.getPolarAngleDeg());
            qDebug("### heading(%s)=%.02f heading(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.heading(),
                                              qPrintable(vec2.toString(false)),
                                                               vec2.heading());

            vec1.setCartesian(5.0, -5.0);
            qDebug("### angle(%s)=%.02f angle(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.getPolarAngleDeg(),
                                              qPrintable(vec2.toString(false)),
                                                               vec2.getPolarAngleDeg());
            qDebug("### heading(%s)=%.02f heading(%s)=%.02f", qPrintable(vec1.toString(false)),
                             vec1.heading(),
                                              qPrintable(vec2.toString(false)),
                                                              vec2.heading());
    */
        }

        // test operator=
        {
            vec1.setCartesian(-1.0, 1.2);
            vec2.setPolarDeg(0, 0);
            vec2 = vec1;
            QVERIFY( vec1 == vec2 );
            QVERIFY( vec2 == vec1 );
            AV_DOUBLE_COMPARE(vec1.speed(), vec2.speed(),delta );
            AV_DOUBLE_COMPARE(vec1.getPolarAngleRad(), vec2.getPolarAngleRad(),delta );
            vec1.setCartesian(-1, 1);
            vec2.setPolarDeg(0, 0);
            vec1 = vec2;
            QVERIFY( vec1 == vec2 );
            QVERIFY( vec2 == vec1 );
            AV_DOUBLE_COMPARE(vec1.speed(), vec2.speed(), delta );
            AV_DOUBLE_COMPARE(vec1.getPolarAngleRad(), vec2.getPolarAngleRad(),delta );
            vec1 = vec1;
            QVERIFY( vec1 == vec1 );
        }

        // test operator+ & operator+=
        {
            qint32 ix;
            double  fx = 5.56;
            qint32 iy;
            double  fy = 7.02;
            vec1.setCartesian(2, 3);
            vec2 = vec1;
            vec2 += vec2;
            vec2.getCartesian(ix, iy);
            QVERIFY( ix == 4 );
            QVERIFY( iy == 6 );
            vec3 = AVVectorCoords::Cartesian(fx, fy) + vec2;
            vec3.getCartesian(fx, fy);
            AV_DOUBLE_COMPARE( 6 + 7.02, fy, delta );
            vec3 = AVVectorCoords::Cartesian(-fx, -fy) + vec2;
            vec3.getCartesian(fx, fy);
            AV_DOUBLE_COMPARE( -5.56, fx, delta );
            AV_DOUBLE_COMPARE( -7.02, fy, delta );
        }

        // test operator- & operator-=
        {
            qint32 ix;
            double  fx = 5.56;
            qint32 iy;
            double  fy = 7.02;
            vec1.setCartesian(2, 3);
            vec2 = vec1;
            vec2 -= vec2;
            vec2.getCartesian(ix, iy);
            QVERIFY( ix == 0 );
            QVERIFY( iy == 0 );
            vec3 = AVVectorCoords::Cartesian(fx, fy) - vec1;
            vec3.getCartesian(fx, fy);
            AV_DOUBLE_COMPARE( 5.56 - 2, fx, delta );
            AV_DOUBLE_COMPARE( 7.02 - 3, fy, delta );
        }

        // test operator* & operator*=
        {
            double  fa = 2.0;
            vec1.setCartesian(2, 3);
            // test double*AVVectorCoords
            vec2 = fa * vec1;
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(4, 6)) );
            // test AVVectorCoords*double
            vec2 = vec1 * 2.2;
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(4.4, 6.6)) );
            // test multiplication with 0
            vec2 = vec1 * 0;
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(0, 0)) );
            // test multiplication with negatives
            vec2 = vec1 * (-1);
            vec3 = vec1+vec2;
            QVERIFY( (vec3 == AVVectorCoords::Cartesian(0, 0)) );
            vec2 = vec1 * (-1.5);
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(-3.0, -4.5)) );
            // test *=
            vec1 *= -1.5;
            QVERIFY( (vec1 == AVVectorCoords::Cartesian(-3.0, -4.5)) );
        }

        // test operator/
        {
            double  fa = 2.0;
            vec1.setCartesian(2, 3);
            // test AVVectorCoords/double
            vec2 = vec1 / fa;
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(1.0, 1.5)) );
            // test division by 0
            vec2 = vec1 / 0;
            QVERIFY( (vec2 == AVVectorCoords::Cartesian(0, 0)) );
        }

        // test normalise
        {
            vec1.setCartesian(2, 3);
            AV_DOUBLE_COMPARE( 3.605551275463989, vec1.norm(), delta );
            vec1.normalise();
            AV_DOUBLE_COMPARE( 1.0, vec1.norm(), delta );
        }

        // test rotate
        {
            vec1.setCartesian(1, 0);
            //AVLogger->Write(LOG_INFO, "%s=%s",
            //                qPrintable(vec1.toString()), qPrintable(vec1.toStringPolar()));
            AV_DOUBLE_COMPARE( 0.0, vec1.getPolarAngleDeg(), delta );
            vec1.rotateGrad(90.0);
            //AVLogger->Write(LOG_INFO, "%s=%s",
            //                qPrintable(vec1.toString()), qPrintable(vec1.toStringPolar()));
            AV_DOUBLE_COMPARE( 90.0, vec1.getPolarAngleDeg(), delta );
            vec1.rotateHeading(90.0);
            //AVLogger->Write(LOG_INFO, "%s=%s",
            //                qPrintable(vec1.toString()), qPrintable(vec1.toStringPolar()));
            AV_DOUBLE_COMPARE( 0.0, vec1.getPolarAngleDeg(), delta );
        }

        delete pvec3;
        delete pvec2;
        delete pvec1;
        delete pvec0;
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVVectorCoordsTest,"avcalculationlib/unittests/config")
#include "avvectorcoordstest.moc"


// End of file
