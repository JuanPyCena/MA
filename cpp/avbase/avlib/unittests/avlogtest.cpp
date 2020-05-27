///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Stefan Kunz, s.kunz@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Unit Tests for new logger. These tests assume that there are no
             logger-specific environment variables set.
*/



// System includes
#include <stdlib.h> // for putenv
#include <QTest>
#include <avunittestmain.h>

// Qt includes
#include <QtGlobal>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QVector>
#include <QSignalSpy>
#include <QProcess>
#include <QString>

// AVLib includes
#include "avexithandler.h"
#include "avconfig.h" // TODO QTP-10
#include "avlog.h"
#include "avlog/avlogmonitor.h"
#include "avlog/avlogmatcher.h"
#include "avshare.h"
#include "avmisc.h"
#include "avlogmuter.h"

#define LOG_TIME_PREFIX "[0-2][0-9]:[0-5][0-9]:[0-5][0-9] "
#define LOG_MONITORED(msg) QString("MONITORED: ") + msg
#define LOG_TIME_DEBUG_PREFIX LOG_TIME_PREFIX + AVLog::levelToPrefixString(AVLog::LOG__DEBUG)
#define LOG_TIME_INFO_PREFIX LOG_TIME_PREFIX + AVLog::levelToPrefixString(AVLog::LOG__INFO)
#define LOG_TIME_WARNING_PREFIX LOG_TIME_PREFIX + AVLog::levelToPrefixString(AVLog::LOG__WARNING)
#define LOG_TIME_ERROR_PREFIX LOG_TIME_PREFIX + AVLog::levelToPrefixString(AVLog::LOG__ERROR)
#define LOG_TIME_FATAL_PREFIX LOG_TIME_PREFIX + AVLog::levelToPrefixString(AVLog::LOG__FATAL)
#define LOG_VERSION_OUTPUT_REGEXP "Package [-\\w]*, version [-\\w\\.]*"
#define LOG_ARGUMENTS_OUTPUT_REGEXP "Arguments:.*"
#define LOG_USER_OUTPUT_REGEXP "Running as user [\\S]+"

#define ENSURE_NEW_LOG_FILE                                 \
        /* only log to file not stdout */                   \
        AVLog *logger = new TestLogger("writemodetest", false);    \
        QVERIFY(logger != 0);                              \
        AVLogMonitor mon(logger, true);                     \
        mon.setMinLevel(AVLog::LOG__INFO);                  \
        AVLOG_MONITOR_START(mon);                           \

#define ENSURE_LOG_FILE_REMOVED                             \
        QVERIFY(m_file.atEnd());                     \
        m_file.close();                                     \
        delete logger;                                      \
        QVERIFY(m_file.remove());

#define OPEN_LOG_FILE_READ                                  \
        m_file.setFileName(logger->curFileName());          \
        m_file.open(QIODevice::ReadOnly);                   \
        validatePreamble();

#if defined(Q_OS_WIN32)
#define putenv(a) _putenv(a)
#endif

/**
 *  The test logger ensures that the log file which it writes to is truncated at startup.
 *  Otherwise, leftover logfiles from previous failed tests would have an influcence on test results.
 */
class TestLogger : public AVLog
{
public:
    explicit TestLogger(const QString& filePrefix, const QString& logDir = 0,
                        const QString& loggerId = 0) :
        AVLog(filePrefix, logDir, loggerId)
        {
            init();
        }
    TestLogger(const QString& filePrefix, const bool stdo, const int level = LOG__INFO,
               const bool filelog = true, const QString& loggerId = 0) :
        AVLog(filePrefix, stdo, level, filelog, loggerId)
        {
            init();
        }

private:

    void init()
    {
        QVERIFY((*logWriter())->truncateLogfile());
        (*logWriter())->setLogThreadContextChanges(false);
    }
};

//! Object that increments a counter on each slot call
class TestSignalReceiver : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    TestSignalReceiver() { }

    QStringList paramsReceived;

public slots:

    void slotAddParam(const QString& param)
    {
        paramsReceived += param;
    }

private:
    //! Hidden copy-constructor
    TestSignalReceiver(const TestSignalReceiver&);
    //! Hidden assignment operator
    const TestSignalReceiver& operator = (const TestSignalReceiver&);
};


//! Exit handler that increments a counter on each slot call
class TestExitHandler : public AVExitHandler
{
public:
    TestExitHandler() { slotCount = 0; }

    uint slotCount;

    void handleExit() override { ++slotCount; }

private:
    //! Hidden copy-constructor
    TestExitHandler(const TestExitHandler&);
    //! Hidden assignment operator
    const TestExitHandler& operator = (const TestExitHandler&);
};

class AVLogTestListener : public AVLogListener
{
public:
    AVLogTestListener()
        : AVLogListener()
    {}

    void doAddLogMsg(int, const QString &message) override
    {
        m_log_content += message + "\n";
    }

    QString getLogContent() const { return m_log_content; }

private:
    QString m_log_content;
};

void standaloneFunction()
{
    AVLOG_INDENT;
    AVLogInfo << "working";
}

class AVLogTest : public QObject
{
    Q_OBJECT

protected:
    QString     m_refDateStr;
    QFile       m_file, m_mainFile;
    QTextStream m_stream;

private:
    // deletes log to get full output.
    QString extractTextFromLogger(AVLog* _log)
    {
        QString _log_filename = _log->curFileName();
        delete _log;

        QFile log_file(_log_filename);
        log_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream logFileStream(&log_file);
        QString ans = logFileStream.readAll();
        log_file.close();
        log_file.remove();
        return ans;
    }

    void memberFunction()
    {
        AVLOG_ENTER_METHOD();
    }

    void validatePreamble(QFile *file=nullptr)
    {
        if (file == nullptr) file = &m_file;

        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "LOG FILE OPENED, DATE: " + m_refDateStr, *file);
    }

private slots:

    void initTestCase()
    {
        AVEnvironment::setApplicationHome(QDir(AVEnvironment::getEnv("HOME", true)).absolutePath());
    }

    void init()
    {
        m_refDateStr = AVDateTime::currentDateTimeUtc().date().toString("yyyyMMdd");
        m_mainFile.setFileName(AVLogger->curFileName());
        QVERIFY(m_mainFile.open(QIODevice::ReadOnly));


        // Reset Root Logger Vars
        QString envvar, envvarvalue;
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_STDOUT_LINES);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_STDOUT_TIMESTAMPS);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));


        // Reset Application Level Vars
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_STDOUT_LINES);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_STDOUT_TIMESTAMPS);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
    }

    void cleanup()
    {
        m_file.close();
        m_mainFile.close();


        // Reset Root Logger Vars
        QString envvar, envvarvalue;
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));

        // Reset Application Level Vars
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("AVCOMMON")
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=";
        putenv(qstrdup( qPrintable(envvarvalue)));
    }

    /**
     * @brief validateLine reads the next line from the given file, and checks it against the
     *                     given regular expression.
     */
    void validateLine(const QString& regexpstr, QFile& file)
    {
        QRegExp re(regexpstr);
        char data[1000];
        QVERIFY(file.readLine(data, 1000) != -1);
        QString next_line = QString(data);
        if (next_line.indexOf(re) != 0)
        {
            QFile complete_file(file.fileName());
            complete_file.open(QIODevice::ReadOnly);
            AVLogInfo << "complete log file content of " << file.fileName()
                      << ":\n " << QString(complete_file.readAll());

            // not strictly correct, put gives better error output
            QCOMPARE(next_line, re.pattern());

            AVASSERT(false);
        }
    }
    //! See above
    void validateLine(const QString& regexpstr)
    {
        validateLine(regexpstr, m_file);
    }

    void testBaseFunctionality()
    {
        AVLOG_ENTER("AVLogTest::testBaseFunctionality");

        if(!AVEnvironment::getEnv("HOME").isEmpty())
        {
            QCOMPARE(AVLogger->curFileName(),
                     AVEnvironment::getEnv("HOME").replace(QChar('\\'), QChar('/')) +
                     "/log/AVLogTest-" +
                     m_refDateStr + ".log");
        }

        // constructor 1
        AVLog* logger1 = new TestLogger("loggerbasetest");
        QVERIFY(logger1 != 0);
        QVERIFY(logger1->logToFile());
        QVERIFY(! logger1->stdOut());
        QVERIFY(! logger1->guiOut(AVLog::LOG__INFO));
        QCOMPARE(logger1->closeInterval(), 0);
        QCOMPARE(logger1->minLevel(), (int)AVLog::LOG__INFO);
        QVERIFY(logger1->id().isNull());
        QCOMPARE(logger1->fileNamePrefix(), QString("loggerbasetest"));
        QVERIFY(logger1->fileNameOverride().isNull());

        logger1->Write(LOG_INFO, "OLD_LOG: param1=%d param2=%s", 99, "TEST");
        if(!AVEnvironment::getEnv("HOME").isEmpty())
        {
            QCOMPARE(logger1->curFileName(),
                     AVEnvironment::getApplicationHome() +
                                 "/log/loggerbasetest-" +
                     m_refDateStr + ".log");
        }
        m_file.setFileName(logger1->curFileName());
        m_file.open(QIODevice::ReadOnly);

        validatePreamble();
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "OLD_LOG: param1=99 param2=TEST");

        AVLogInfoTo(*logger1)<< "OLD_LOG: no params";

        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "OLD_LOG: no params");


        // constructor 2
        TestLogger logger2("loggerbasetest2", true, AVLog::LOG__ERROR,
                      false, "LID2");
        QVERIFY(! logger2.logToFile());
        QVERIFY(logger2.stdOut());
        QVERIFY(! logger2.guiOut(AVLog::LOG__ERROR));
        QCOMPARE(logger2.closeInterval(), 0);
        QCOMPARE(logger2.minLevel(), (int)AVLog::LOG__ERROR);
        QCOMPARE(logger2.id(), QString("LID2"));
        QCOMPARE(logger2.fileNamePrefix(), QString("loggerbasetest2"));
        QVERIFY(logger2.fileNameOverride().isNull());
        // nothing written yet
        QVERIFY(logger2.curFileName().isNull());

        delete logger1; // necessary under windows before m_file.remove()
        QVERIFY(m_file.remove());
    }

    void testExitHandler()
    {
        qDebug("AVLogTest::testExitHandler");

        AVLog* logger = new TestLogger("exithandlertest");
        QVERIFY(logger != 0);
        TestExitHandler* exitHandler = new TestExitHandler;
        QVERIFY(exitHandler != 0);
        logger->setExitHandler(exitHandler);
        QCOMPARE(exitHandler->slotCount, (uint)0);
        AVLogFatalTo(*logger)<< "FATAL LOG MESSAGE 1 (no exit)";
        QCOMPARE(exitHandler->slotCount, (uint)1);

        m_file.setFileName(logger->curFileName());
        m_file.open(QIODevice::ReadOnly);

        validatePreamble();
        validateLine(QString(LOG_TIME_FATAL_PREFIX) +
                     "FATAL LOG MESSAGE 1 \\(no exit\\)");
        validateLine(QString(" *at line="));

        delete logger;
        QVERIFY(m_file.remove());
    }

    void testNewLogFileSignal()
    {
        qDebug("AVLogTest::testNewLogFileSignal");

        TestSignalReceiver signalReceiver;

        AVLog* logger = new TestLogger("newlogfilesignaltest");
        QVERIFY(logger != 0);
        QObject::connect(logger, SIGNAL(signalNewLogFile(const QString&)),
                         &signalReceiver, SLOT(slotAddParam(const QString&)));

        QCOMPARE( (size_t)signalReceiver.paramsReceived.count(), (size_t)0);
        AVLogInfoTo(*logger)<< "New log message";
        QString fileName = logger->curFileName();
        QCOMPARE( (size_t)signalReceiver.paramsReceived.count(), (size_t)1);
        QCOMPARE(signalReceiver.paramsReceived[0], fileName);

        delete logger;
        QVERIFY(QFile::remove(fileName));
    }

    void testAssert()
    {
        qDebug("AVLogTest::testAssert");

        int i = 5, j = 0;
        if (i > 0)
            QVERIFY(i < 10);
        else
            QVERIFY(i == j);

        i = -1, j = -1;
        if (i > 0)
            QVERIFY(i < 10);
        else
            QVERIFY(i == j);
    }

    void testShortWriting()
    {
        qDebug("AVLogTest::testShortWriting");

        QVERIFY(m_mainFile.seek(m_mainFile.size())); // seek to eof
        AVLogInfo << "Info Message1 without params";
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Info Message1 without params", m_mainFile);
        QVERIFY(m_mainFile.seek(m_mainFile.size())); // seek to eof
        AVLogInfoTo(LOGGER_AVCOMMON_AVLIB)<< "Info Message2 without params";
        // same log file
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Info Message2 without params", m_mainFile);
        QVERIFY(m_mainFile.seek(m_mainFile.size())); // seek to eof
        LOGGER_AVCOMMON_AVLIB.Write(LOG_INFO, "Info Message3 with param %d", 1);
        // same log file
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Info Message3 with param 1", m_mainFile);

        AVLog* logger = new TestLogger("shortwritingtest");
        QVERIFY(logger != 0);
        AVLogInfoTo(*logger)<< "Info Message without params";

        m_file.setFileName(logger->curFileName());
        m_file.open(QIODevice::ReadOnly);

        validatePreamble();
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Info Message without params");

        delete logger;
        QVERIFY(m_file.remove());
    }

    void testFactory()
    {
        qDebug("AVLogTest::testFactory");

        AVLogFactory* logFactory = &AVLogFactory::getInstance();

        QVERIFY(logFactory->hasLogger(LOGGER_ROOT_ID));
        QVERIFY(! logFactory->hasLogger("UNREGISTERED_LOGGER"));

        QStringList registeredLoggers = logFactory->registeredLoggers();
        QVERIFY(registeredLoggers.count() >= 1);
        QVERIFY(registeredLoggers.contains(LOGGER_ROOT_ID));

        QVERIFY(&(logFactory->getLogger(LOGGER_ROOT_ID)) == AVLogger);

        static const QString TMP_LOGGER_ID = "TMP_LOGTEST_LOGGER_ID";

        QVERIFY(! logFactory->hasLogger(TMP_LOGGER_ID));
        // lazy creation
        AVLog* newLogger = &(logFactory->getLogger(TMP_LOGGER_ID));
        QVERIFY(newLogger != 0);
        registeredLoggers = logFactory->registeredLoggers();
        QVERIFY(registeredLoggers.count() >= 2);
        QVERIFY(registeredLoggers.contains(TMP_LOGGER_ID));
        QCOMPARE(newLogger->id(), TMP_LOGGER_ID);
        QCOMPARE(newLogger->minLevel(), (int)AVLog::LOG__INFO);
        QVERIFY(newLogger->stdOut());
        QVERIFY(newLogger->logToFile());
        QCOMPARE(newLogger->curFileName(), AVLogger->curFileName());
        QCOMPARE(newLogger->fileNameOverride(), AVLogger->fileNameOverride());
        QCOMPARE(newLogger->fileNamePrefix(), AVLogger->fileNamePrefix());
        QCOMPARE(newLogger->closeInterval(), AVLogger->closeInterval());

        QVERIFY(logFactory->hasLogger(TMP_LOGGER_ID));
        size_t logCount = (size_t) logFactory->registeredLoggers().count();
        QVERIFY(logFactory->unregisterLogger(TMP_LOGGER_ID));
        QCOMPARE((size_t)logFactory->registeredLoggers().count(), logCount-1);
        QVERIFY(! logFactory->hasLogger(TMP_LOGGER_ID));
    }

    void testFactoryRegister()
    {
        AVLogInfo << "---- testFactoryRegister ----";

        QString loggerId("TESTLOGGER");

        QVERIFY(!AVLogFactory::getInstance().hasLogger(loggerId));
        int num_loggers=AVLogFactory::getInstance().registeredLoggers().count();

        AVLog* logger = new (LOG_HERE) AVLog(QString(), QString(), loggerId);
        if(!AVLogFactory::getInstance().registerLogger(logger))
        {
            AVLogWarning << "Failed to register logger with id:" <<
                    loggerId;
        }
        else
        {
            AVLogInfo << "testFactoryRegister: registered";
        }

        QVERIFY(AVLogFactory::getInstance().hasLogger(loggerId));
        QCOMPARE(AVLogFactory::getInstance().registeredLoggers().count(), num_loggers+1);

        AVLogInfo << "---- testFactoryRegister ended ----";
    }

    //! Regression test for SWE-4906 to verify that open files are not inherited by sub-processes
    //! Starts "ping" subprocess to trigger fd inheritance.
    void testFileInheritance()
    {
        AVLOG_ENTER_METHOD();

        qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

        QString logfile_name;
        QProcess proc;
        bool could_remove_with_logger;
        {
            TestLogger logger("subproc_fd_inheritance_test");
            AVLogInfoTo(logger) << "test";
            logfile_name = logger.curFileName();
            AVLogInfo << logfile_name;

//            QSignalSpy spy(&proc, SIGNAL(started()));

#if defined(Q_OS_WIN32)
            proc.start("ping", QStringList() << "-t" << "localhost");
#else
            proc.start("cat", QStringList() << "/proc/version" << "-");
#endif
//            QVERIFY(spy.wait(5000));
            QSignalSpy spy2(&proc, SIGNAL(readyReadStandardOutput()));
            if (!spy2.wait(2000))
            {
                AVLogError << "stderr says: "
                           << proc.readAllStandardError();
                QFAIL("Could not start process");
            }
            AVLogInfo << "stdout: " << QString(proc.readAllStandardOutput());
            AVLogInfo << "stderr: " << QString(proc.readAllStandardError());

            could_remove_with_logger = QFile::remove(logfile_name);
        }

#if defined(Q_OS_WIN32)
        // logger is gone, but file descriptor inheritance prevents deletion. Reproduces SWE-4906.
        // evaluate results later so ping process is terminated correctly
        bool could_remove_after_logger = QFile::remove(logfile_name);
#endif
        proc.kill();
        QSignalSpy spy(&proc, SIGNAL(finished(int, QProcess::ExitStatus)));
        QVERIFY(spy.wait(5000));

#if defined(Q_OS_WIN32)
        bool could_remove_after_terminate = QFile::remove(logfile_name);
#endif

#if defined(Q_OS_WIN32)
        // was already removed beforehand in Qt5
        QVERIFY(!could_remove_after_terminate);
        // was still opened by our logger
        QVERIFY(!could_remove_with_logger);
        // currently prevented by fd inheritance
        QVERIFY(could_remove_after_logger);
#else
        // removal is immediately possible in lonux
        QVERIFY(could_remove_with_logger);
#endif
    }

    void testMultipleLoggersSingleFile()
    {
        qDebug("AVLogTest::testMultipleLoggersSingleFile");

        static const QString LOGGER1_ID = "LOGGER1_ID";
        static const QString LOGGER2_ID = "LOGGER2_ID";

        AVLog* logger1 = new TestLogger("loggertest", QString::null, LOGGER1_ID);
        QVERIFY(logger1 != 0);
        AVLog* logger2 = new TestLogger("loggertest", QString::null, LOGGER2_ID);
        QVERIFY(logger2 != 0);

        // different file handle because not yet registered in factory
        QVERIFY(*(logger1->logWriter()) != *(logger2->logWriter()));

        AVLogFactory* logFactory = &AVLogFactory::getInstance();
        QVERIFY(logFactory != 0);
        QVERIFY(logFactory->registerLogger(logger1));
        QVERIFY(logFactory->registerLogger(logger2));

        // same file handle now
        QVERIFY(*(logger1->logWriter()) == *(logger2->logWriter()));

        AVLogInfoTo(*logger1) << "Message from logger1";
        m_file.setFileName(logger1->curFileName());
        QVERIFY(m_file.open(QIODevice::ReadOnly));
        validatePreamble();
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Message from logger1");
        AVLogInfoTo(*logger2) << "Message from logger2";
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Message from logger2");

        // same file name after first write of each
        QCOMPARE(logger2->curFileName(), logger1->curFileName());

        // unregister to allow for immediate deletion
        QVERIFY(logFactory->unregisterLogger(logger1->id()));
        QVERIFY(logFactory->unregisterLogger(logger2->id()));

        delete logger1;
        delete logger2;

        QVERIFY(m_file.remove());
    }

    void testMultipleLoggersMultipleFiles()
    {
        qDebug("AVLogTest::testMultipleLoggersMultipleFiles");

        static const QString LOGGER1_ID = "LOGGER1_ID";
        static const QString LOGGER2_ID = "LOGGER2_ID";

        AVLog* logger1 = new TestLogger("loggertest1", QString::null, LOGGER1_ID);
        QVERIFY(logger1 != 0);
        AVLog* logger2 = new TestLogger("loggertest2", QString::null, LOGGER2_ID);
        QVERIFY(logger2 != 0);

        QVERIFY(*(logger1->logWriter()) != *(logger2->logWriter()));
        // nothing written yet
        QVERIFY(logger1->curFileName().isNull());
        QVERIFY(logger2->curFileName().isNull());

        QFile file1, file2;

        AVLogInfoTo(*logger1) << "Message from logger1";
        file1.setFileName(logger1->curFileName());
        QVERIFY(file1.open(QIODevice::ReadOnly));
        validatePreamble(&file1);
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Message from logger1", file1);

        AVLogInfoTo(*logger2)<< "Message from logger2";
        file2.setFileName(logger2->curFileName());
        QVERIFY(file2.open(QIODevice::ReadOnly));
        validatePreamble(&file2);
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     "Message from logger2", file2);

        QVERIFY(logger1->curFileName()  != logger2->curFileName());

        delete logger1;
        delete logger2;

        QVERIFY(file1.remove());
        QVERIFY(file2.remove());
    }

    void testEnvironmentVars()
    {
        qDebug("AVLogTest::testEnvironmentVars");

#if defined (_OS_WIN32_) || defined(Q_OS_WIN32)
        qDebug("This test is skipped under windows ...");
#else

        // this is necessary if AVDaemonInit was never called and we want to
        // use environ vars starting with the process name and logger id
        // instead of only the application name
        AVEnvironment::setProcessName("commontest");

        static const QString LOGGER1_ID = "LOGGER1_ID";
        static const QString LOGGER2_ID = "LOGGER2_ID";
        static const QString LOGGER3_ID = "LOGGER3_ID";
        static const QString LOGGER4_ID = "LOGGER4_ID";
        static const QString LOGGER4_ID_SPECIAL = "LOGGER4_ID_SPECIAL";

        // private method test
        QString value;
        int level = 0;
        bool found = false;
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_LEVEL, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_TO_FILE, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_FILE_PREFIX, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_CLOSE_INTERVAL, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_FILENAME, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_STDOUT, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_STDOUT_LINES, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);
        found = AVLogger->findEnvironVar(LOGGER_VARNAME_STDOUT_TIMESTAMPS, value, level);
        QVERIFY(! found);
        QCOMPARE(level, -1);

        TestLogger logger1("environtest1", QString::null, LOGGER1_ID);

        // functional test
        QVERIFY(logger1.logToFile());
        QVERIFY(! logger1.stdOut());
        QVERIFY(! logger1.guiOut(AVLog::LOG__INFO));
        QCOMPARE(logger1.closeInterval(), 0);
        QCOMPARE(logger1.minLevel(), (int)AVLog::LOG__INFO);
        QCOMPARE(logger1.id(), QString(LOGGER1_ID));
        QCOMPARE(logger1.fileNamePrefix(), QString("environtest1"));
        QVERIFY(logger1.fileNameOverride().isNull());
        QVERIFY(logger1.curFileName().isNull());

        QString envvar, envvarvalue;
        // Application Level Vars
        envvar = QString("%1_%2")
                 .arg(AVEnvironment::getApplicationName())
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=3"; // INFO
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg(AVEnvironment::getApplicationName())
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=1";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg(AVEnvironment::getApplicationName())
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=avcommon";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg(AVEnvironment::getApplicationName())
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=0";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg(AVEnvironment::getApplicationName())
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "="; // no override
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg(AVEnvironment::getApplicationName())
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=1";
        putenv(qstrdup(qPrintable(envvarvalue)));

        TestLogger logger2("environtest2", QString::null, LOGGER2_ID);

        // private method test
        found = logger2.findEnvironVar(LOGGER_VARNAME_LEVEL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 0);
        QCOMPARE(value, QString("3"));
        found = logger2.findEnvironVar(LOGGER_VARNAME_TO_FILE, value, level);
        QVERIFY(found);
        QCOMPARE(level, 0);
        QCOMPARE(value, QString("1"));
        found = logger2.findEnvironVar(LOGGER_VARNAME_FILE_PREFIX, value, level);
        QVERIFY(found);
        QCOMPARE(level, 0);
        QCOMPARE(value, QString("avcommon"));
        found = logger2.findEnvironVar(LOGGER_VARNAME_CLOSE_INTERVAL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 0);
        QCOMPARE(value, QString("0"));
        found = logger2.findEnvironVar(LOGGER_VARNAME_FILENAME, value, level);
        QVERIFY(!found);
        QCOMPARE(level, -1);
        found = logger2.findEnvironVar(LOGGER_VARNAME_STDOUT, value, level);
        QVERIFY(found);
        QCOMPARE(level, 0);
        QCOMPARE(value, QString("1"));

        // functional test
        QVERIFY(logger2.logToFile());
        QVERIFY(logger2.stdOut());
        QVERIFY(! logger2.guiOut(AVLog::LOG__INFO));
        QCOMPARE(logger2.closeInterval(), 0);
        QCOMPARE(logger2.minLevel(), (int)AVLog::LOG__INFO);
        QCOMPARE(logger2.id(), LOGGER2_ID);
        QCOMPARE(logger2.fileNamePrefix(), QString("avcommon"));
        QVERIFY(logger2.fileNameOverride().isNull());
        QVERIFY(logger2.curFileName().isNull());

        // Root Logger Vars
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=3"; // INFO
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=1";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=commontest";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=0";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "="; // no override
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2")
                 .arg("COMMONTEST")
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=1";
        putenv(qstrdup(qPrintable(envvarvalue)));

        TestLogger logger3("environtest3", QString::null, LOGGER3_ID);

        // private method test
        found = logger3.findEnvironVar(LOGGER_VARNAME_LEVEL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 1);
        QCOMPARE(value, QString("3"));
        found = logger3.findEnvironVar(LOGGER_VARNAME_TO_FILE, value, level);
        QVERIFY(found);
        QCOMPARE(level, 1);
        QCOMPARE(value, QString("1"));
        found = logger3.findEnvironVar(LOGGER_VARNAME_FILE_PREFIX, value, level);
        QVERIFY(found);
        QCOMPARE(level, 1);
        QCOMPARE(value, QString("commontest"));
        found = logger3.findEnvironVar(LOGGER_VARNAME_CLOSE_INTERVAL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 1);
        QCOMPARE(value, QString("0"));
        found = logger3.findEnvironVar(LOGGER_VARNAME_FILENAME, value, level);
        QVERIFY(!found);
        QCOMPARE(level, -1);
        found = logger3.findEnvironVar(LOGGER_VARNAME_STDOUT, value, level);
        QVERIFY(found);
        QCOMPARE(level, 1);
        QCOMPARE(value, QString("1"));

        // functional test
        QVERIFY(logger3.logToFile());
        QVERIFY(logger3.stdOut());
        QVERIFY(! logger3.guiOut(AVLog::LOG__INFO));
        QCOMPARE(logger3.closeInterval(), 0);
        QCOMPARE(logger3.minLevel(), (int)AVLog::LOG__INFO);
        QCOMPARE(logger3.id(), LOGGER3_ID);
        QCOMPARE(logger3.fileNamePrefix(), QString("commontest"));
        QVERIFY(logger3.fileNameOverride().isNull());
        QVERIFY(logger3.curFileName().isNull());

        // Specific logger vars
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID)
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=4"; // WARNING
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID)
                 .arg(LOGGER_VARNAME_TO_FILE);
        envvarvalue = envvar + "=0";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID)
                 .arg(LOGGER_VARNAME_FILE_PREFIX);
        envvarvalue = envvar + "=logger4prefix";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID)
                 .arg(LOGGER_VARNAME_CLOSE_INTERVAL);
        envvarvalue = envvar + "=60";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID)
                 .arg(LOGGER_VARNAME_FILENAME);
        envvarvalue = envvar + "=logger4override";
        putenv(qstrdup(qPrintable(envvarvalue)));
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID)
                 .arg(LOGGER_VARNAME_STDOUT);
        envvarvalue = envvar + "=0";
        putenv(qstrdup(qPrintable(envvarvalue)));

        TestLogger logger4("environtest4", QString::null, LOGGER4_ID);

        // private method test
        found = logger4.findEnvironVar(LOGGER_VARNAME_LEVEL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("4"));
        found = logger4.findEnvironVar(LOGGER_VARNAME_TO_FILE, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("0"));
        found = logger4.findEnvironVar(LOGGER_VARNAME_FILE_PREFIX, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("logger4prefix"));
        found = logger4.findEnvironVar(LOGGER_VARNAME_CLOSE_INTERVAL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("60"));
        found = logger4.findEnvironVar(LOGGER_VARNAME_FILENAME, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("logger4override"));
        found = logger4.findEnvironVar(LOGGER_VARNAME_STDOUT, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("0"));

        // functional test
        QVERIFY(! logger4.logToFile());
        QVERIFY(! logger4.stdOut());
        QVERIFY(! logger4.guiOut(AVLog::LOG__INFO));
        QCOMPARE(logger4.closeInterval(), 60*60);
        QCOMPARE(logger4.minLevel(), (int)AVLog::LOG__WARNING);
        QCOMPARE(logger4.id(), LOGGER4_ID);
        QCOMPARE(logger4.fileNamePrefix(), QString("logger4prefix"));
        QCOMPARE(logger4.fileNameOverride(), QString("logger4override"));
        QVERIFY(logger4.curFileName().isNull());

        // Override one single variable, all other will be taken from logger4
        envvar = QString("%1_%2_%3")
                 .arg("COMMONTEST")
                 .arg(LOGGER4_ID_SPECIAL)
                 .arg(LOGGER_VARNAME_LEVEL);
        envvarvalue = envvar + "=5"; // ERROR
        putenv(qstrdup(qPrintable(envvarvalue)));

        TestLogger logger4special("environtest4", QString::null, LOGGER4_ID_SPECIAL);

        // private method test
        found = logger4special.findEnvironVar(LOGGER_VARNAME_LEVEL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 4);
        QCOMPARE(value, QString("5"));
        found = logger4special.findEnvironVar(LOGGER_VARNAME_TO_FILE, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("0"));
        found = logger4special.findEnvironVar(LOGGER_VARNAME_FILE_PREFIX, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("logger4prefix"));
        found = logger4special.findEnvironVar(LOGGER_VARNAME_CLOSE_INTERVAL, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("60"));
        found = logger4special.findEnvironVar(LOGGER_VARNAME_FILENAME, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("logger4override"));
        found = logger4special.findEnvironVar(LOGGER_VARNAME_STDOUT, value, level);
        QVERIFY(found);
        QCOMPARE(level, 3);
        QCOMPARE(value, QString("0"));

        // functional test
        QVERIFY(! logger4special.logToFile());
        QVERIFY(! logger4special.stdOut());
        QVERIFY(! logger4special.guiOut(AVLog::LOG__INFO));
        QCOMPARE(logger4special.closeInterval(), 60*60);
        QCOMPARE(logger4special.minLevel(), (int)AVLog::LOG__ERROR);
        QCOMPARE(logger4special.id(), LOGGER4_ID_SPECIAL);
        QCOMPARE(logger4special.fileNamePrefix(), QString("logger4prefix"));
        QCOMPARE(logger4special.fileNameOverride(), QString("logger4override"));
        QVERIFY(logger4special.curFileName().isNull());

//         QVERIFY(QFile::remove(logger1.curFileName()));
//         QVERIFY(QFile::remove(logger2.curFileName()));

#endif // ! WIN32
    }

    void testLogMonitor()
    {
        // first general test to make sure the log monitor finds different expected lines
        {
            ENSURE_NEW_LOG_FILE

            logger->Write(LOG_INFO, "This is Info message 1");
            mon.filterOut("This is Info message 1", 1, 1, AVLog::LOG__INFO);

            OPEN_LOG_FILE_READ

            validateLine(QString(LOG_MONITORED(LOG_TIME_INFO_PREFIX)) +
                         "This is Info message 1");

            logger->Write(LOG_WARNING, "This is Warning message 1");
            mon.filterOut(QString("This is Warning message 1"), 1, 1, AVLog::LOG__WARNING);
            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                         "This is Warning message 1");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");

            logger->Write(LOG_ERROR, "This is Error message 1");
            mon.filterOut(QString("This is Error message 1"), 1, 1, AVLog::LOG__ERROR);
            validateLine(QString(LOG_MONITORED(LOG_TIME_ERROR_PREFIX)) +
                         "This is Error message 1");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");

            QVERIFY(mon.stop(0));

            // check that the report was written correctly
            validateLine(QString(LOG_TIME_INFO_PREFIX) + "========= AVLogMonitor Report "
                         "BEGIN from .*");
            validateLine("AVStringMatcher: level INFO log CONTAINS \"This is Info message 1\"");
            validateLine("Matched: Type \"INFO\" Message \"This is Info message 1\"");
            validateLine("AVStringMatcher: level WARNING log CONTAINS "
                         "\"This is Warning message 1\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 1\"");
            validateLine("AVStringMatcher: level ERROR log CONTAINS \"This is Error message 1\"");
            validateLine("Matched: Type \"ERROR\" Message \"This is Error message 1\"");
            validateLine("AVLogNoProblemsMatcher: mintype WARNING");
            validateLine("========= AVLogMonitor Report END =========");
            validateLine("\\s+");

            ENSURE_LOG_FILE_REMOVED
        }

        // second test looks for two matches of an expected log output
        {
            ENSURE_NEW_LOG_FILE

            // expected min 2 max 2 --> is 2
            logger->Write(LOG_WARNING, "This is Warning message 1");

            OPEN_LOG_FILE_READ

            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                                 "This is Warning message 1");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");
            logger->Write(LOG_WARNING, "This is Warning message 2");
            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                                         "This is Warning message 2");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");

            mon.filterOut(QRegExp("This is Warning message \\d"), 2, 2, AVLog::LOG__WARNING);

            // expected min -1 max 2 --> is 0
            mon.filterOut(QString("This is unfound Warning message 1"), -1, 2, AVLog::LOG__WARNING);

            // expected min -1 max -1 --> is 1
            logger->Write(LOG_WARNING, "This is Warning message 3");
            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                                         "This is Warning message 3");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");

            mon.filterOut(QString("This is Warning message 3"), -1, -1, AVLog::LOG__WARNING);

            QVERIFY(mon.stop(0));

            // check that the report was written correctly
            validateLine(QString(LOG_TIME_INFO_PREFIX) + "========= AVLogMonitor Report "
                         "BEGIN from .*");
            validateLine("AVRegExpMatcher: level WARNING \"This is Warning message \\\\d\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 1\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 2\"");
            validateLine("AVStringMatcher: level WARNING log CONTAINS "
                         "\"This is unfound Warning message 1\"");
            validateLine("AVStringMatcher: level WARNING log CONTAINS "
                         "\"This is Warning message 3\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 3\"");
            validateLine("AVLogNoProblemsMatcher: mintype WARNING");
            validateLine("========= AVLogMonitor Report END =========");
            validateLine("\\s+");

            ENSURE_LOG_FILE_REMOVED
        }

        // third test looks gets some expected and some unexpected matches of the log output
        {
            ENSURE_NEW_LOG_FILE

            // expected min 1 max 1 --> is 1
            logger->Write(LOG_WARNING, "This is Warning message 1");

            OPEN_LOG_FILE_READ

            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                                             "This is Warning message 1");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");

            mon.filterOut(QString("This is Warning message 1"), 1, 1, AVLog::LOG__WARNING);

            // expect min 1 max 1 --> is 2
            logger->Write(LOG_WARNING, "This is Warning message 3");
            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                                 "This is Warning message 3");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");
            logger->Write(LOG_WARNING, "This is Warning message 3");
            validateLine(QString(LOG_MONITORED(LOG_TIME_WARNING_PREFIX)) +
                                 "This is Warning message 3");
            validateLine(".*line=[0-9]+, file=.*avlogtest.cpp");
            mon.filterOut(QString("This is Warning message 3"), 1, 1, AVLog::LOG__WARNING);

            // expected min 1 max 1 --> is 0 (is Info instead of Warning)
            logger->Write(LOG_INFO, "This is Warning message 2");
            validateLine(QString(LOG_MONITORED(LOG_TIME_INFO_PREFIX)) +
                                         "This is Warning message 2");

            mon.filterOut(QString("This is Warning message 1"), 1, 1, AVLog::LOG__WARNING);

            QVERIFY(!mon.stop(0));

            // check that the report was written correctly
            validateLine(QString(LOG_TIME_INFO_PREFIX) + "========= AVLogMonitor Report "
                         "BEGIN from .*");
            validateLine("AVStringMatcher: level WARNING log CONTAINS "
                         "\"This is Warning message 1\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 1\"");
            validateLine("AVStringMatcher: level WARNING log CONTAINS "
                         "\"This is Warning message 3\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 3\"");
            validateLine("Matched: Type \"WARNING\" Message \"This is Warning message 3\"");
            validateLine("AVStringMatcher: level WARNING log CONTAINS "
                         "\"This is Warning message 1\"");
            validateLine("AVLogNoProblemsMatcher: mintype WARNING");
            validateLine("========= AVLogMonitor Report END =========");

            validateLine(AVLog::levelToString(AVLog::LOG__ERROR) +
                         ": ========= AVLogMonitor Issues BEGIN from .*");
            validateLine(QString("AVStringMatcher: level " +
                         AVLog::levelToString(AVLog::LOG__WARNING) + " log " +
                         AVLogStringMatcher::matchTypeToString(AVLogStringMatcher::CONTAINS) +
                          " \"This is Warning message 3\" Matched 2 "
                         "times. Expected: min=1, max=1"));
            validateLine(QString("AVStringMatcher: level " +
                         AVLog::levelToString(AVLog::LOG__WARNING) + " log " +
                         AVLogStringMatcher::matchTypeToString(AVLogStringMatcher::CONTAINS) +
                         " \"This is Warning message 1\" Matched 0 "
                         "times. Expected: min=1, max=1"));
            validateLine(QString("AVLogMonitor: Unexpected number of log lines. Expected 0 but "
                                 "got 1."));
            validateLine(QString("========= AVLogMonitor Issues END ========="));
            validateLine(QString(" *"));

            ENSURE_LOG_FILE_REMOVED
        }
    }

    void testLogMatcher()
    {
        AVLogger->Write(LOG_INFO,"testLogMatcher: start");

        // create the "log" input
        QVector<AVLogMatcherLogLine> lines;
        AVLogMatcherLogLine line1 = { AVLog::LOG__WARNING, "warning 1" };
        AVLogMatcherLogLine line2 = { AVLog::LOG__DEBUG, "debug 1" };
        AVLogMatcherLogLine line3 = { AVLog::LOG__WARNING, "warning 1" };
        AVLogMatcherLogLine line4 = { AVLog::LOG__ERROR, "error 1" };
        AVLogMatcherLogLine line5 = { AVLog::LOG__WARNING, "warning 123 2" };
        AVLogMatcherLogLine line6 = { AVLog::LOG__WARNING, "warning 444 2" };
        AVLogMatcherLogLine line7 = { AVLog::LOG__WARNING, "warning 1" };
        lines.append(line1);
        lines.append(line2);
        lines.append(line3);
        lines.append(line4);
        lines.append(line5);
        lines.append(line6);
        lines.append(line7);

        // create different matchers
        AVLogStringMatcher matcher1("warning 1", AVLogStringMatcher::EQUALS, AVLog::LOG__WARNING);
        matcher1.setMatchConstraints(3, 3);
        AVLogStringMatcher matcher2("warning 1", AVLogStringMatcher::EQUALS, AVLog::LOG__WARNING);
        matcher2.setMatchConstraints(-1, -1);
        AVLogStringMatcher matcher3("warning 1", AVLogStringMatcher::EQUALS, AVLog::LOG__WARNING);
        matcher3.setMatchConstraints(4, 4);
        AVLogStringMatcher matcher4("warning 1", AVLogStringMatcher::EQUALS, AVLog::LOG__WARNING);
        matcher4.setMatchConstraints(4, -1);
        AVLogStringMatcher matcher5("error 1", AVLogStringMatcher::EQUALS, AVLog::LOG__WARNING);
        AVLogStringMatcher matcher6("error 1", AVLogStringMatcher::EQUALS, AVLog::LOG__ERROR);
        AVLogRegExpMatcher matcher7(QRegExp("warning .*"));
        matcher7.setMatchConstraints(5, 5);
        AVLogRegExpMatcher matcher8(QRegExp("warning \\d{3} 2"));
        matcher8.setMatchConstraints(5, 5);
        AVLogRegExpMatcher matcher9(QRegExp("warning \\d{3} 2"));
        matcher9.setMatchConstraints(2, 2);
        AVLogTypeMatcher matcher10(AVLog::LOG__ERROR);
        AVLogTypeMatcher matcher11(AVLog::LOG__ERROR);
        matcher11.setMatchConstraints(5, 5);
        AVLogNoProblemsMatcher matcher12;
        AVLogNoProblemsMatcher matcher13(AVLog::LOG__FATAL);
        AVLogStringMatcher matcher14("warning", AVLogStringMatcher::CONTAINS, AVLog::LOG__WARNING);
        matcher14.setMatchConstraints(5, 5);

        // call match for each matcher
        QVector<AVLogMatcherLogLine>::const_iterator it;
        QVector<AVLogMatcherLogLine>::const_iterator itEnd = lines.constEnd();
        for (it = lines.constBegin(); it != itEnd; ++it){
            matcher1.match(*it);
            matcher2.match(*it);
            matcher3.match(*it);
            matcher4.match(*it);
            matcher5.match(*it);
            matcher6.match(*it);
            matcher7.match(*it);
            matcher8.match(*it);
            matcher9.match(*it);
            matcher10.match(*it);
            matcher11.match(*it);
            matcher12.match(*it);
            matcher13.match(*it);
            matcher14.match(*it);
        }

        // now check the result
        QVERIFY(matcher1.fullfilledConstraints());
        QVERIFY(matcher2.fullfilledConstraints());
        QVERIFY(!matcher3.fullfilledConstraints());
        QVERIFY(!matcher4.fullfilledConstraints());
        QVERIFY(!matcher5.fullfilledConstraints());
        QVERIFY(matcher6.fullfilledConstraints());
        QVERIFY(matcher7.fullfilledConstraints());
        QVERIFY(!matcher8.fullfilledConstraints());
        QVERIFY(matcher9.fullfilledConstraints());
        QVERIFY(matcher10.fullfilledConstraints());
        QVERIFY(!matcher11.fullfilledConstraints());
        QVERIFY(!matcher12.fullfilledConstraints());
        QVERIFY(matcher13.fullfilledConstraints());
        QVERIFY(matcher14.fullfilledConstraints());

        AVLogger->Write(LOG_INFO,"testLogMatcher: end");
    }

    void testLogStream()
    {
        AVLogger->Write(LOG_INFO,"testLogStream: start");

        AVLogInfo << "info log entry " << 1;

        AVLogDebug << "debug log entry " << 24;

        AVLogWarning << "warning log entry " << 313;

        AVLogError << "error log entry " << 4.24;

        //AVLogFatal << "fatal log entry " << -32.5;

        AVLog *logger1=new AVLog("testLogStream", true);
        QVERIFY(logger1 != NULL);

        AVLogInfoTo(*logger1) << "logger1 info log entry " << 4.23;

        QString logger1_filename = logger1->curFileName();

        delete logger1;

        //check if logger1 logged to file
        QFile logger1_file(logger1_filename);
        QVERIFY(logger1_file.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream logger1_file_stream(&logger1_file);
        QString logger1_file_contents=logger1_file_stream.readAll();
        logger1_file.close();
        QVERIFY(logger1_file_contents.contains("INFO:    logger1 info log entry 4.23"));
        QVERIFY(logger1_file.remove());
        AVLogger->Write(LOG_INFO,"testLogStream: end");
    }

    void testLogRepeatedMessagesSuppression1()
    {
        AVLog *logger1 = new AVLog("testLogRepeatedMessagesSuppression1", true);
        QVERIFY(logger1 != NULL);

        for(int i = 0; i < 100; ++i)
            AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(60, true) << "noisy message";

        QString logger1_file_contents = extractTextFromLogger(logger1);

        QCOMPARE(logger1_file_contents.count("INFO:    (suppressed for the next 60 seconds) noisy message"), 1);
        QCOMPARE(logger1_file_contents.count("noisy message"), 1);
        QCOMPARE(logger1_file_contents.count("(no longer applies)"), 0);
    }

    void testLogRepeatedMessagesSuppressionByText()
    {
        AVLog *logger1 = new AVLog("testLogRepeatedMessagesSuppressionByText", true);
        QVERIFY(logger1 != NULL);

        for(int i = 0; i < 20; ++i)
        {
            const QString ban_by_content_pattern("ban by content %1");
            AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(60, false, false) << ban_by_content_pattern.arg(i);

            const QString ban_by_context_pattern("ban by context %2");
            AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(60, false, true) << ban_by_context_pattern.arg(i);
        }

        QString logger1_file_contents = extractTextFromLogger(logger1);

        QCOMPARE(logger1_file_contents.count("ban by context"), 1);
        QCOMPARE(logger1_file_contents.count("ban by content"), 20);
    }

    void testLogRepeatedMessagesPeriodicalSuppression()
    {
        AVLog *logger1 = new AVLog("testLogRepeatedMessagesPeriodicalSuppression", true);

        bool exit_loop = false;
        for(int i = 0; i < 40; ++i) // provide ample time, but limit to 4 seconds (see SWE-3930)
        {
            AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, false, true) << "banned continuously";
            AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, true,  true)  << "banned and repeated";
            if (exit_loop) break;
            runEventLoop(100);

            if (logger1->m_messages_suppressed_by_position.count() < 2)
            {
                // log repeated another time, then bail
                exit_loop = true;
            }
        }

        QCOMPARE(logger1->m_messages_suppressed_by_position.count(), 2);

        QString logger_file_contents = extractTextFromLogger(logger1);

        QCOMPARE(logger_file_contents.count("banned continuously"), 1);
        QCOMPARE(logger_file_contents.count("banned and repeated"), 2);
    }

    void testRepeatedMessagesHousekeeping()
    {
        AVLog *logger1 = new AVLog("testRepeatedMessagesHousekeeping", true);
        QTime start_time;
        start_time.start();

        for(int i = 0; i < 10; ++i)
        {
            AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, false)  << QString("noisy message");
        }
        while (!logger1->m_messages_suppressed_by_text.empty())
        {
            runEventLoop(50);
            // don't be too strict about timing in case of busy test system, but set an upper limit...
            // see SWE-3930.
            QVERIFY(start_time.elapsed() < 10000);
        }
        AVLogInfo << "testRepeatedMessagesHousekeeping: suppression removed after " << start_time.elapsed() << " ms";

        QString logger_file_contents = extractTextFromLogger(logger1);

        QCOMPARE(logger_file_contents.count("(repeated occurrences suppressed) noisy message"), 1);
        QCOMPARE(logger_file_contents.count("(no longer applies) noisy message"), 1);
    }

    void testRepeatedMessagesInNewLogFile()
    {
        AVLog *logger1 = new AVLog("testRepeatedMessagesInNewLogFile", true);
        QVERIFY(logger1 != NULL);

        AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, false, true)  << "ban forever by position";
        AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, true,  true)  << "ban for a second by position";
        AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, false, false) << "ban forever by a message";
        AVLogInfoTo(*logger1) << AVLogStream::SuppressRepeated(1, true,  false) << "ban for a second by a message";

        logger1->slotRepeatedSuppressionsOverview();

        QString logger_file_contents = extractTextFromLogger(logger1);

        QCOMPARE(logger_file_contents.count("repeated occurrences suppressed"  ), 4);
        QCOMPARE(logger_file_contents.count("suppressed for the next 1 seconds"), 2);
    }

    //! SWE-3425
    void testLazyInitialization()
    {
        // This thread just creates the logger and then goes into a sleeping loop
        class TestThread : public QThread
        {
        public:
            TestThread() : m_run(true) {}
            void run() override
            {
                m_mutex.lock();
                m_log.reset(new AVLog("", "/tmp", "TESTLOGGER"));
                m_condition.wakeAll();
                m_mutex.unlock();
                while (m_run) AVSleep(10);
            }
            QMutex m_mutex;
            QWaitCondition m_condition;
            bool m_run;
            std::unique_ptr<AVLog> m_log;
        } thread;

        thread.m_mutex.lock();
        thread.start();
        thread.m_condition.wait(&thread.m_mutex);
        thread.m_mutex.unlock();

        qApp->processEvents();
        QVERIFY(thread.m_log->m_housekeeping_timer != nullptr);

        QSignalSpy spy(thread.m_log->m_housekeeping_timer, SIGNAL(timeout()));
        QVERIFY(spy.wait(2*AVLog::HOUSEKEEP_TIMER_INTERVAL_MS));
        QVERIFY(thread.m_log->logWriter()->getShare().m_openNewLogfileTimer->isActive());

        thread.m_run = false;
        thread.wait();
    }

    //! SWE-3197
    void testLogIndent()
    {
        // method name
        {
            AVLogTestListener listener;
            AVLogger->registerListener(&listener);
            standaloneFunction();
            AVLogger->unregisterListener(&listener);

            QString logged_string = listener.getLogContent();
            // the log listener does not capture the header...
            QRegExp re("->.*standaloneFunction.*\n"
                       "   working\n"
                       "<-.*standaloneFunction.*\n");
            QVERIFY(re.exactMatch(logged_string));
        }

        // arbitrary section name
        {
            AVLogTestListener listener;
            AVLogger->registerListener(&listener);
            {
                AVLOG_INDENT.setSectionName("mysection");
                AVLogInfo << "working";
            }
            AVLogger->unregisterListener(&listener);

            QString string_to_logout = listener.getLogContent();
            QCOMPARE(string_to_logout, QString("-> mysection\n"
                                               "   working\n"
                                               "<- mysection\n"));
        }

        // explicit usage without manipulator / macro
        {
            AVLogTestListener listener;
            AVLogger->registerListener(&listener);
            {
                AVLogIndent indent("explicit_usage");
                AVLogInfo << "working";
            }
            AVLogger->unregisterListener(&listener);

            QString string_to_logout = listener.getLogContent();
            QCOMPARE(string_to_logout, QString("-> explicit_usage\n"
                                               "   working\n"
                                               "<- explicit_usage\n"));
        }
    }

    //! SWE-3197
    void testLogIndentOnMemberFunc()
    {
        AVLogTestListener* listener = new AVLogTestListener();
        AVLogger->registerListener(listener);

        memberFunction();
        QString string_to_logout = listener->getLogContent();

        QCOMPARE(string_to_logout.count("AVLogTest::memberFunction"), 2);
        QRegExp exp(".*\\->.*AVLogTest::memberFunction.*\n.*<\\-.*AVLogTest::memberFunction.*");
        QVERIFY(exp.exactMatch(string_to_logout));
    }
    void testLogVersionString()
    {
        AVLog *logger = new TestLogger("testLogVersionString", true);
        logger->logPreamble(qApp->arguments());

        OPEN_LOG_FILE_READ

        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     LOG_VERSION_OUTPUT_REGEXP);
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     LOG_ARGUMENTS_OUTPUT_REGEXP);
        validateLine(QString(LOG_TIME_INFO_PREFIX) +
                     LOG_USER_OUTPUT_REGEXP);
        //ENSURE_LOG_FILE_REMOVED
    }

    void testWhiteListing() {
        AVLog log("test");
        class ExitHandler: public AVExitHandler {
            void handleExit() override {
                called = true;
            }
        public:
            bool called = false;
        } *exit_handler = new ExitHandler;

        log.setExitHandler(exit_handler);
        log.setFatalExit(false);
        AVLogErrorTo(log) << "Test";
        log.Write(LOG_ERROR, "Test old");
        QVERIFY(!exit_handler->called);
        AVEnvironment::setEnv(LOGGER_VARNAME_FATAL_ERRORS, "1");

        AVLogErrorTo(log) << "Test1";
        QVERIFY(exit_handler->called);
        exit_handler->called = false;

        log.Write(LOG_ERROR, "Test1 old log");
        QVERIFY(exit_handler->called);
        exit_handler->called = false;

        AVLog::addFatalErrorWhitelistRegex(QRegularExpression("Test"));

        AVLogErrorTo(log) << "Test2";
        QVERIFY(!exit_handler->called);
        exit_handler->called = false;

        log.Write(LOG_ERROR, "Test2 old");
        QVERIFY(!exit_handler->called);
        exit_handler->called = false;

        AVLogErrorTo(log) << "test3";
        QVERIFY(exit_handler->called);
        exit_handler->called = false;

        log.Write(LOG_ERROR, "test3");
        QVERIFY(exit_handler->called);
        exit_handler->called = false;

        AVEnvironment::setEnv(LOGGER_VARNAME_FATAL_ERRORS, "0");
    }

    void testRecategorizeQtWarnings() {
        AVLogTestListener* listener = new AVLogTestListener();
        listener->setMinLevel(AVLog::LOG__ERROR);
        AVLogger->registerListener(listener);

        QObject::connect(qApp, &QApplication::styleSheet, []{}); //clazy:exclude=connect-non-signal

        QString string_to_logout = listener->getLogContent();

        QVERIFY(string_to_logout.contains("QObject::connect: signal not found"));

    }

    void testAVLogMuter() {
        AVLogger->setMinLevel(AVLog::LOG__INFO);

        {
            AVLogMuter muter;
            QCOMPARE(AVLogger->minLevel(), static_cast<int>(AVLog::LOG__FATAL));
        }

        QCOMPARE(AVLogger->minLevel(), static_cast<int>(AVLog::LOG__INFO));

        {
            AVLogMuter muter(AVLog::LOG__WARNING);
            QCOMPARE(AVLogger->minLevel(), static_cast<int>(AVLog::LOG__WARNING));
        }

        QCOMPARE(AVLogger->minLevel(), static_cast<int>(AVLog::LOG__INFO));
    }

    void testAV_FUNC_INFO() const {
        QCOMPARE(AV_FUNC_INFO, QByteArrayLiteral("AVLogTest::testAV_FUNC_INFO"));
    }

    void testAV_FUNC_PREAMBLE() {
        AVLogTestListener listener;
        AVLogger->registerListener(&listener);

        AVLogInfo << AV_FUNC_PREAMBLE << "Log message";

        QString string_to_logout = listener.getLogContent();
        QVERIFY(string_to_logout.contains("AVLogTest::testAV_FUNC_PREAMBLE: Log message"));
        AVLogger->unregisterListener(&listener);
    }

    //       * Logger Template fuer AVShare
    //       * Doku
};

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVLogTest,"avlib/unittests/config")
#include "avlogtest.moc"


// End of file
