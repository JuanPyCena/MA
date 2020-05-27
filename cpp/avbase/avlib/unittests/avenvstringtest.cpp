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
  \brief   Unit test for the AVEnvString class
*/


#include "stdio.h"
#include <QTest>
#include <avunittestmain.h>
#include <stdlib.h>

#include <qstring.h>
#include <qdir.h>


#include "avconfig.h" // TODO QTP-10
#include "avlog.h"
#include "avenvstring.h"


///////////////////////////////////////////////////////////////////////////////

class AVEnvStringUnitTest : public QObject
{
    Q_OBJECT

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

    void testMainFunctionality()
    {
        AVLogger->Write(LOG_INFO, "Testing main functions of avenvstring");

        QString content,
                var_name_1,
                var_name_2,
                statement,
                expected;
        AVEnvString env_string;

        // Test non existing env variables
        expected = QString("$(FOOBAR)");
        env_string.setEnvString(expected);

        switchLogLevel(LOGGER_ROOT, AVLog::LOG__FATAL);
        QCOMPARE(env_string.expandEnvString(), expected);
        restoreLogLevel(LOGGER_ROOT);

        // Test empty string
        expected = QString::null;
        statement = QString("");
        env_string.setEnvString(statement);
        QVERIFY(env_string.isEmpty());
        QCOMPARE(env_string.expandEnvString(), expected);

        //Test string containing no env vars
        expected = QString("Any string");
        statement= expected;
        env_string.setEnvString(statement);
        QCOMPARE(env_string.expandEnvString(), expected);

        // Test with one env variable
        content  = QString("-any param");
        var_name_1 = QString("AVENVSTRING_UNITTEST_1");
        AVEnvironment::setEnv(var_name_1, content);

        expected = QString("%1 and stuff").arg(content);
        statement= QString("$(%1) and stuff").arg(var_name_1);
        env_string.setEnvString(statement);
        QCOMPARE(env_string.expandEnvString(), expected);

        // Test with two env variable
        content  = QString("-any param");
        var_name_1 = QString("AVENVSTRING_UNITTEST_1");
        var_name_2 = QString("AVENVSTRING_UNITTEST_2");
        AVEnvironment::setEnv(var_name_1, content);
        AVEnvironment::setEnv(var_name_2, content);

        expected = QString("%1 and %2").arg(content, content);
        statement= QString("$(%1) and $(%2)").arg(var_name_1, var_name_2);
        env_string.setEnvString(statement);
        QCOMPARE(env_string.expandEnvString(), expected);
    }
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVEnvStringUnitTest,"avlib/unittests/config")
#include "avenvstringtest.moc"

// End of file
