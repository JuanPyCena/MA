///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*!
  \file
  \author  Andreas Schuller, a.schuller@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief   Unit test for the AVTrafoFactory class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>
#include <stdlib.h>

#include <qstring.h>


#include "avgausskruegersystemtrafo.h"
#include "avlog.h"
#include "avgausskruegertrafo.h"
#include "avgeoref.h"
#include "avgeoreftrafo.h"
#include "avmeteosattrafo.h"
#include "avmisc.h"
#include "avpath.h"
#include "avsystemtrafo.h"
#include "avtrafofactory.h"
#include "avweatherradartrafo.h"


///////////////////////////////////////////////////////////////////////////////

class AVTrafoFactoryTest : public QObject
{
    Q_OBJECT

public:
    AVTrafoFactoryTest() {}

    ~AVTrafoFactoryTest() override { AVTrafoFactorySingleton::destroyInstance(); }

protected:

    void switchLogLevel(AVLog& logger, int level)
    {
        m_savedLogLevels[logger.id()] = logger.minLevel();
        logger.setMinLevel(level);
    }

    void restoreLogLevel(AVLog& logger)
    {
        logger.setMinLevel(m_savedLogLevels[logger.id()]);
    }

    void initSavedLogLevels()
    {
        AVLogFactory& factory = AVLogFactory::getInstance();

        QStringList loggerIDs = factory.registeredLoggers();
        QStringList::const_iterator it    = loggerIDs.begin();
        QStringList::const_iterator endIt = loggerIDs.end();

        for (; it != endIt; ++it)
        {
            AVLog& logger = factory.getLogger(*it);

            m_savedLogLevels[logger.id()] = logger.minLevel();
        }
    }

protected:

        QMap<QString, int> m_savedLogLevels;
private slots:

    void init()
    {
        initSavedLogLevels();
    }

    //----------------------------------------

    void cleanup()
    {
    }

    //----------------------------------------

    void testTrafoByType()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testTrafoByName-----");

        // test with unkown type
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        QVERIFY(!TRAFOFACTORY.trafoByType("foo"));
        restoreLogLevel(LOGGER_ROOT);
    }

    //----------------------------------------

    void testAVSystemTrafo()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testAVSystemTrafo-----");

        AVCoordinateTransformBase* trafo = 0;
        QStringList paramList;
        double x = 0.0, y = 0.0;

        // ** trafo without parameters

        // create trafo only by type
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::SYSTEM_TRAFO);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),AVTrafoFactory::SYSTEM_TRAFO);
        QVERIFY(trafo == TRAFOFACTORY.trafoByType(AVTrafoFactory::SYSTEM_TRAFO));

        // create trafo with type and name
        trafo = TRAFOFACTORY.trafoByTypeAndName(AVTrafoFactory::SYSTEM_TRAFO, "mySystemTrafo");
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),QString("mySystemTrafo"));

        trafo->latlong2xy(47.5, 14.0, x, y);
        QVERIFY2(                               (fabs(x) < 1e-8) && (fabs(y) < 1e-8),
                                                "Origin of created AVSystemTrafo is invalid");

        // create trafo with parameter list
        paramList = AVTrafoFactory::createSystemParamList(50.0, 20.0);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::SYSTEM_TRAFO, paramList);
        trafo->latlong2xy(50.0, 20.0, x, y);
        QVERIFY2(                                (fabs(x) < 1e-8) && (fabs(y) < 1e-8),
                                                 "Origin of created AVSystemTrafo is invalid");

        // test trafo with invalid number of parameters
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        paramList.clear();
        paramList << "arg1";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::SYSTEM_TRAFO, paramList));
        paramList << "arg2" << "arg3";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::SYSTEM_TRAFO, paramList));
        restoreLogLevel(LOGGER_ROOT);
    }

    //----------------------------------------

    void testAVWeatherRadarTrafo()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testAVSystemTrafo-----");

        AVCoordinateTransformBase* trafo = 0;
        QStringList paramList;

        // ** trafo without parameters

        // create trafo only by type
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::WEATHER_RADAR_TRAFO);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),AVTrafoFactory::WEATHER_RADAR_TRAFO);
        QVERIFY(trafo == TRAFOFACTORY.trafoByType(AVTrafoFactory::WEATHER_RADAR_TRAFO));

        // create trafo with type and name
        trafo = TRAFOFACTORY.trafoByTypeAndName(AVTrafoFactory::WEATHER_RADAR_TRAFO, "myWTrafo");
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),QString("myWTrafo"));

        QVERIFY(TRAFOFACTORY.trafoByTypeAndName(
                           AVTrafoFactory::WEATHER_RADAR_TRAFO, "myWTrafo") !=
                       TRAFOFACTORY.trafoByType(AVTrafoFactory::WEATHER_RADAR_TRAFO));

        // test rafo with invalid number of parameters
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        paramList << "arg1";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::WEATHER_RADAR_TRAFO, paramList));
        paramList << "arg2";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::WEATHER_RADAR_TRAFO, paramList));
        restoreLogLevel(LOGGER_ROOT);
    }

    //----------------------------------------

    void testAVMeteoSatTrafo()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testAVMeteoSatTrafo-----");

        AVCoordinateTransformBase* trafo = 0;
        QStringList paramList;

        // ** trafo without parameters

        // create trafo only by type
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::METEO_SAT_TRAFO);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),AVTrafoFactory::METEO_SAT_TRAFO);
        QVERIFY(trafo == TRAFOFACTORY.trafoByType(AVTrafoFactory::METEO_SAT_TRAFO));

        // create trafo with type and name
        trafo = TRAFOFACTORY.trafoByTypeAndName(AVTrafoFactory::METEO_SAT_TRAFO, "myMSatTrafo");
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),QString("myMSatTrafo"));

        QVERIFY(TRAFOFACTORY.trafoByTypeAndName(
                           AVTrafoFactory::METEO_SAT_TRAFO, "myMSatTrafo") !=
                       TRAFOFACTORY.trafoByType(AVTrafoFactory::METEO_SAT_TRAFO));

        // test trafo with invalid number of parameters
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        paramList << "arg1";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::METEO_SAT_TRAFO, paramList));
        paramList << "arg2";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::METEO_SAT_TRAFO, paramList));
        restoreLogLevel(LOGGER_ROOT);
    }

    //----------------------------------------

    void testAVGeoRefTrafo()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testAVMeteoSatTrafo-----");

        AVCoordinateTransformBase* trafo = 0;
        QStringList paramList;

        // Use tie point file from wlayertest
        // We assume here that commontest will be started in $AVCOMMON/src/tests/unit

        QString geoRefFile = AVEnvironment::getApplicationConfig() + "/u_radar_eu.udl";

        paramList = AVTrafoFactory::createGeoRefParamList(geoRefFile);

        // create trafo only by type
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GEO_REF_TRAFO, paramList);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),AVTrafoFactory::GEO_REF_TRAFO);
        QVERIFY(trafo == TRAFOFACTORY.trafoByType(AVTrafoFactory::GEO_REF_TRAFO,paramList));

        // create trafo with type and name
        trafo = TRAFOFACTORY.trafoByTypeAndName(AVTrafoFactory::GEO_REF_TRAFO,"myGeoRefTrafo",
                                                paramList);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),QString("myGeoRefTrafo"));

        QVERIFY(TRAFOFACTORY.trafoByTypeAndName(
                           AVTrafoFactory::GEO_REF_TRAFO, "myGeoRefTrafo", paramList) !=
                       TRAFOFACTORY.trafoByType(AVTrafoFactory::GEO_REF_TRAFO, paramList));

        // test trafo with invalid number of parameters
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::GEO_REF_TRAFO));
        QVERIFY(! TRAFOFACTORY.trafoByTypeAndName(
                            AVTrafoFactory::GEO_REF_TRAFO, "fooGeoRef"));
        paramList.clear();
        paramList << "arg1";
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::GEO_REF_TRAFO, paramList));
        paramList << "arg2" << "arg3";
        restoreLogLevel(LOGGER_ROOT);

        // test trafo with none existent tie point file
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        paramList = AVTrafoFactory::createGeoRefParamList("somewhere");
        QVERIFY(! TRAFOFACTORY.trafoByType(AVTrafoFactory::GEO_REF_TRAFO, paramList));
        restoreLogLevel(LOGGER_ROOT);
    }

    //----------------------------------------

    void testAVGaussKruegerSystemTrafo()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testAVGaussKruegerSystemTrafo-----");

        AVCoordinateTransformBase* trafo = 0;
        QStringList paramList;

        paramList = AVTrafoFactory::createGaussKruegerSystemParamList(
            AVGaussKruegerSystemTrafo::RM_BESSEL_M28);

        // create trafo only by type
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO);
        QVERIFY(trafo == TRAFOFACTORY.trafoByType(
                        AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList));

        // create trafo with type and name
        trafo = TRAFOFACTORY.trafoByTypeAndName(
                    AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, "myGaussKruegerSystemTrafo",
                    paramList);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),QString("myGaussKruegerSystemTrafo"));

        QVERIFY(TRAFOFACTORY.trafoByTypeAndName(
                       AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO,
                       "myGaussKruegerSystemTrafo", paramList) !=
                       TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO,
                                                paramList));

        // test trafo with all known ref meridians
        paramList = AVTrafoFactory::createGaussKruegerSystemParamList(
            AVGaussKruegerSystemTrafo::RM_BESSEL_M28);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList);
        QVERIFY(trafo);

        paramList = AVTrafoFactory::createGaussKruegerSystemParamList(
            AVGaussKruegerSystemTrafo::RM_BESSEL_M31);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList);
        QVERIFY(trafo);

        paramList = AVTrafoFactory::createGaussKruegerSystemParamList(
            AVGaussKruegerSystemTrafo::RM_BESSEL_M34);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList);
        QVERIFY(trafo);

        // test with invalid parameters
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO);
        trafo = TRAFOFACTORY.trafoByTypeAndName(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO,
                                                "fooGaussKruegerSystem");
        QVERIFY(! trafo);

        paramList.clear();
        paramList << "1000";
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList);
        QVERIFY(! trafo);

        paramList << "arg2";
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_SYSTEM_TRAFO, paramList);
        QVERIFY(! trafo);
        restoreLogLevel(LOGGER_ROOT);
    }

    //----------------------------------------

    void testAVGaussKruegerTrafo()
    {
        AVLogger->Write(LOG_INFO, "----- AVTrafoFactoryTest: testAVGaussKruegerTrafo-----");

        AVCoordinateTransformBase* trafo = 0;
        QStringList paramList;

        paramList = AVTrafoFactory::createGaussKruegerParamList(20.0,
                                                                AVCoordEllipsoid::ET_WGS84);

        // create trafo only by type
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),AVTrafoFactory::GAUSS_KRUEGER_TRAFO);
        QVERIFY(trafo == TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO,
                                                  paramList));

        // create trafo with type and name
        trafo = TRAFOFACTORY.trafoByTypeAndName(AVTrafoFactory::GAUSS_KRUEGER_TRAFO,
                                                "myGaussKrueger", paramList);
        QVERIFY(trafo);
        QCOMPARE(trafo->name(),QString("myGaussKrueger"));

        QVERIFY(TRAFOFACTORY.trafoByTypeAndName(
                           AVTrafoFactory::GAUSS_KRUEGER_TRAFO, "myGaussKrueger", paramList) !=
                       TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList));


        // test trafo with all known ref ellipsoids
        paramList = AVTrafoFactory::createGaussKruegerParamList(20.0,
                                                                AVCoordEllipsoid::ET_WGS84);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList);
        QVERIFY(trafo);

        paramList = AVTrafoFactory::createGaussKruegerParamList(20.0,
                                                                AVCoordEllipsoid::ET_BESSEL);
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList);
        QVERIFY(trafo);

        // test customized ref ellipsoid
        AVCoordEllipsoid refEll(AVCoordEllipsoid::ET_BESSEL);
        paramList = AVTrafoFactory::createGaussKruegerParamList(10.0, refEll.geta(),
                                                                refEll.getf());

        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList);
        QVERIFY(trafo);

        // test trafo with invalid parameters
        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        paramList = AVTrafoFactory::createGaussKruegerParamList(20.0,
                                                                AVCoordEllipsoid::ET_CUSTOM);
        // for custum ref ellipsoids calling createGaussKruegerParamList with 2 params must return
        // an empty list
        QVERIFY(paramList.isEmpty());

        paramList.clear();
        paramList << "arg1";
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList);
        QVERIFY(! trafo);

        paramList << "arg2" << "arg3" << "arg4";
        trafo = TRAFOFACTORY.trafoByType(AVTrafoFactory::GAUSS_KRUEGER_TRAFO, paramList);
        QVERIFY(! trafo);
        restoreLogLevel(LOGGER_ROOT);
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVTrafoFactoryTest,"avcalculationlib/unittests/config")
#include "avtrafofactorytest.moc"


// End of file
