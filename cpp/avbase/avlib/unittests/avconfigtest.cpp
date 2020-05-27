///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Wolfgang Eder, w.eder@avibit.com
    \author   QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief    Tests the AVConfigBase and AVConfigList classes
*/


#include <iostream>
#include <QTest>
#include <avunittestmain.h>

// Qt includes
#include <QDir>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTemporaryDir>

// AviBit common includes
#include "avconfig.h" // TODO QTP-10
#include "avconfiglist.h"

///////////////////////////////////////////////////////////////////////////////

class Config : public AVConfigBase
{

public:
    Config() : AVConfigBase("avconfigtest"), triedToExit(false)
        { ADDCEP("str", &str, "default", "a string", "str"); }
    void exitApp(int) override { triedToExit = true; }

public:
    QString str;
    bool triedToExit;
};


///////////////////////////////////////////////////////////////////////////////

class ConfigWithStandardInit : public Config
{
public:
    ConfigWithStandardInit() : Config() { standardInit(); }
};

///////////////////////////////////////////////////////////////////////////////

class ConfigWithReadConfig : public Config
{
public:
    ConfigWithReadConfig() : Config() { readConfig(); }
};

///////////////////////////////////////////////////////////////////////////////

class ConfigWithParseParams : public Config
{
public:
    ConfigWithParseParams() : Config() { parseParams(); }
};

///////////////////////////////////////////////////////////////////////////////

class AVConfigTest : public QObject
{
  Q_OBJECT

private:

    QString contents(const QString& str, const QString& astr = "aaa", const QString& bstr = "bbb");
    void writeStr(const QString& str = "test", const QString& configfile = "avconfigtest.cfg");
    void readStr(const QString& str = "test", const QString& configfile = "avconfigtest.cfg");
    QString filename(const QString& configfile);
    void writeConfig(const QString& contents, const QString& configfile = "avconfigtest.cfg");
    void readConfig(const QString& expected, const QString& configfile = "avconfigtest.cfg");

    QString m_configPath;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();
    void testWithoutParams();
    void testWithParam();
    void testOptionSave();
    void testParamWithOptionSave();
    void testConfigList();
    void testConfigListWithOptionSave();
    void testStandardPathsAreAbsolute();


private:

    QString m_config_path;
    QTemporaryDir m_tempdir;
};

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::init()
{
    AVConfigBase::params() = QStringList("commontest");
    AVConfigBase::option_help = false;
    AVConfigBase::option_version = false;
    AVConfigBase::option_save = false;
    AVConfigBase::option_cfg = "";
    AVConfigBase::option_defps = "";
    AVConfigBase::option_debug = 0;

    QString configfile = "avconfigtest.cfg";

    QVERIFY(m_tempdir.isValid());

    QFile::remove(m_tempdir.path() + "/" + configfile);
    QFile::copy(filename(configfile),m_tempdir.path() + "/" + configfile);
    m_config_path = AVEnvironment::getApplicationConfig();

    qDebug() << "Setting configpath to: " << m_tempdir.path();
    AVEnvironment::setApplicationConfig(m_tempdir.path());
}

void AVConfigTest::initTestCase()
{
    QString configfile = "avconfigtest.cfg";

    QVERIFY(m_tempdir.isValid());
    QString tempdir = m_tempdir.path();
    qDebug() << "temp dir is: " << tempdir;

    QString tmpfile = tempdir + "/" + configfile;
    if(QFile::exists(tmpfile))
    {
        qDebug() << tmpfile << " exists. trying to remove...";
        QVERIFY(QFile::remove(tmpfile));
    }
    else
    {
        qDebug() << tmpfile << " doesn't exist -> OK";
    }
    qDebug() << "copying " << filename(configfile) << " to " << tmpfile;
    QVERIFY(QFile::copy(filename(configfile), tmpfile));

    m_configPath = AVEnvironment::getApplicationConfig();

    AVEnvironment::setApplicationConfig(tempdir);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::cleanupTestCase()
{
    AVEnvironment::setApplicationConfig(m_configPath);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::cleanup()
{
    // It's important to set option_save to false, because
    // subsequent tests using a readConfig or writeConfig will exit
    AVConfigBase::option_save = false;

    qDebug() << "Resetting configpath to: " << m_config_path;
    AVEnvironment::setApplicationConfig(m_config_path);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testWithoutParams()
{
    writeStr();
    Config config;
    QCOMPARE(config.str, QString("default"));

    ConfigWithStandardInit configWithStandardInit;
    QCOMPARE(configWithStandardInit.str, QString("test"));

    ConfigWithReadConfig configWithReadConfig;
    QCOMPARE(configWithReadConfig.str, QString("test"));

    ConfigWithParseParams configWithParseParams;
    QCOMPARE(configWithParseParams.str, QString("default"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testWithParam()
{
    writeStr();
    AVConfigBase::params() << "-str" << "override";
    Config config;
    QCOMPARE(config.str, QString("default"));

    AVConfigBase::params() << "-str" << "override";
    ConfigWithStandardInit configWithStandardInit;
    QCOMPARE(configWithStandardInit.str, QString("override"));

    AVConfigBase::params() << "-str" << "override";
    ConfigWithReadConfig configWithReadConfig;
    QCOMPARE(configWithReadConfig.str, QString("test"));

    AVConfigBase::params() << "-str" << "override";
    ConfigWithParseParams configWithParseParams;
    QCOMPARE(configWithParseParams.str, QString("override"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testOptionSave()
{
    AVConfigBase::option_save = true;
    writeStr();

    AVLogInfo << "AVConfigTest::testOptionSave: Config";
    Config config;
    QCOMPARE(config.str, QString("default"));
    QVERIFY(!config.triedToExit);
    readStr();

    AVLogInfo << "AVConfigTest::testOptionSave: ConfigWithStandardInit";
    ConfigWithStandardInit configWithStandardInit;
    QCOMPARE(configWithStandardInit.str, QString("test"));
    QVERIFY(configWithStandardInit.triedToExit);
    readStr();

    AVLogInfo << "AVConfigTest::testOptionSave: ConfigWithReadConfig";
    ConfigWithReadConfig configWithReadConfig;
    QCOMPARE(configWithReadConfig.str, QString("test"));
    QVERIFY(configWithReadConfig.triedToExit);
    readStr();

    AVLogInfo << "AVConfigTest::testOptionSave: ConfigWithParseParams";
    ConfigWithParseParams configWithParseParams;
    QCOMPARE(configWithParseParams.str, QString("default"));
    QVERIFY(configWithParseParams.triedToExit);
    readStr("default");
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testParamWithOptionSave()
{
    AVConfigBase::option_save = true;
    AVConfigBase::params() << "-str" << "override";
    writeStr();
    Config config;
    QCOMPARE(config.str, QString("default"));
    QVERIFY(!config.triedToExit);
    readStr();

    AVConfigBase::params() << "-str" << "override";
    ConfigWithStandardInit configWithStandardInit;
    QCOMPARE(configWithStandardInit.str, QString("override"));
    QVERIFY(configWithStandardInit.triedToExit);
    readStr("override");

    writeStr();
    AVConfigBase::params() << "-str" << "override";
    ConfigWithReadConfig configWithReadConfig;
    QCOMPARE(configWithReadConfig.str, QString("test"));
    QVERIFY(configWithReadConfig.triedToExit);
    readStr();

    AVConfigBase::params() << "-str" << "override";
    ConfigWithParseParams configWithParseParams;
    QCOMPARE(configWithParseParams.str, QString("override"));
    QVERIFY(configWithParseParams.triedToExit);
    readStr("override");
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testConfigList()
{
    writeStr();
    AVConfigList<Config> list;
    list.readAll();
    QCOMPARE(list.getParaSetList().join(","),QString("a,b,default"));

    Config* config = list.switchToParaSet("default");
    QVERIFY(config != 0);
    QCOMPARE(config->str, QString("test"));

    config = list.switchToParaSet("a");
    QVERIFY(config != 0);
    QCOMPARE(config->str, QString("aaa"));

    config = list.switchToParaSet("b");
    QVERIFY(config != 0);
    QCOMPARE(config->str, QString("bbb"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testConfigListWithOptionSave()
{
    AVConfigBase::option_save = true;
    writeStr();
    AVConfigList<Config> list;
    list.readAll();
    readStr();
    QCOMPARE(list.getParaSetList().join(","),QString("a,b,default"));

    Config* config = list.switchToParaSet("a");
    QVERIFY(config != 0);
    QCOMPARE(config->str, QString("aaa"));
    QVERIFY(!config->triedToExit);

    config = list.switchToParaSet("b");
    QVERIFY(config != 0);
    QCOMPARE(config->str, QString("bbb"));
    QVERIFY(!config->triedToExit);

    config = list.switchToParaSet("default");
    QVERIFY(config != 0);
    QCOMPARE(config->str, QString("test"));
    QVERIFY(!config->triedToExit);

    config->str = "override";
    list.writeAll();
    readStr("override");
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfigTest::filename(const QString& configfile)
{
    QString path = AVEnvironment::getApplicationConfig();
    if (path.isEmpty()) {
        path = QDir::homePath();
        if (path.isEmpty()) path = ".";
    }

    return path + "/" + configfile;
}

///////////////////////////////////////////////////////////////////////////////

QString AVConfigTest::contents(const QString& str, const QString& astr,
                               const QString& bstr)
{
    return
        "[a]\n"
        "str,a string\n"
        "str = 133,\"" + astr + "\",1024,\"default\",helpText:\n"
        "[b]\n"
        "str,a string\n"
        "str = 133,\"" + bstr + "\",1024,\"default\",helpText:\n"
        "[default]\n"
        "str,a string\n"
        "str = 133,\"" + str + "\",1024,\"default\",helpText:\n";
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::writeStr(const QString& str, const QString& configfile)
{
    writeConfig(contents(str), configfile);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::readStr(const QString& str, const QString& configfile)
{
    readConfig(contents(str), configfile);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::writeConfig(const QString& contents, const QString& configfile)
{
    QString filenamestring = AVConfigTest::filename(configfile);

    QVERIFY(!filenamestring.startsWith("#"));

    QFile file(filenamestring);

    file.remove();
    QVERIFY(!file.exists());

    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << contents;
    file.close();

    QVERIFY(file.exists());
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::readConfig(const QString& expected, const QString& configfile)
{
    QString filename = AVConfigTest::filename(configfile);

    QVERIFY(!filename.startsWith("#"));

    QFile file(filename);

    QVERIFY(file.exists());

    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream stream(&file);
    QString contents = stream.readAll();
    file.close();
    QCOMPARE(contents, expected);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfigTest::testStandardPathsAreAbsolute()
{
    QString path = AVEnvironment::getApplicationHome();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());

    path = AVEnvironment::getApplicationConfig();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());

    path = AVEnvironment::getApplicationLog();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());

    path = AVEnvironment::getApplicationRun();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());

    path = AVEnvironment::getApplicationData();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());

    path = AVEnvironment::getApplicationCache();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());

    path = AVEnvironment::getApplicationMaps();
    QVERIFY(!path.isEmpty());
    QVERIFY(!QDir(path).isRelative());
}

///////////////////////////////////////////////////////////////////////////////
AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVConfigTest,"avlib/unittests/config")
#include "avconfigtest.moc"


// End of file
