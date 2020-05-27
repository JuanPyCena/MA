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
  \brief   Unit test for the AVPath class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>
#include <stdlib.h>

#include <qstring.h>
#include <qdir.h>


#include "avconfig.h" // TODO QTP-10
#include "avlog.h"
#include "avpath.h"


///////////////////////////////////////////////////////////////////////////////

class AVPathUnitTest : public QObject
{
    Q_OBJECT


public:

    AVPathUnitTest()
    {
        m_avCommonHome   = QDir::cleanPath(AVEnvironment::getApplicationHome());
        m_avCommonConfig = QDir::cleanPath(AVEnvironment::getApplicationConfig());
        m_avCommonData   = QDir::cleanPath(AVEnvironment::getApplicationData());
        m_avCommonRun    = QDir::cleanPath(AVEnvironment::getApplicationRun());
        m_avCommonLog    = QDir::cleanPath(AVEnvironment::getApplicationLog());
    }

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

        QString m_avCommonHome;
        QString m_avCommonConfig;
        QString m_avCommonData;
        QString m_avCommonRun;
        QString m_avCommonLog;

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

    void testMainFunctionality()
    {
        AVLOG_ENTER_TESTCASE;

        QString path;
        AVPath::AppEnv matchedEnv;

        QString application_name = AVEnvironment::getApplicationName();

        AVPath pathExp(m_avCommonHome);
        QCOMPARE(pathExp.pathRelativeToAppEnv(&matchedEnv), QString("$("+application_name+"_HOME)/"));
        QCOMPARE(matchedEnv, AVPath::HOME);
        QCOMPARE(pathExp.expandPath(), m_avCommonHome);

        pathExp.setPath(m_avCommonConfig);
        QCOMPARE(pathExp.pathRelativeToAppEnv(&matchedEnv),QString("$("+application_name+"_CONFIG)/"));
        QCOMPARE(matchedEnv, AVPath::CONFIG);
        QCOMPARE(pathExp.expandPath(), m_avCommonConfig);

        pathExp.setPath(m_avCommonData);
        QCOMPARE(pathExp.pathRelativeToAppEnv(&matchedEnv), QString("$("+application_name+"_DATA)/"));
        QCOMPARE(matchedEnv, AVPath::DATA);
        QCOMPARE(pathExp.expandPath(), m_avCommonData);

        pathExp.setPath(m_avCommonRun);
        QCOMPARE(pathExp.pathRelativeToAppEnv(&matchedEnv), QString("$("+application_name+"_RUN)/"));
        QCOMPARE(matchedEnv, AVPath::RUN);
        QCOMPARE(pathExp.expandPath(), m_avCommonRun);

        pathExp.setPath(m_avCommonLog);
        QCOMPARE(pathExp.pathRelativeToAppEnv(&matchedEnv), QString("$("+application_name+"_LOG)/"));
        QCOMPARE(matchedEnv, AVPath::LOG);
        QCOMPARE(pathExp.expandPath(), m_avCommonLog);

        // test none existent env
        path = "$(FOOBAR)/home";
        pathExp.setPath(path);

        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        QCOMPARE(pathExp.expandPath(), path);
        restoreLogLevel(LOGGER_ROOT);

        // test path which can not be made relative to app env
        QCOMPARE(pathExp.pathRelativeToAppEnv(), path);

        // test exisiting env variable which doesn't start with application name
        path = "$(HOME)";
        pathExp.setPath(path);
        QString expected = QDir::homePath(); // $HOME should expand to QDir::homeDirPath
        QCOMPARE(pathExp.expandPath(), QDir::cleanPath(expected));

        // a pool of environment variables if one of those shouldn't exist
        QStringList envvars;

        envvars << "OPTAMOS_HOME" << "OPTAMOS_CONFIG" << "SNMP_HOME"
                << "long_lowercase_test_environment_variable" << "short_lower" << "A_B" << "a_b"
                << "a_home" << "home_a" << "A_HOME" << "HOME_A";

        int count = 0;
        for (QString var : envvars)
        {
            if (AVEnvironment::getEnv(var, false, AVEnvironment::NoPrefix).isNull())
                AVEnvironment::setEnv(var, "test" + QString::number(count++));
        }

        bool onechecked = false;
        for (QString var : envvars)
        {
            expected = AVEnvironment::getEnv(var, false, AVEnvironment::NoPrefix);

            if (!expected.isNull())
            {
                path = "$(" + var + ")";
                pathExp.setPath(path);
                AVLogInfo << "comparing: " << pathExp.expandPath() << " vs. " << QDir::cleanPath(expected);
                QCOMPARE(pathExp.expandPath(), QDir::cleanPath(expected));
                onechecked = true;
            }
        }
        QVERIFY(onechecked);
    }

    //----------------------------------------

    void testSearchOrder()
    {
        QString application_name = AVEnvironment::getApplicationName();

        AVPath::AppEnvList list;
        list << AVPath::HOME << AVPath::DATA;

        QCOMPARE(m_avCommonData, m_avCommonHome + "/data");

        AVPath pathExp(m_avCommonHome + "/data");
        pathExp.setEnvSearchOrder(list);
        QCOMPARE(pathExp.pathRelativeToAppEnv(), QString("$("+application_name+"_HOME)/data"));
        list.clear();
        list << AVPath::DATA << AVPath::HOME;
        pathExp.setEnvSearchOrder(list);
        QCOMPARE(pathExp.pathRelativeToAppEnv(), QString("$("+application_name+"_DATA)/"));
    }

    //----------------------------------------

    void testUrlConversion()
    {
#ifdef Q_OS_WIN
        QCOMPARE(AVPath::pathNameToUrl(R"(c:\path\to\file.txt)").toString(), QStringLiteral("file:///c:/path/to/file.txt"));
#else
        QCOMPARE(AVPath::pathNameToUrl(R"(/local/path/to/file)").toString(), QStringLiteral("file:///local/path/to/file"));
#endif
        QCOMPARE(AVPath::pathNameToUrl(":/name/to/resource").toString(), QStringLiteral("qrc:/name/to/resource"));


#ifdef Q_OS_WIN
        QCOMPARE(AVPath(R"(c:\path\to\file.txt)").expandUrl().toString(), QStringLiteral("file:///c:/path/to/file.txt"));
#else
        QCOMPARE(AVPath(R"(/local/path/to/file)").expandUrl().toString(), QStringLiteral("file:///local/path/to/file"));
#endif
        QCOMPARE(AVPath(":/name/to/resource").expandUrl().toString(), QStringLiteral("qrc:/name/to/resource"));


        // check for preserving last delimiter
#ifdef Q_OS_WIN
        QCOMPARE(AVPath(R"(c:\path\)").expandUrl().toString(), QStringLiteral("file:///c:/path/"));
#else
        QCOMPARE(AVPath(R"(/local/path/)").expandUrl().toString(), QStringLiteral("file:///local/path/"));
#endif
        QCOMPARE(AVPath(":/").expandUrl().toString(), QStringLiteral("qrc:/"));

    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVPathUnitTest,"avlib/unittests/config")
#include "avpathtest.moc"

// End of file
