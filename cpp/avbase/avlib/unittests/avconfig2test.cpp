///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH,2001-2010
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Tests the AVConfig2 functionality.
*/

#include "avconfig2test.h"
#include <functional>

// Qt includes
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>

#include "avconfig.h" // TODO QTP-10
#include "avconfig2exporterfactory.h"
#include "avconfig2saver.h"
#include "avdaemon.h"
#include "avlog.h"
#include "avmisc.h"
#include "avconfig2cstyle.h"

#include <avsingleton.h>

///////////////////////////////////////////////////////////////////////////////

QString fileContent(const QString& path)
{
    QString ans;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return ans;

    QTextStream in(&file);
    while (!in.atEnd())
        ans += in.readLine() + "\n";
    return ans;
}

QString AVConfig2Test::getTempConfigFileCopy(const QString &name)
{
    AVConfig2Container config;

    AVConfig2StorageLocation location = config.locateConfig(name, true);
    QString original_file_location = location.getFullFilePath();
    QString copy_file_location = m_temp_dir->path() + "/" + name + ".cc";

    AVASSERT(QFile::copy(original_file_location, copy_file_location));
    return copy_file_location;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::verifyEqualFileContent(const QString &actual, const QString &expected)
{
    QFile file1(AVConfig2Global::singleton().locateConfig(actual, true).getFullFilePath());
    QFile file2(AVConfig2Global::singleton().locateConfig(expected, true).getFullFilePath());

    QVERIFY(file1.open(QIODevice::ReadOnly));
    QVERIFY(file2.open(QIODevice::ReadOnly));

    QByteArray content1 = file1.readAll();
    QByteArray content2 = file2.readAll();

    // missing QString constructor -> use STL instead...
    QCOMPARE(std::string(content1.data(), content1.count()),
             std::string(content2.data(), content2.count()));
}

///////////////////////////////////////////////////////////////////////////////

bool AVConfig2Test::checkedRefresh(AVConfig2Container& container)
{
    AVConfig2Container::RefreshResult ret = container.refreshAllParams();
    if (ret.refreshSuccessful()) return true;
    AVLogError << "AVConfig2Test::checkedRefresh:\n" << ret.getErrorMessage();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::init()
{
    m_temp_dir = new QTemporaryDir();
    m_temp_dir->setAutoRemove(true);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::cleanup()
{
    delete m_temp_dir;
    m_temp_dir = nullptr;
    AVLog::getRootLogger()->setFatalSegfault(false);
    AVLog::getRootLogger()->setFatalExit(false);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testNestedConfigs()
{
    AVConfig2Container config;
    TestParamImporter importer(config);
    importer.import();

    TestConfig test_config(config);

    QVERIFY(checkedRefresh(config));

    QVERIFY(test_config.m_sub_cfg.size() == 3);
    QVERIFY(test_config.m_sub_cfg.contains("section1"));
    QVERIFY(test_config.m_sub_cfg.contains("section2"));
    QVERIFY(test_config.m_sub_cfg.contains("section3"));

    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg.size() == 2);
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg.contains("first"));
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg.contains("second"));
    QVERIFY(test_config.m_sub_cfg["section1"]->getConfigMapKey() == "section1");

    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg["first"]->m_double == 1.1);
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg["first"]->m_pure_cmdline == "pure_default");
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg["second"]->m_double == 2.2);
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg["second"]->getConfigMapKey() == "second");
    QVERIFY(test_config.m_sub_cfg["section1"]->m_string == "s1");
    QVERIFY(test_config.m_sub_cfg["section1"]->m_int == 1);

    QVERIFY(test_config.m_sub_cfg["section2"]->m_string == "s2");
    QVERIFY(test_config.m_sub_cfg["section2"]->m_int == 2);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCmdlineSwitch()
{
    // Check that the config complains if a command line switch is used for multiple params
    AVConfig2Container config;
    TestParamImporter importer(config);
    importer.import();

    TestConfig        test_config1(config);
    CmdlineTestConfig test_config2(config);

    AVConfig2Container::RefreshResult res = config.refreshAllParams();
    config.checkCmdlineParams(res.m_cmdline_errors);

    QVERIFY(!res.refreshSuccessful());
    if (res.m_errors.size() != 1           ||
        !res.m_metadata_mismatch.isEmpty() ||
        !res.m_missing_parameters.isEmpty())
    {
        AVLogger->Write(LOG_ERROR,"%s", qPrintable(res.getErrorMessage()));
    }
    QVERIFY(res.m_metadata_mismatch.isEmpty());
    QVERIFY(res.m_missing_parameters.isEmpty());
    QVERIFY(res.m_errors.size() == 0);
    QVERIFY(res.m_cmdline_errors.size() == 1);
    QVERIFY(res.m_cmdline_errors[0].contains("multiple times"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCmdlineSwitchExtraParams_data()
{
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("extra_cmd_line");

    QTest::newRow("row1") << (QStringList() << "-A" << "A" << "--B" << "B" << "-")
                          << (QStringList() << "-");
    QTest::newRow("row2") << (QStringList() << "-")
                          << (QStringList() << "-");
    QTest::newRow("row3") << (QStringList() << "--")
                          << (QStringList());
    QTest::newRow("row4") << (QStringList() << "-an" << "DIFLIS" << "a:b")
                          << (QStringList() << "a:b");
    QTest::newRow("row5") << (QStringList() << "--" << "-")
                          << (QStringList() << "-");
    QTest::newRow("row6") << (QStringList() << "--" << "--")
                          << (QStringList() << "--");
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCmdlineSwitchExtraParams()
{
    AVLOG_ENTER_TESTCASE;

    QFETCH(QStringList, args);
    QFETCH(QStringList, extra_cmd_line);

    AVConfig2Container config;
    config.parseCmdlineParams(args, true);

    QCOMPARE(config.getExtraCmdline(), extra_cmd_line);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testReference()
{
    QStringList args = QStringList()
            << "-test_config.nested_prefix.section3.nested_sub_prefix.first.double" << "3.4"
            << "-test_config.nested_prefix.section1.nested_sub_prefix.second.double" <<  "5.6";

    AVConfig2Container original_config;
    original_config.parseCmdlineParams(args, false);
    TestParamImporter importer(original_config);
    importer.import();

    AVConfig2Container config;
    original_config.assignDataTo(config); // test assignment while we're at it...

    TestConfig test_config(config);

    AVConfig2Container::RefreshResult res = config.refreshAllParams();
    config.checkCmdlineParams(res.m_cmdline_errors);
    if (!res.refreshSuccessful())
    {
        AVLogInfo << res.getErrorMessage();
    }
    QVERIFY(res.refreshSuccessful());

    QVERIFY(test_config.m_sub_cfg.contains("section1"));
    QVERIFY(test_config.m_sub_cfg.contains("section2"));
    QVERIFY(test_config.m_sub_cfg.contains("section3"));

    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg.contains("first"));
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg.contains("second"));

    QVERIFY(test_config.m_sub_cfg["section3"]->m_sub_cfg.contains("first"));
    QVERIFY(test_config.m_sub_cfg["section3"]->m_sub_cfg.contains("second"));

    QCOMPARE(test_config.m_sub_cfg["section3"]->m_sub_cfg["first"]->m_double, 3.4);

    QCOMPARE(test_config.m_sub_cfg["section1"]->m_sub_cfg["second"]->m_double, 5.6);
    // Is loaded as reference to the param checked in the previous line, so must be equal.
    QCOMPARE(test_config.m_sub_cfg["section3"]->m_sub_cfg["second"]->m_double, 5.6);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testReference2()
{
    // reference to optional param (not yet registered), reference overridden by command line
    {
        QStringList args = QStringList() << "-port" << "2";

        AVConfig2Container container;
        container.parseCmdlineParams(args, false);
        AVConfig2Container::RefreshResult res;

        TestReference2ParamImporter importer(container);
        importer.import();

        QVERIFY(checkedRefresh(container));

        CmdlineTestConfig test_config(container);

        res = container.refreshAllParams();
        container.checkCmdlineParams(res.m_cmdline_errors);
        if (!res.refreshSuccessful())
        {
            AVLogInfo << res.getErrorMessage();
        }
        QVERIFY(res.refreshSuccessful());

        QCOMPARE(test_config.m_port, 2u);
    }

    // reference to freestanding param specified on command line
    {
        QStringList args = QStringList() << "--free.standing" << "321";

        AVConfig2Container container;
        container.parseCmdlineParams(args, false);
        TestReference2ParamImporter importer(container);
        importer.import();

        CmdlineTestConfig test_config(container);

        AVConfig2Container::RefreshResult res = container.refreshAllParams();
        container.checkCmdlineParams(res.m_cmdline_errors);
        if (!res.refreshSuccessful())
        {
            AVLogInfo << res.getErrorMessage();
        }
        QVERIFY(res.refreshSuccessful());

        QCOMPARE(test_config.m_port, 321u);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCommandLineOverride()
{
    // Check if negative values can be given for override parameters in command line
    QStringList args = QStringList() << "--test_config.nested_prefix.section1.nested_sub_prefix.first.double" << "-1.0";

    AVConfig2Container config;
    config.parseCmdlineParams(args, false);
    TestParamImporter importer(config);
    importer.import();

    TestConfig test_config(config);

    AVConfig2Container::RefreshResult res = config.refreshAllParams();
    config.checkCmdlineParams(res.m_cmdline_errors);
    if (!res.refreshSuccessful())
    {
        AVLogError << res.getErrorMessage();
    }
    QVERIFY(res.refreshSuccessful());

    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg["first"]->m_double == -1.0);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testAggregateReference()
{
    AVLOG_ENTER_METHOD();

    // Test escaping
    {
        AVConfig2Container config;

        {
            AVConfig2Metadata metadata("test_config.port", AVConfig2Types::getTypeName<uint>(), DEFAULT_HELP);
            metadata.m_command_line_switch = "port";
            config.addParameter("123", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
        }
        {
            AVConfig2Metadata metadata("test_config.test_string", AVConfig2Types::getTypeName<QString>(), DEFAULT_HELP);
            QString refstring = AVToString(QString("\"\\\"$(test_config.port)\\\" is "), true);
            config.addReference("[" + refstring + "; $(test_config.port); \"\\\"\"]", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
        }

        TestConfig test_config(config);

        AVConfig2Container::RefreshResult res = config.refreshAllParams();
        config.checkCmdlineParams(res.m_cmdline_errors);
        if (!res.refreshSuccessful())
        {
            AVLogError << res.getErrorMessage();
        }
        QVERIFY(res.refreshSuccessful());

        QCOMPARE(test_config.m_test_string, QString("\"$(test_config.port)\" is 123"));
        QCOMPARE(test_config.m_port, 123u);
    }

    // Test quoted strings used in aggregate references. See SWE-4315
    // Using the port is not really realistic.
    {
        AVConfig2Container config;

        {
            AVConfig2Metadata metadata("test_config.test_string", AVConfig2Types::getTypeName<QString>(), DEFAULT_HELP);
            config.addParameter("\"123\"", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
        }
        {
            AVConfig2Metadata metadata("test_config.port", AVConfig2Types::getTypeName<uint>(), DEFAULT_HELP);
            metadata.m_command_line_switch = "port";
            config.addReference("[$(test_config.test_string)]", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
        }

        TestConfig test_config(config);

        AVConfig2Container::RefreshResult res = config.refreshAllParams();
        config.checkCmdlineParams(res.m_cmdline_errors);
        if (!res.refreshSuccessful())
        {
            AVLogError << res.getErrorMessage();
        }
        QVERIFY(res.refreshSuccessful());

        QCOMPARE(test_config.m_test_string, QString("123"));
        QCOMPARE(test_config.m_port, 123u);
    }
}
///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSimpleSaver()
{
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());

    AVConfig2StorageLocation location;
    location.m_location = temp_dir.path();
    location.m_format = CF_CSTYLE;
    location.m_name = "test1";

    location.pushCondition(AVConfig2StorageLocation::FactCondition(0, QString("A"), QStringList() << "B" << "C"));
    location.m_source = AVConfig2StorageLocation::PS_FILE_ORDINARY;

    AVConfig2StorageLocation location2 = location;
    location2.pushCondition(AVConfig2StorageLocation::FactCondition(0, QString("X"), QStringList() << "Y" << "Z"));

    AVConfig2Metadata metadata;
    metadata.m_type = "int";

    // TODO uncomment different fact conditions once those are handled correctly

    AVConfig2LoadedData saved_data(false);
    saved_data.addDefine("name", "value", location);
    saved_data.addOverride("override", "to", location);
    saved_data.addConfigMapping("map_from", "dir", "to", location);
//    saved_data.addConfigMapping("xfrom", "dir", "xto", location2);
    saved_data.addConfigMapping("from2", "temp_dir", "to2", location);
    saved_data.addSearchPath("path", location);
    saved_data.addInclude("test_include1", location);
    saved_data.addInclude("test_include2", location);

    metadata.m_name = "test.param_name";
    saved_data.addParameter("value", metadata, location);
    metadata.m_name = "test.ref_name";
    saved_data.addReference("ref", metadata, location);
//    metadata.m_name = "test.param_name2";
//    saved_data.addParameter("value", metadata, location2);
    metadata.m_name = "test.param_name3";
    saved_data.addParameter("value", metadata, location);

    AVConfig2SimpleSaver saver;
    saver.save(saved_data);

    QFileInfo saved_file(temp_dir.path() + "/" + "test1.cc");
    QVERIFY(saved_file.exists());

    QFile file(saved_file.filePath());
    file.open(QIODevice::ReadOnly);
    AVLogDebug << file.readAll();

    AVConfig2LoadedData loaded_data(false);
    QVERIFY(AVConfig2ImporterBase::import(&loaded_data, saved_file.filePath()));

//    AVLogInfo << loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedMapping>(0).m_location;

    QVERIFY(saved_data == loaded_data);

    // test that preloading works...
    saved_data.removeElementByIndex<AVConfig2ImporterClient::LoadedInclude>(0);
    QVERIFY(saved_data.getElementCount<AVConfig2ImporterClient::LoadedInclude>() == 1);
    saver.setUnknownParameterPolicy(AVConfig2SimpleSaver::UPP_KEEP);
    saver.save(saved_data);
    AVConfig2LoadedData loaded_data2(false);
    QVERIFY(AVConfig2ImporterBase::import(&loaded_data2, saved_file.filePath()));
    QVERIFY(loaded_data == loaded_data2);

    // Now test that discarding of elements works as well...
    saver.setUnknownParameterPolicy(AVConfig2SimpleSaver::UPP_DISCARD);
    saver.save(saved_data);

    AVConfig2LoadedData loaded_data3(false);
    QVERIFY(AVConfig2ImporterBase::import(&loaded_data3, saved_file.filePath()));
    QVERIFY(saved_data == loaded_data3);

    // Test that removal of all elements works (reproduces CMS-30)
    AVConfig2LoadedData empty_saved_data(false);
    empty_saved_data.addInclude("test_include1", location);
    empty_saved_data.removeElementByIndex<AVConfig2LoadedData::LoadedInclude>(0);
    saver.save(empty_saved_data);
    AVConfig2LoadedData loaded_data4(false);
    QVERIFY(AVConfig2ImporterBase::import(&loaded_data4, saved_file.filePath()));
    QVERIFY(empty_saved_data == loaded_data4);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigSave()
{
    AVConfig2Container container;
    TestConfig test_config(container);

    QSharedPointer<NestedConfig> nested_config(new NestedConfig("test_config.nested_config", container));
    nested_config->m_int = 1;
    nested_config->m_string = "test_string_value";

    test_config.m_sub_cfg["sub"] = nested_config;

    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());
    AVConfig2StorageLocation location;
    location.m_location = temp_dir.path();
    location.m_name = "test_cfg_file";
    location.m_format = CF_CSTYLE;
    container.setStorageLocation(location);

    // store one other param to a different file
    location.m_name = "test2_cfg_file";
    container.setStorageLocation(location, QRegExp("test_config.nested_config.str.*"));

    AVConfig2Saver saver;
    saver.save(container);

    AVConfig2LoadedData loaded_data(false);
    AVConfig2ImporterBase::import(&loaded_data, location);

    QVERIFY(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>() == 1);
    AVConfig2ImporterClient::LoadedParameter param =
            loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(0);
    QVERIFY(param.getValue() == "test_string_value");
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testMultipleInstances()
{
    // First, make sure creating two instances of a config doesn't crash the test...
    AVConfig2Container container;
    TestConfig test_config1(container);
    TestConfig test_config2(container);

    test_config1.m_test_string = "changed1";
    test_config2.m_test_string = "changed2";

    // Now save back the config, verify that the change from the first config is adopted.
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());
    AVConfig2StorageLocation location;
    location.m_location = temp_dir.path();
    location.m_name = "testMultipleInstances_test";
    location.m_format = CF_CSTYLE;
    container.setStorageLocation(location);

    AVConfig2Saver saver;
    saver.save(container);

    AVConfig2Container loaded_container;
    QVERIFY(loaded_container.loadConfigAbsolute(location.getFullFilePath()));
    TestConfig loaded_test_config1(loaded_container);
    TestConfig loaded_test_config2(loaded_container);

    QVERIFY(checkedRefresh(loaded_container));
    AVLogInfo << loaded_test_config1.m_test_string;
    AVLogInfo << loaded_test_config2.m_test_string;
    QVERIFY(loaded_test_config1.m_test_string == "changed1");
    QVERIFY(loaded_test_config2.m_test_string == "changed1");
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testConfigSave()
{
    QString filename = "config_save_test";
    QString path = getTempConfigFileCopy(filename);
    AVLogInfo << "configs copy content:\n" << fileContent(path);

    AVConfig2LoadedData loaded_data(false);
    QVERIFY(loaded_data.load(path));

    QVERIFY(QFile(path).remove());

    AVConfig2SimpleSaver saver;
    saver.save(loaded_data);

    AVLogInfo << "saved config:\n" << fileContent(path);

    AVConfig2LoadedData another_loaded_data(false);
    QVERIFY(another_loaded_data.load(path));

    QVERIFY(loaded_data == another_loaded_data);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testDeprecatedNames()
{
    AVConfig2Container container;
    DeprecatedTestConfig config(container);

    QStringList p1_names = container.getRegisteredParameterInfo("deprecated_test_config.some_port").second->getDeprecatedNames();
    AVLogInfo << "AVConfig2Test::testDeprecatedNames: 1st param, got " <<p1_names.count()<< " entries: "<< p1_names.join(",");
    QVERIFY(p1_names.count() == 2);
    QVERIFY(p1_names.contains("deprecated_test_config.SomePort"));
    QVERIFY(p1_names.contains("deprecated_test_config.SomeOldPort"));

    QStringList p2_names = container.getRegisteredParameterInfo("deprecated_test_config.some_other_port").second->getDeprecatedNames();
    AVLogInfo << "AVConfig2Test::testDeprecatedNames: 2nd param, got " <<p2_names.count()<< " entries: "<< p2_names.join(",");
    QVERIFY(p2_names.count() == 3);
    QVERIFY(p2_names.contains("deprecated_test_config.SomeOtherPort"));
    QVERIFY(p2_names.contains("DeprecatedTestConfig.SomeOtherPort"));
    QVERIFY(p2_names.contains("DeprecatedTestConfig.some_other_port"));

    QStringList p3_names = container.getRegisteredParameterInfo("deprecated_test_config.third_port").second->getDeprecatedNames();
    AVLogInfo << "AVConfig2Test::testDeprecatedNames: 3rd param, got " <<p3_names.count()<< " entries: "<< p3_names.join(",");
    QVERIFY(p3_names.count() == 1);
    QVERIFY(p3_names.contains("DeprecatedTestConfig.third_port"));

    QStringList p4_names = container.getRegisteredParameterInfo("deprecated_test_config.no_precated_port").second->getDeprecatedNames();
    AVLogInfo << "AVConfig2Test::testDeprecatedNames: 4th param, got " <<p4_names.count()<< " entries: "<< p4_names.join(",");
    QVERIFY(p4_names.isEmpty());

    // 1st loading step - load a config file containing non-deprecated names

    AVConfig2StorageLocation location;
    location.m_location = AVEnvironment::getApplicationConfig();
    location.m_name = "deprecated_test_config1";
    location.m_format = CF_CSTYLE;

    AVConfig2Container container1;
    container1.setStorageLocation(location);

    QVERIFY(container1.loadConfigAbsolute(location.getFullFilePath()));
    DeprecatedTestConfig config1(container1);
    QVERIFY(checkedRefresh(container1));

    QVERIFY(config1.m_port1 == 1);
    QVERIFY(config1.m_port2 == 2);
    QVERIFY(config1.m_port3 == 3);
    QVERIFY(config1.m_port4 == 4);

    // 2nd loading step - load a config file containing deprecated names (1st deprecated variant)

    location.m_name = "deprecated_test_config2";

    AVConfig2Container container2;
    container2.setStorageLocation(location);

    QVERIFY(container2.loadConfigAbsolute(location.getFullFilePath()));
    DeprecatedTestConfig config2(container2);
    QVERIFY(checkedRefresh(container2));

    QVERIFY(config2.m_port1 == 5);
    QVERIFY(config2.m_port2 == 6);
    QVERIFY(config2.m_port3 == 7);
    QVERIFY(config2.m_port4 == 8);

    // 3rd loading step - load a config file containing deprecated names (2nd deprecated variant)

    location.m_name = "deprecated_test_config3";

    AVConfig2Container container3;
    container3.setStorageLocation(location);

    QVERIFY(container3.loadConfigAbsolute(location.getFullFilePath()));
    DeprecatedTestConfig config3(container3);
    QVERIFY(checkedRefresh(container3));

    QVERIFY(config3.m_port1 == 9);
    QVERIFY(config3.m_port2 == 10);
    QVERIFY(config3.m_port3 == 11);
    QVERIFY(config3.m_port4 == 12);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testOptionalParamWithoutFile()
{
    QStringList args = QStringList() << "-optional" << "cmdline_value";
    AVConfig2Container container;
    container.parseCmdlineParams(args, false);

    OptionalParamTestImporter importer(container);
    importer.import();

    OptionalCmdlineTestConfig config(container);

    QVERIFY(checkedRefresh(container));

    QCOMPARE(config.m_optional_param,              QString("cmdline_value"));
    QCOMPARE(config.m_optional_param_dev_override, QString("dev_override_value"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testOptionalRegistrationFirst()
{
    {
        // load optional parameter from file after registration
        AVConfig2Container container;
        OptionalCmdlineTestConfig config(container);

        AVConfig2Metadata metadata("test_config.optional_param", "QString", DEFAULT_HELP);
        metadata.m_optional = true;
        metadata.m_default_value = "default_value";
        metadata.m_command_line_switch = "optional";
        container.addParameter("file_value", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));

        QVERIFY(checkedRefresh(container));

        QCOMPARE(config.m_optional_param, QString("file_value"));
    }

    {
        // dev override for optional parameter after registration
        AVConfig2Container container;
        OptionalCmdlineTestConfig config(container);

        container.addOverride(
                    "test_config.optional_param", "dev_override_value", AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_DEV_OVERRIDE));

        QVERIFY(checkedRefresh(container));

        QCOMPARE(config.m_optional_param, QString("dev_override_value"));
    }

    {
        // reference for optional parameter after registration
        AVConfig2Container container;
        OptionalCmdlineTestConfig config(container);

        {
            // add referenced value
            AVConfig2Metadata metadata("test_config.referenced_value", "QString", DEFAULT_HELP);
            container.addParameter("referenced_value", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
        }
        {
            // add referencing value
            AVConfig2Metadata metadata("test_config.optional_param", "QString", DEFAULT_HELP);
            metadata.m_optional = true;
            metadata.m_default_value = "default_value";
            metadata.m_command_line_switch = "optional";
            container.addReference("test_config.referenced_value", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
        }

        QVERIFY(checkedRefresh(container));

        QCOMPARE(config.m_optional_param, QString("referenced_value"));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testOptionalDeprecatedParameters()
{
    AVLOG_ENTER_METHOD();

    const char * TEST_FILE_NAME = "deprecated_optional";

    QString file_copy = getTempConfigFileCopy(TEST_FILE_NAME);

    class LocalTestConfig : public AVConfig2
    {
    public:
        LocalTestConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container),
            m_param(0)
        {
            registerParameter("new_name", &m_param, DEFAULT_HELP).
                    setOptional(1).
                    setDeprecatedName("old_name");
        }
        uint m_param;
    };

    AVConfig2Container container;
    LocalTestConfig config("prefix", container);

    {
        // Part#1: load deprecated optional parameter, ensure that translation works
        QVERIFY(container.loadConfigAbsolute(file_copy));
        QVERIFY(checkedRefresh(container));

        QCOMPARE(config.m_param, 2u);
    }

    {
        AVLOG_ENTER(save filtered);

        // Part#2: verify that there is no crash if the target file is filtered (SWE-4838)
        AVConfig2Saver saver;
        saver.setFilenameSaveFilter(QRegularExpression("any other"));
        saver.save(container);
    }

    {
        AVLOG_ENTER(ordinary save);

        // Part#3: save it, and ensure that only the new name makes it into the config file
        AVConfig2Saver saver;
        saver.save(container);

        QFile file(file_copy);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QByteArray raw_content = file.readAll();
        QVERIFY(!raw_content.isEmpty());
        raw_content[raw_content.count()-1] = 0; // replace final newline with 0 for subsequent string conversion - lazy and ugly.

        QString content(raw_content);
        AVLogInfo << content;
        QVERIFY(!content.contains("old_name"));
        QVERIFY(content.contains("new_name"));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testOptionalChangedInApplication()
{
    AVLOG_ENTER_METHOD();

    class LocalTestConfig : public AVConfig2
    {
    public:
        LocalTestConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container),
            m_opt_param(0),
            m_unchanged_opt_param(0),
            m_sug_param(0)
        {
            registerParameter("opt_param", &m_opt_param, DEFAULT_HELP)
                    .setOptional(1);
            registerParameter("unchanged_opt_param", &m_unchanged_opt_param, DEFAULT_HELP)
                    .setOptional(1);
            registerParameter("sug_param", &m_sug_param, DEFAULT_HELP)
                    .setSuggestedValue(2);
        }
        uint m_opt_param;
        uint m_unchanged_opt_param;
        uint m_sug_param;
    };

    AVConfig2Container container;

    // provide any parameter for the heuristic to find the correct file name
    // (since we don't call setStorageLocation)
    AVConfig2Metadata metadata("top_level.any", "int", DEFAULT_HELP);
    AVConfig2StorageLocation location(m_temp_dir->path(), "test", CF_CSTYLE);
    container.addParameter("0", metadata, location);

    LocalTestConfig config("top_level.nested_prefix", container);

    // setOptional() immediately adopts the default value
    QCOMPARE(config.m_unchanged_opt_param, 1u);
    config.m_opt_param = 20;
    config.m_sug_param = 21;

    AVConfig2Saver saver;
    saver.save(container);

    // now check the content of the saved file
    {
        AVConfig2LoadedData data(true);
        QVERIFY(data.load(location.getFullFilePath()));

        // make sure that the unchanged optional value was not saved out
        QCOMPARE(data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 2u);

        AVConfig2ImporterClient::LoadedParameter *sug_param =
                data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("top_level.nested_prefix.sug_param");
        QVERIFY(sug_param != nullptr);
        QCOMPARE(sug_param->getValue(), QString("21"));

        AVConfig2ImporterClient::LoadedParameter *opt_param =
                data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("top_level.nested_prefix.opt_param");
        QVERIFY(opt_param != nullptr);
        QCOMPARE(opt_param->getValue(), QString("20"));
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testPureCmdlineSwitch()
{
    QStringList args = QStringList() << "-test_config.nested_prefix.section1.nested_sub_prefix.first.pure_cmdline"
                                     << "pure_overridden";

    AVConfig2Container original_container;
    original_container.parseCmdlineParams(args, false);

    TestParamImporter importer(original_container);
    importer.import();

    AVConfig2Container container;
    original_container.assignDataTo(container); // test assignment while we're at it...

    TestConfig test_config(container);

    QVERIFY(checkedRefresh(container));

    QVERIFY(test_config.m_sub_cfg.size() == 3);
    QVERIFY(test_config.m_sub_cfg.contains("section1"));

    QCOMPARE(test_config.m_sub_cfg["section1"]->m_sub_cfg.size(), 2);
    QVERIFY(test_config.m_sub_cfg["section1"]->m_sub_cfg.contains("first"));

    QCOMPARE(test_config.m_sub_cfg["section1"]->m_sub_cfg["first"]->m_pure_cmdline, QString("pure_overridden"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testChangeSuggestedValues()
{
    AVConfig2Container container;
    OptionalCmdlineTestConfig test_config(container);

    test_config.changeSuggestedValues();

    QVERIFY(checkedRefresh(container));

    QCOMPARE(test_config.m_optional_param,              QString("CHANGED_DEFAULT1"));
    QCOMPARE(test_config.m_optional_param_dev_override, QString("CHANGED_DEFAULT2"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testEmptyDefines()
{
    QString filename = "empty_defines_test";
    QString path = getTempConfigFileCopy(filename);

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));
    QVERIFY(container.m_loaded_duplicates.isEmpty());

    auto keys = container.getLoadedParameters().keys();
    QVERIFY( keys.contains("testNamespace.emptyIsEmpty"));
    QVERIFY(!keys.contains("testNamespace.emptyIsNotEmpty"));

    QVERIFY(!keys.contains("testNamespace.notEmptyIsEmpty"));
    QVERIFY( keys.contains("testNamespace.notEmptyisNotEmpty"));

    AVConfig2Saver saver;
    saver.setSaveMode(AVConfig2Saver::SM_ALL_LOADED);
    saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
    saver.save(container);

    // to insure that saved correctly
    AVConfig2Container container2;
    QVERIFY(container2.loadConfigAbsolute(path));
    QVERIFY(container2.m_loaded_duplicates.isEmpty());

    QVERIFY(container2.getLoadedParameters().keys() == keys);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testNestedLevels()
{
    QString filename = "test_nested_levels";
    QString path = getTempConfigFileCopy(filename);

    AVConfig2Container container;
    container.loadConfigAbsolute(path);

    const AVConfig2LoadedData& loaded_data = container.getLoadedData();
    QVERIFY(loaded_data.m_loaded_locations.size() != 0);

    for (uint i = 0; i< loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(); ++i)
    {
        const AVConfig2ImporterClient::LoadedParameter& cur_loaded = loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(i);
        auto conditions = cur_loaded.getLocation().getFactConditions();
        for(AVConfig2StorageLocation::FactCondition condition : conditions)
        {
            QString level_pattern = QStringLiteral("LEVEL_");
            QString condition_namespace_level = QString::number(condition.getLevel());
            QString condition_name_part = level_pattern + condition_namespace_level;

            QVERIFY(condition.getFactName().contains(condition_name_part));
            AVLogInfo << Q_FUNC_INFO << "\t" << condition.getFactName() << "\t" << condition.getLevel();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testReferenceToken()
{
    AVConfig2Container container;
    QVERIFY(container.loadConfig("references_test"));
    QVERIFY(container.m_loaded_duplicates.isEmpty());

    const AVConfig2ImporterClient::LoadedReference *param = container.getLoadedData().getElementByName<AVConfig2ImporterClient::LoadedReference>("references.port_1");
    QCOMPARE(param->getName(), QString("references.port_1"));
    QCOMPARE(param->m_metadata.m_type, QString("uint"));
    QCOMPARE(param->m_referenced_param, QString("1"));

    const AVConfig2ImporterClient::LoadedReference *param2 = container.getLoadedData().getElementByName<AVConfig2ImporterClient::LoadedReference>("references.port_2");
    QCOMPARE(param2->getName(), QString("references.port_2"));
    QCOMPARE(param2->m_metadata.m_type, QString("uint"));
    QCOMPARE(param2->m_referenced_param, QString("2"));

    const AVConfig2ImporterClient::LoadedReference *param3 = container.getLoadedData().getElementByName<AVConfig2ImporterClient::LoadedReference>("references.port_3");
    QCOMPARE(param3->getName(), QString("references.port_3"));
    QCOMPARE(param3->m_metadata.m_type, QString("uint"));
    QCOMPARE(param3->m_referenced_param, QString("3"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCommandlineWithMissingParam()
{
    AVLOG_ENTER_METHOD();

    class LocalTestConfig : public AVConfig2
    {
    public:
        LocalTestConfig(AVConfig2Container& container) :
            AVConfig2("prefix", container)
        {
            registerParameter("port", &m_port, DEFAULT_HELP)
                    .setCmdlineSwitch("port")
                    .setSuggestedValue(1);
        }
        uint m_port;
    };

    AVConfig2Container::CheckingMode checking_mode[2] = {AVConfig2Container::CheckingMode::CM_STRICT, AVConfig2Container::CheckingMode::CM_LENIENT};

    for (AVConfig2Container::CheckingMode cur_mode : checking_mode)
    {
        AVConfig2Container container;
        container.setCheckingMode(cur_mode);
        container.parseCmdlineParams(QStringList() << "-port" << "1234", false);
        LocalTestConfig config(container);

        AVConfig2Container::RefreshResult result = container.refreshAllParams();

        if (cur_mode == AVConfig2Container::CheckingMode::CM_STRICT)
        {
            QVERIFY(result.m_missing_parameters.count() == 1);
        } else
        {
            QVERIFY(result.m_missing_parameters.isEmpty());
            QCOMPARE(config.m_port, 1234u);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testEnumConfig()
{
    AVConfig2Container container;
    EnumTestConfig enum_config(container);

    auto& parameter_list = container.m_registered_param.getParameterList(&enum_config);
    for (const auto& parameter_base_ptr : parameter_list)
    {
        if (parameter_base_ptr->pointsToVariable(&enum_config.m_avenum_value)) {
            auto& metadata = parameter_base_ptr->getMetadata();
            auto* listRestriction = dynamic_cast<const AVConfig2ValueListRestriction<EnumTestConfig::ValueAVEnum>*>(metadata.m_restriction.get());
            QVERIFY(listRestriction);
            QList<EnumTestConfig::ValueAVEnum> values;
            listRestriction->getValidValues(values);

            QList<EnumTestConfig::ValueAVEnum> expected_values {
                EnumTestConfig::ValueAVEnum::TestValue1,
                EnumTestConfig::ValueAVEnum::TestValue2
            };

            QCOMPARE(values, expected_values);
        }
    }

    container.loadConfig("enum_test_config");

    AVConfig2Container::RefreshResult result;
    QVERIFY2(enum_config.refreshParams(&result), result.getErrorMessage().toUtf8());

    QCOMPARE(enum_config.m_qenum_value, EnumTestConfig::ValueQEnum::Value2);
    QCOMPARE(enum_config.m_avenum_value, EnumTestConfig::ValueAVEnum::TestValue2);
    QCOMPARE(enum_config.m_namespace_avenum_value, avconfig2_enum_test_namespace::TestEnum::E2);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testGetTypeName()
{
    QCOMPARE(AVConfig2Types::getTypeName<QVector<QString>>(), QString("QVector<QString >"));
    QString map_type = AVConfig2Types::getTypeName<QMap<QString, QString>>();
    QCOMPARE(map_type, QString("QMap<QString, QString >"));

    QString hash_name = AVConfig2Types::getTypeName<QHash<QString, quint16> >();
    QCOMPARE(hash_name, QString("QHash<QString, quint16 >"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testUtf8Encoding()
{
    AVLOG_ENTER_METHOD();

    QByteArray arr(3, 0);
    arr[0] = 0xe2;
    arr[1] = 0x98;
    arr[2] = 0x95;
    QString special_char = "have some " + QString::fromUtf8(arr);

    AVLogInfo << special_char;

    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());

    AVConfig2StorageLocation location(AVConfig2StorageLocation::PS_FILE_ORDINARY);
    location.m_location = temp_dir.path();
    location.m_name = "test_cfg_file";
    location.m_format = CF_CSTYLE;

    AVConfig2Container container;

    AVConfig2Metadata metadata("test_config.some_param", AVConfig2Types::getTypeName<QString>(), DEFAULT_HELP);
    metadata.m_command_line_switch = "port";
    container.addParameter(special_char, metadata, location);


    AVConfig2SimpleSaver saver;
    saver.save(container.getLoadedData());

    // verify on file level
    {
        QFile save_file(location.getFullFilePath());
        QVERIFY(save_file.open(QIODevice::ReadOnly));
        QString s = save_file.readAll();
        QVERIFY(s.endsWith("// EOF\n"));
        QVERIFY(s.contains(special_char));
    }

    // read back in
    {
        AVConfig2Container container2;
        QVERIFY(container2.loadConfigAbsolute(location.getFullFilePath()));
        AVLogInfo << "read param: " << container2.getLoadedParameter("test_config.some_param")->getValue();
        QVERIFY(container2.getLoadedParameter("test_config.some_param")->getValue() == special_char);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCreateDuringRefreshAll()
{
    class LocalNestedSubConfig : public AVConfig2
    {
    public:
        LocalNestedSubConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            AVLogInfo << " --> LocalNestedSubConfig constructor <--";
            registerParameter("param", &m_param, DEFAULT_HELP);
        }
        int m_param;
    };
    class LocalSubConfig : public AVConfig2
    {
    public:
        LocalSubConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container),
            m_subconfig(new LocalNestedSubConfig(prefix + ".nested_sub", container))
        {
            AVLOG_ENTER_METHOD();
        }
        std::unique_ptr<LocalNestedSubConfig> m_subconfig;
    };
    /**
     * This test class covers two cases:
     *
     * - config constructed in postRefresh() call (via m_subconfig)
     * - config constructed during refreshParams() call (via m_subconfig_map)
     */
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".submap", &m_subconfig_map);
        }

        virtual QString postRefresh() override
        {
            AVLOG_ENTER_METHOD();
            m_subconfig.reset(new LocalSubConfig(getPrefix() + ".sub", getContainer()));
            return "";
        }
        std::unique_ptr<LocalSubConfig> m_subconfig;
        AVConfig2Map<LocalSubConfig> m_subconfig_map;
    };

    {
        AVLOG_ENTER(test missing param);

        AVConfig2Container container;
        LocalParentConfig parent_config(container);
        {
            AVLOG_ENTER(refresh all);
            AVConfig2Container::RefreshResult result = container.refreshAllParams();
            QVERIFY(!result.refreshSuccessful());
        }
    }

    {
        AVLOG_ENTER(test success);

        AVConfig2Container container;
        addParameterToContainer("parent.sub.nested_sub.param",      42, container);
        addParameterToContainer("parent.submap.a.nested_sub.param", 43, container);

        LocalParentConfig parent_config(container);
        {
            AVLOG_ENTER(refresh all);
            QVERIFY(checkedRefresh(container));
        }
        QVERIFY(parent_config.m_subconfig->m_subconfig->m_param == 42);
        QVERIFY(parent_config.m_subconfig_map["a"]->m_subconfig->m_param == 43);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testNamespaceOrderOnSave()
{
    class LocalSubConfig : public AVConfig2
    {
    public:
        LocalSubConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param", &m_param, DEFAULT_HELP);
        }
        uint m_param;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".submap", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig> m_subconfig_map;
    };

    QString filename = "namespace_order_test";
    QString tmp_filename = getTempConfigFileCopy(filename);
    AVLogInfo << "configs copy content:\n" << fileContent(tmp_filename);

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(tmp_filename));
    // avoid preloading step to mess with parameter order -> delete file
    // drawback: referenced parameter cannot be in this file because it is not saved out
    QVERIFY(QFile::remove(tmp_filename));

    LocalParentConfig config(container);
    QVERIFY(checkedRefresh(container));
    QCOMPARE(config.m_subconfig_map.count(), 3);

    AVConfig2Saver saver;
    saver.save(container);
    AVLogInfo << "saved config:\n" << fileContent(tmp_filename);
    verifyEqualFileContent(tmp_filename, filename);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testUnitTestConstructor()
{
    class UnitTestConfig : public AVConfig2
    {
    public:
        UnitTestConfig() :
            AVConfig2(UNIT_TEST_CONSTRUCTOR),
            m_param(3)
        {
        }

        int m_param;
    };

    UnitTestConfig config;
    QVERIFY(config.m_param == 3);
}

///////////////////////////////////////////////////////////////////////////////

class TestGlobalConfig : public AVConfig2, public AVSingleton<TestGlobalConfig>
{
public:
    TestGlobalConfig() : AVConfig2("global") {

        //registerParameter(QStringLiteral("dummy"), &dummy, "no help");
    }
    int dummy;

};

REGISTER_CONFIG_SINGLETON(TestGlobalConfig)

void AVConfig2Test::testConfigSingleton()
{
    QVERIFY(TestGlobalConfig::hasInstance());
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testEmptyLinesInParameterValues()
{
    auto check_empty_lines = [](const QString& path)
    {
        QString content = fileContent(path);
        QStringList lines = content.split("\n");

        bool next_is_empty = false;
        for(const QString& line : lines)
        {
            if(next_is_empty)
            {
                QVERIFY(line.simplified().isEmpty());
                next_is_empty = false;
            }

            if(line.contains("NEXTISEMPTY", Qt::CaseInsensitive))
            {
                next_is_empty = true;
            }
        }
    };

    QString filename = "empty_lines_in_parameter_values_test";
    QString path = getTempConfigFileCopy(filename);

    // pre-check
    check_empty_lines(path);

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));
    QVERIFY(container.m_loaded_duplicates.isEmpty());

    AVConfig2Saver saver;
    saver.setSaveMode(AVConfig2Saver::SM_ALL_LOADED);
    saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
    saver.save(container);

    check_empty_lines(path);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSuddenEnd()
{
    AVLogInfo << "skipping manual test AVConfig2Test::testSuddenEnd";
//    QString filename = "sudden_end_test";
//    QString path = getTempConfigFileCopy(filename);

//    AVConfig2Container container;
//    QVERIFY(container.loadConfigAbsolute(path));
//    QVERIFY(container.m_loaded_duplicates.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testInheritance()
{
    AVConfig2Container container;
    QVERIFY(container.loadConfig("inheritance_test"));
    QVERIFY(container.m_loaded_duplicates.isEmpty());

    QCOMPARE(container.getLoadedParameter("instance1.some_other_port")->getValue(), QString("3"));
    QCOMPARE(container.getLoadedParameter("instance1.some_port")->getValue(), QString("2"));
    QCOMPARE(container.getLoadedParameter("instance1.nested_instance1.nested_param")->getValue(), QString("overridden_nested"));

    QCOMPARE(container.getLoadedParameter("instance3.some_other_port")->getValue(), QString("3"));
    QCOMPARE(container.getLoadedParameter("instance3.some_port")->getValue(), QString("2"));
    QCOMPARE(container.getLoadedParameter("instance3.nested_instance1.nested_param")->getValue(), QString("overridden_nested"));
    QCOMPARE(container.getLoadedParameter("instance3.nested_instance3.nested_param")->getValue(), QString("nested"));

    QCOMPARE(container.getLoadedParameter("overriden_instance.some_port")->getValue(), QString("66"));

    QVERIFY(container.getLoadedParameter("instance2.some_other_port") == nullptr);

    AVConfig2Saver saver;
    saver.setSaveMode(AVConfig2Saver::SM_ALL_LOADED);
    saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
    saver.save(container);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testInheritanceWithFacts()
{
    AVConfig2Container container;
    AVConfig2StorageLocation::FactContainer facts;
    facts["A"] = "B";
    container.addFacts(facts);
    QVERIFY(container.loadConfig("inheritance_test"));

    QCOMPARE(container.getLoadedParameter("instance2.some_other_port")->getValue(), QString("3"));
    QCOMPARE(container.getLoadedParameter("instance2.some_port")->getValue(), QString("1"));
    QCOMPARE(container.getLoadedParameter("instance4.some_port")->getValue(), QString("1"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testInheritanceWithCmdlineOverrides()
{
    QStringList args = QStringList() << "-instance1_port" << "7"
                                     << "--instance3.some_port" << "4"
                                     << "--instance3.elements.element1.element_content" << "123";
    AVConfig2Container container;
    container.setCheckingMode(AVConfig2Container::CheckingMode::CM_LENIENT); // don't bother with metadata
    container.parseCmdlineParams(args, false);
    QVERIFY(container.loadConfig("inheritance_test"));

    QCOMPARE(container.getLoadedParameter("instance1.some_port")->getValue(), QString("7"));
    QCOMPARE(container.getLoadedParameter("instance3.some_port")->getValue(), QString("4"));

    // This actually was renamed due to a bug, see SWE-5787
    QCOMPARE(container.getLoadedParameter("instance3.some_port")->getName(), QString("instance3.some_port"));

    class ElementsConfig : public AVConfig2
    {
    public:
        ElementsConfig(const QString &prefix, AVConfig2Container &container)
            : AVConfig2(prefix, container)
        {
            registerParameter("element_content", &m_element_content, DEFAULT_HELP);
        }
        uint m_element_content;
    };

    class Config : public AVConfig2
    {
    public:
        Config(AVConfig2Container& container)
            : AVConfig2("instance3", container)
        {
            registerParameter("some_port", &m_some_port, DEFAULT_HELP);

            registerSubconfig(getPrefix() + ".elements", &m_elements);
        }
        uint m_some_port;
        AVConfig2Map<ElementsConfig> m_elements;
    } test_config(container);
    QVERIFY(checkedRefresh(container));
    QCOMPARE(test_config.m_some_port, 4u);
    QCOMPARE(test_config.m_elements.count(), 1);
    QCOMPARE(test_config.m_elements["element1"]->m_element_content, 123u);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testInheritanceMetadata()
{
    const QString TEST_CONFIG_NAME = "inheritance_metadata_test";
    AVConfig2Saver saver;
    saver.setSaveMode(AVConfig2Saver::SM_ALL_LOADED);
    saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);

    // first, just check that saving to another file yields the same results
    {
        AVConfig2Container container;
        QVERIFY(container.loadConfig(TEST_CONFIG_NAME));

        QTemporaryDir temp_dir;
        saver.setSaveDir(temp_dir.path());
        saver.save(container);

        QFile f1(container.locateConfig(TEST_CONFIG_NAME, true).getFullFilePath());
        QFile f2((QDir(temp_dir.path())).absoluteFilePath(TEST_CONFIG_NAME + ".cc"));
        QVERIFY(f1.open(QIODevice::ReadOnly));
        QVERIFY(f2.open(QIODevice::ReadOnly));
        QCOMPARE(f2.readAll(), f1.readAll());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testInheritanceWithReferences()
{
    static const char * TEST_CONFIG_FILENAME = "inheritance_reference_test";
    class LocalTestConfig : public AVConfig2
    {
    public:
        LocalTestConfig(QString prefix, AVConfig2Container& container) : AVConfig2(prefix, container)
        {
            registerParameter("port_1", &m_port_1, DEFAULT_HELP);
            registerParameter("port_2", &m_port_2, DEFAULT_HELP);
            registerParameter("port_3", &m_port_3, DEFAULT_HELP);
            registerParameter("port_4", &m_port_4, DEFAULT_HELP);
            registerParameter("port_5", &m_port_5, DEFAULT_HELP).setCmdlineSwitch("p5");
        }
        uint m_port_1;
        uint m_port_2;
        uint m_port_3;
        uint m_port_4;
        uint m_port_5;
    };

    QString temp_file = getTempConfigFileCopy(TEST_CONFIG_FILENAME);

    AVConfig2Container container;
    container.parseCmdlineParams(QStringList() << "-p5" << "6", false);
    QVERIFY(container.loadConfigAbsolute(temp_file));

    LocalTestConfig config("instance1", container);

    AVConfig2Container::RefreshResult ret;
    config.refreshParams(&ret);

    if (!ret.refreshSuccessful())
    {
        AVLogError << ret.getErrorMessage();
    }
    QCOMPARE(ret.getErrorMessage(), QString());

    QCOMPARE(config.m_port_1, 1u);
    QCOMPARE(config.m_port_2, 4u);
    QCOMPARE(config.m_port_3, 2u);
    QCOMPARE(config.m_port_4, 5u);
    QCOMPARE(config.m_port_5, 6u);

    {
        AVConfig2Saver saver;
        saver.setSaveMode(AVConfig2Saver::SM_ALL_LOADED);
        saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
        saver.save(container);

        verifyEqualFileContent(temp_file, TEST_CONFIG_FILENAME);
    }

    /*
     * This currently doesn't work correctly (order of port1 and port2 is exchanged)
     * see SWE-1428
    {
        AVConfig2Saver saver;
        saver.setSaveMode(AVConfig2Saver::SM_ONLY_REGISTERED);
        saver.setSaveValueMode(AVConfig2Saver::SVM_KEEP_LOADED_VALUES);
        saver.save(container);

        verifyEqualFileContent(temp_file, TEST_CONFIG_FILENAME);
    }
    */
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testInheritanceWithReferences2()
{
    AVConfig2Container container;
    class TestImporter : public TestParamImporterBase
    {
    public:
        explicit TestImporter(AVConfig2Container& container) : TestParamImporterBase(container) {}
        const uint REFERENCE_PORT = 123;
        void import() override
        {
            // This test case is based on the original problem when this bug was detected during fdp2 template config
            // restructuring.
            //
            // Note: Naming of the parameters is important - this problem only occurs if the order
            //       when resolving references is "correct" (container is QSet)

            // reference ports file
            addParameter("ports.cmd", REFERENCE_PORT);

            // port in fdp2 template config is 0
            addParameter("template_fdp2.cmd", 0u);

            // template inheritance of port 0
            {
                AVConfig2StorageLocation location;
                m_client->addInheritedSection("template_fdp2", "fdp2", location);
            }

            // reference to full port list
            {
                AVConfig2StorageLocation location;
                AVConfig2Metadata metadata("fdp2.cmd", "uint", DEFAULT_HELP);
                m_client->addReference("ports.cmd", metadata, location);
            }

            // reference to previous reference within some other configuration
            {
                AVConfig2StorageLocation location;
                AVConfig2Metadata metadata("daq_aftn2.fdp_cmd", "uint", DEFAULT_HELP);
                m_client->addReference("fdp2.cmd", metadata, location);
            }
        }

    } importer(container);

    importer.import();

    {
        AVLOG_ENTER(refresh);
        QVERIFY(checkedRefresh(container));
    }

    const AVConfig2ImporterClient::LoadedParameter * loaded =
            container.getLoadedData().getElementByName<AVConfig2ImporterClient::LoadedParameter>("daq_aftn2.fdp_cmd");

    QVERIFY(loaded);
    QCOMPARE(loaded->getValue(), QString("%1").arg(importer.REFERENCE_PORT));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigs()
{
    AVLOG_ENTER_METHOD();

    class ChildConfig : public AVConfig2
    {
    public:
        ChildConfig(const QString& prefix, AVConfig2Container& container) : AVConfig2(prefix, container)
        {
            registerParameter("param", &m_param, DEFAULT_HELP);
            refreshParams();
        }
        int m_param;
    };
    class ParentConfig : public AVConfig2
    {
    public:
        ParentConfig(AVConfig2Container& container) : AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".subs", &m_subconfig);
            refreshParams();
        }
        AVConfig2Map<ChildConfig> m_subconfig;
    };

    AVConfig2Container container;
    ParentConfig parent_config(container);

    addParameterToContainer("parent.subs.A.param", 1, container);
    addParameterToContainer("parent.subs.B.param", 2, container);

    QVERIFY(checkedRefresh(container));

    QCOMPARE(parent_config.m_subconfig.count(), 2);
    QCOMPARE(parent_config.m_subconfig["A"]->m_param, 1);
    QCOMPARE(parent_config.m_subconfig["B"]->m_param, 2);
    auto old_a = parent_config.m_subconfig["A"];

    container.replaceParameterValue("parent.subs.A.param", "20");
    addParameterToContainer("parent.subs.C.param", 3, container);

    QVERIFY(checkedRefresh(container));

    QCOMPARE(parent_config.m_subconfig.count(), 3);
    QCOMPARE(parent_config.m_subconfig["A"]->m_param, 20);
    QCOMPARE(parent_config.m_subconfig["B"]->m_param, 2);

    QCOMPARE(old_a, parent_config.m_subconfig["A"]);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigWithOnlyReferences()
{
    AVLOG_ENTER(testSubconfigWithOnlyReferences);

    AVConfig2Container original_container;

    {
        // this parameter triggers creation of a subsection (which then complains about its missing parameters, this
        // is ok in this test)
        AVConfig2Metadata metadata("test_config.nested_prefix.subconfig.dummy", "int", DEFAULT_HELP);
        original_container.addReference("other", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
    }
    {
        // references by the param above
        AVConfig2Metadata metadata("other", "int", DEFAULT_HELP);
        original_container.addParameter("value", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
    }
    {
        // don't have any other "missing param" errors
        AVConfig2Metadata metadata("test_config.test_string", "QString", DEFAULT_HELP);
        original_container.addParameter("test", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
    }
    {
        // don't have any other "missing param" errors
        AVConfig2Metadata metadata("test_config.port", "uint", DEFAULT_HELP);
        metadata.m_command_line_switch = "port";
        original_container.addParameter("2", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
    }

    AVConfig2Container container;
    original_container.assignDataTo(container); // test assignment while we're at it...

    TestConfig test_config(container);
    AVConfig2Container::RefreshResult result;
    test_config.refreshParams(&result);

    QVERIFY(result.m_missing_parameters.contains("test_config.nested_prefix.subconfig.string"));
    QVERIFY(result.m_missing_parameters.contains("test_config.nested_prefix.subconfig.int"));
    QVERIFY(test_config.m_sub_cfg.contains("subconfig"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSavingNewConfigMap()
{
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());
    QString filename("testSavingNewConfigMap");

    // save
    {
        AVConfig2Container container;

        AVConfig2Metadata metadata("test_config.test_string","QString", DEFAULT_HELP);
        container.addOverride("test_config.test_string", "overridden", AVConfig2StorageLocation());
        container.addParameter("original_value", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));

        TestConfig test_config(container);

        container.refreshAllParams();

        QSharedPointer<NestedConfig> nested(new NestedConfig(NESTED_PREFIX + ".new_sub", container));
        nested->m_int = 123;
        test_config.m_sub_cfg.insert("new_sub", nested);

        container.setStorageLocation(AVConfig2StorageLocation(temp_dir.path(), filename, CF_CSTYLE));

        AVConfig2Saver saver;
        saver.setSaveValueMode(AVConfig2Saver::SVM_ADOPT_PROCESS_VALUES);
        saver.save(container);
    }

    // load and check
    {
        AVConfig2Container container;
        TestConfig test_config(container);
        QVERIFY(container.loadConfigAbsolute(temp_dir.path() + "/" + filename + ".cc"));
        QVERIFY(checkedRefresh(container));

        QVERIFY(test_config.m_sub_cfg.contains("new_sub"));
        QVERIFY(test_config.m_sub_cfg["new_sub"]->m_int == 123);
        QVERIFY(test_config.m_test_string == "original_value");
    }
}

TestParamImporterBase::TestParamImporterBase(AVConfig2Container &container) :
    AVConfig2ImporterBase(AVConfig2StorageLocation())
{
    setClient(&container);
}

///////////////////////////////////////////////////////////////////////////////

TestParamImporter::TestParamImporter(AVConfig2Container &container) :
    TestParamImporterBase(container)
{

}

///////////////////////////////////////////////////////////////////////////////

void TestParamImporter::import()
{
    QVERIFY(m_client != 0);

    // Nested test params
    addParameter("test_config.test_string",QString("A"));
    addParameter("test_config.port",65000u, "port");

    addParameter(NESTED_PREFIX + ".section1.string",QString("s1"));
    addParameter(NESTED_PREFIX + ".section1.int",1);
    addParameter(NESTED_PREFIX + ".section1." + NESTED_SUB_PREFIX + ".first.double",1.1,
                 "test_config.nested_prefix.section1.nested_sub_prefix.first.double");
    addParameter(NESTED_PREFIX + ".section1." + NESTED_SUB_PREFIX + ".second.double",2.2,
                 "test_config.nested_prefix.section1.nested_sub_prefix.second.double");

    addParameter(NESTED_PREFIX + ".section2.string",QString("s2"));
    addParameter(NESTED_PREFIX + ".section2.int",2);

    addParameter(NESTED_PREFIX + ".section3.string",QString("s3"));
    addParameter(NESTED_PREFIX + ".section3.int",3);

    // cmdline test params
    // "port" command line switch conflict is intentional.
    addParameter("cmdline_test_config.second_port",400u, "port");

    // reference test params/references
    AVConfig2Metadata metadata1(
                NESTED_PREFIX + ".section3." + NESTED_SUB_PREFIX + ".first.double",
                AVConfig2Types::getTypeName<double>(), DEFAULT_HELP);

    metadata1.m_command_line_switch =
            NESTED_PREFIX + ".section3." + NESTED_SUB_PREFIX + ".first.double";

    m_client->addReference(NESTED_PREFIX + ".section1." + NESTED_SUB_PREFIX + ".first.double",
                           metadata1, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));

    AVConfig2Metadata metadata2(
                NESTED_PREFIX + ".section3." + NESTED_SUB_PREFIX + ".second.double",
                AVConfig2Types::getTypeName<double>(), DEFAULT_HELP);

    metadata2.m_command_line_switch =
            NESTED_PREFIX + ".section3." + NESTED_SUB_PREFIX + ".second.double";
    m_client->addReference(NESTED_PREFIX + ".section1." + NESTED_SUB_PREFIX + ".second.double",
                           metadata2, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
}

///////////////////////////////////////////////////////////////////////////////

TestReference2ParamImporter::TestReference2ParamImporter(AVConfig2Container &container) :
    TestParamImporterBase(container)
{

}

///////////////////////////////////////////////////////////////////////////////

void TestReference2ParamImporter::import()
{
    QVERIFY(m_client != 0);

    // Let cmdline_test_config.second_port refer to free.standing
    {
        AVConfig2Metadata metadata(
                    "cmdline_test_config.second_port", AVConfig2Types::getTypeName<uint>(), DEFAULT_HELP);
        metadata.m_command_line_switch = "port";
        m_client->addReference("free.standing", metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));

        addParameter("free.standing", 123u);
    }
}

///////////////////////////////////////////////////////////////////////////////

OptionalParamTestImporter::OptionalParamTestImporter(AVConfig2Container &container) :
    TestParamImporterBase(container)
{

}

///////////////////////////////////////////////////////////////////////////////

void OptionalParamTestImporter::import()
{
    QVERIFY(m_client != 0);
    m_client->addOverride("test_config.optional_param_dev_override", "dev_override_value",
                          AVConfig2StorageLocation());
}

///////////////////////////////////////////////////////////////////////////////

NestedSubConfig::NestedSubConfig(const QString &prefix, AVConfig2Container &config) :
    AVConfig2(prefix, config)
{
    QVERIFY(prefix.startsWith("test_config"));
    registerParameter("double",&m_double, DEFAULT_HELP).setCmdlineSwitch(prefix + ".double");
    registerParameter("pure_cmdline", &m_pure_cmdline, DEFAULT_HELP).setPureCmdlineOption("pure_default");
}

///////////////////////////////////////////////////////////////////////////////

NestedConfig::NestedConfig(const QString &prefix, AVConfig2Container &config) :
    AVConfig2(prefix, config)
{
    QVERIFY(prefix.startsWith("test_config"));

    registerParameter("string",&m_string, DEFAULT_HELP);
    registerParameter("int",&m_int, DEFAULT_HELP);
    registerSubconfig(m_prefix + "." + NESTED_SUB_PREFIX,&m_sub_cfg);
}

///////////////////////////////////////////////////////////////////////////////

TestConfig::TestConfig(AVConfig2Container &container) : AVConfig2("test_config", container)
{
    setHelpGroup("Test");

    registerParameter("test_string",&m_test_string, DEFAULT_HELP);
    registerParameter("port",&m_port, DEFAULT_HELP).setCmdlineSwitch("port");
    registerSubconfig(NESTED_PREFIX,&m_sub_cfg);
}

///////////////////////////////////////////////////////////////////////////////

CmdlineTestConfig::CmdlineTestConfig(AVConfig2Container &config) : AVConfig2("cmdline_test_config", config)
{
    // "port" command line switch conflict is intentional.
    registerParameter("second_port",&m_port, DEFAULT_HELP).setCmdlineSwitch("port");
}

///////////////////////////////////////////////////////////////////////////////

OptionalCmdlineTestConfig::OptionalCmdlineTestConfig(AVConfig2Container &config) : AVConfig2("test_config", config)
{
    registerParameter("optional_param", &m_optional_param, DEFAULT_HELP).
            setCmdlineSwitch("optional").
            setOptional("default_value");
    registerParameter("optional_param_dev_override", &m_optional_param_dev_override, DEFAULT_HELP).
            setOptional("default_value");
}

void OptionalCmdlineTestConfig::changeSuggestedValues()
{
    changeSuggestedValue(&m_optional_param,              QString("CHANGED_DEFAULT1"));
    changeSuggestedValue(&m_optional_param_dev_override, QString("CHANGED_DEFAULT2"));
}

///////////////////////////////////////////////////////////////////////////////

DeprecatedTestConfig::DeprecatedTestConfig(AVConfig2Container &config) : AVConfig2("deprecated_test_config", config)
{
    registerParameter("some_port", &m_port1, DEFAULT_HELP).setDeprecatedName("SomePort").setDeprecatedName("SomeOldPort");
    registerParameter("some_other_port", &m_port2, DEFAULT_HELP).setDeprecatedName("SomeOtherPort").setDeprecatedPrefix("DeprecatedTestConfig");
    registerParameter("third_port", &m_port3, DEFAULT_HELP).setDeprecatedPrefix("DeprecatedTestConfig");
    registerParameter("no_precated_port", &m_port4, DEFAULT_HELP);
}

///////////////////////////////////////////////////////////////////////////////

EnumTestConfig::EnumTestConfig(AVConfig2Container &config) : AVConfig2("enum_test_config", config)
{
    registerParameter("qenum_value", &m_qenum_value, DEFAULT_HELP).setValidValues({ValueQEnum::Value1, ValueQEnum::Value2});
    registerParameter("avenum_value", &m_avenum_value, DEFAULT_HELP);
    registerParameter("namespace_avenum_value", &m_namespace_avenum_value, DEFAULT_HELP);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testRefreshParameters()
{
    // 1. Neccessary preparation for this unit test,
    // in order to come to the deepest point in function AVConfig2Container::refreshParams()

    // locally defined classes for testing all kinds of value, defined for parameters in a config file
    class ChildNode : public AVConfig2
    {
    public:
        ChildNode(const QString& prefix, AVConfig2Container& container)
            : AVConfig2(prefix, container)
        {
            // here no code is needed for this test
        }
    };

    class RootNode : public AVConfig2
    {
    public:
        RootNode(const QString& prefix, AVConfig2Container& container)
            : AVConfig2(prefix, container)
        {
            registerParameter("string_value_of_null_terminator", &m_test_string, DEFAULT_HELP);

            // SPECIAL NOTE: here "root_config" is registered, which causes error in SWE-5739
            // properly usage should be: registerSubconfig("root_config.child_config", &m_sub_cfg);
            registerSubconfig(prefix, &m_sub_cfg);
        }
        QString                 m_test_string;
        AVConfig2Map<ChildNode> m_sub_cfg;
    };

    const QString TEST_CONFIG_NAME = "parameters_test";

    AVConfig2Container container;
    RootNode config("root_config", container);
    QVERIFY(container.loadConfig(TEST_CONFIG_NAME));

    AVConfig2Container::RefreshResult result;

    // 2. MAIN activity for this unit test
    // verifying that with an invalid registerSubconfig (register a root node as a subconfig)
    // will fail refreshParams() activity which returns false
    QVERIFY(! config.refreshParams(&result));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigTemplate()
{
    const QString FILENAME = "subconfig_template_test";
    QString path = getTempConfigFileCopy(FILENAME);

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));

    const AVConfig2LoadedData& container_loaded_data = container.getLoadedData();

    AVConfig2LoadedData loaded_data(false);
    QVERIFY(loaded_data.load(path));

    QCOMPARE(container_loaded_data.m_loaded_subconfig_template_parameter,
             loaded_data.m_loaded_subconfig_template_parameter);

    QStringList expected_template_parameter_names = QStringList()
            << "alertchecker.filters.*.enabled"
            << "alertchecker.filters.*.client.host"
            << "alertchecker.filters.*.client.port"
            << "alertchecker.filters.*.regions.*.area";
    QStringList template_parameter_names;
    QCOMPARE(loaded_data.m_loaded_subconfig_template_parameter.size(), 4);
    for (auto& template_param : loaded_data.m_loaded_subconfig_template_parameter)
    {
        QVERIFY2(expected_template_parameter_names.contains(template_param.getName()),
                 ("Unexpected parameter name: " + template_param.getName()).toUtf8());
        template_parameter_names.append(template_param.getName());
    }
    template_parameter_names.removeDuplicates();
    // there should be no duplicates
    QCOMPARE(template_parameter_names.size(), expected_template_parameter_names.size());

    QList<AVConfig2LoadedData::SubconfigTemplateSpecification> templates;

    // Providing correct subconfig template
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters");
    QCOMPARE(templates.size(), 1);
    QVERIFY(templates[0].getTemplateName() == "alertchecker.filters");
    QVERIFY(templates[0].getLocation().getFullFilePath() == path);

    // Providing correct nested subconfig template
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters.*.regions");
    QCOMPARE(templates.size(), 1);
    QVERIFY(templates[0].getTemplateName() == "alertchecker.filters.*.regions");
    QVERIFY(templates[0].getLocation().getFullFilePath() == path);

    // Providing correct nested subconfig template with defined subconfig namespace (filter_a)
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters.filter_a.regions");
    QCOMPARE(templates.size(), 1);
    QVERIFY(templates[0].getTemplateName() == "alertchecker.filters.*.regions");
    QVERIFY(templates[0].getLocation().getFullFilePath() == path);

    // Providing incomplete prefix for subconfig template
    templates = loaded_data.listSubconfigTemplates("alertchecker");
    QCOMPARE(templates.size(), 0);

    // Providing subconfig template with extra namespace (filter_a)
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters.filter_a");
    QCOMPARE(templates.size(), 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigTemplateInstantiation()
{
    const QString FILENAME = "subconfig_template_test";
    QString path = getTempConfigFileCopy(FILENAME);

    AVConfig2LoadedData loaded_data(true);
    QVERIFY(loaded_data.load(path));

    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 0u);

    AVConfig2StorageLocation location(m_temp_dir->path(), FILENAME, CF_CSTYLE);

    AVConfig2LoadedData::SubconfigTemplateSpecification spec = { "alertchecker.filters", location };
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "filter_a"));

    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 3u);

    const AVConfig2ImporterClient::LoadedParameter* enabled_param =
            loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("alertchecker.filters.filter_a.enabled");
    QVERIFY(enabled_param != nullptr);
    QCOMPARE(enabled_param->getValue(), QString("true"));
    QCOMPARE(enabled_param->getMetadata().m_type, QString("uint"));

    const AVConfig2ImporterClient::LoadedParameter* client_host_param =
            loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("alertchecker.filters.filter_a.client.host");
    QVERIFY(client_host_param != nullptr);
    QCOMPARE(client_host_param->getValue(), QString("localhost"));
    QCOMPARE(client_host_param->getMetadata().m_type, QString("QString"));

    const AVConfig2ImporterClient::LoadedParameter* client_port_param =
            loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("alertchecker.filters.filter_a.client.port");
    QVERIFY(client_port_param != nullptr);
    QCOMPARE(client_port_param->getValue(), QString("2233"));
    QCOMPARE(client_port_param->getMetadata().m_type, QString("uint"));

    // Subconfig template instantiation with defined subconfig namespace (filter_a)
    spec = { "alertchecker.filters.filter_a.regions", location };
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "area1"));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 4u);
    const AVConfig2ImporterClient::LoadedParameter* region_area1_param =
            loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("alertchecker.filters.filter_a.regions.area1.area");
    QVERIFY(region_area1_param != nullptr);
    QCOMPARE(region_area1_param->getValue(), QString("[1;2;3;4]"));
    QCOMPARE(region_area1_param->getMetadata().m_type, QString("QRect"));

    // Subconfig template instantiation with defined subconfig namespace (filter_a) and instance name "*"
    spec = { "alertchecker.filters.filter_a.regions", location };
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "*"));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 5u);
    const AVConfig2ImporterClient::LoadedParameter* region_area2_param =
            loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("alertchecker.filters.filter_a.regions.*.area");
    QVERIFY(region_area2_param != nullptr);
    QCOMPARE(region_area2_param->getValue(), QString("[1;2;3;4]"));
    QCOMPARE(region_area2_param->getMetadata().m_type, QString("QRect"));

    spec = { "alertchecker.filters.*.regions", location };

    // Nested subconfig template instantiation
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, { "filter_b", "area_b" }));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 6u);
    const AVConfig2ImporterClient::LoadedParameter* region_area_b_param =
            loaded_data.getElementByName<AVConfig2ImporterClient::LoadedParameter>("alertchecker.filters.filter_b.regions.area_b.area");
    QVERIFY(region_area_b_param != nullptr);
    QCOMPARE(region_area_b_param->getValue(), QString("[1;2;3;4]"));
    QCOMPARE(region_area_b_param->getMetadata().m_type, QString("QRect"));

    // Attempt to instantiate subconfig template without the required number of instance names
    QVERIFY(! loaded_data.instantiateSubconfigTemplate(spec, "area_b"));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 6u);

    // Attempt to instantiate non existing subconfig template
    spec = { "alertchecker.invalid.*.regions", location };
    QVERIFY(! loaded_data.instantiateSubconfigTemplate(spec, { "filter_a", "area_b" }));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 6u);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigTemplateDuplication()
{
    AVConfig2LoadedData loaded_data(false);
    using SubconfigTemplateParam = AVConfig2ImporterClient::LoadedSubconfigTemplateParameter;

    const QString FILENAME_1 = "subconfig_template_test";
    QString path1 = getTempConfigFileCopy(FILENAME_1);
    QVERIFY(loaded_data.load(path1));
    QCOMPARE(loaded_data.getElementCount<SubconfigTemplateParam>(), 4u);

    const QString FILENAME_2 = "subconfig_template_duplication_test";
    QString path2 = getTempConfigFileCopy(FILENAME_2);
    QVERIFY(loaded_data.load(path2));
    QCOMPARE(loaded_data.getElementCount<SubconfigTemplateParam>(), 8u);

    const QString FILENAME_3 = "subconfig_template_duplication_test2";
    QString path3 = getTempConfigFileCopy(FILENAME_3);
    QVERIFY(loaded_data.load(path3));
    QCOMPARE(loaded_data.getElementCount<SubconfigTemplateParam>(), 12u);

    QList<AVConfig2LoadedData::SubconfigTemplateSpecification> templates;
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters");
    QCOMPARE(templates.size(), 3);
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters.*.regions");
    QCOMPARE(templates.size(), 2);

    QMap<AVConfig2StorageLocation, QVector<QString>> location_params;

    AVConfig2StorageLocation location1(m_temp_dir->path(), FILENAME_1, CF_CSTYLE);
    AVConfig2LoadedData::SubconfigTemplateSpecification spec ("alertchecker.filters", location1);
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "filter_a"));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 3u);
    location_params[location1].append( { "alertchecker.filters.filter_a.enabled",
                                         "alertchecker.filters.filter_a.client.host",
                                         "alertchecker.filters.filter_a.client.port" } );

    AVConfig2StorageLocation location2(m_temp_dir->path(), FILENAME_2, CF_CSTYLE);
    spec.setLocation(location2);
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "filter_b"));
    QCOMPARE(loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>(), 6u);
    location_params[location2].append( { "alertchecker.filters.filter_b.enabled",
                                         "alertchecker.filters.filter_b.client.host",
                                         "alertchecker.filters.filter_b.client.port" } );

    AVConfig2StorageLocation location3(m_temp_dir->path(), FILENAME_3, CF_CSTYLE);
    spec.setLocation(location3);
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "filter_c"));
    uint element_count = loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>();
    QCOMPARE(element_count, 10u);
    location_params[location3].append( { "alertchecker.filters.filter_c.enabled",
                                         "alertchecker.filters.filter_c.client.name",
                                         "alertchecker.filters.filter_c.client.host",
                                         "alertchecker.filters.filter_c.client.port" } );

    // Check if locations of the instantiated parameters are correct
    for (uint i = 0; i < element_count; ++i)
    {
        AVConfig2ImporterClient::LoadedParameter& param = loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(i);
        QVERIFY(location_params[param.getLocation()].contains(param.getName()));
    }

    // Instantiating duplicate parameters:
    //  alertchecker.filters.filter_b.enabled
    //  alertchecker.filters.filter_b.client.host
    //  alertchecker.filters.filter_b.client.port
    QVERIFY(loaded_data.instantiateSubconfigTemplate(spec, "filter_b"));

    element_count = loaded_data.getElementCount<AVConfig2ImporterClient::LoadedParameter>();
    QCOMPARE(element_count, 14u);
    location_params[location3].append( { "alertchecker.filters.filter_b.enabled",
                                         "alertchecker.filters.filter_b.client.name",
                                         "alertchecker.filters.filter_b.client.host",
                                         "alertchecker.filters.filter_b.client.port" } );

    // Check once again if paramters have correct locations
    for (uint i = 0; i < element_count; ++i)
    {
        AVConfig2ImporterClient::LoadedParameter& param = loaded_data.getElementByIndex<AVConfig2ImporterClient::LoadedParameter>(i);
        QVERIFY(location_params[param.getLocation()].contains(param.getName()));
        // Remove parameter name from the locations map
        location_params[param.getLocation()].removeOne(param.getName());
    }
    // There must be no parameter names in the map
    for (const QVector<QString>& params : location_params)
    {
        QVERIFY(params.empty());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testRegisterSubconfigForTemplateCreation()
{
    class LocalSubConfig3 : public AVConfig2
    {
    public:
        LocalSubConfig3(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param3", &m_param, DEFAULT_HELP);
            registerParameter("submap3_element_namespace.ns_param1", &m_ns_param, DEFAULT_HELP);
        }
        uint m_param;
        int  m_ns_param;
    };
    class LocalSubConfigNamespace : public AVConfig2
    {
    public:
        LocalSubConfigNamespace(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("ns_param1", &m_param1, DEFAULT_HELP);
            registerParameter("ns_param2", &m_param2, DEFAULT_HELP);
        }

        QString m_param1;
        QString m_param2;
    };
    class LocalSubConfig2 : public AVConfig2
    {
    public:
        LocalSubConfig2(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param2", &m_param, DEFAULT_HELP);
            m_namespace = new LocalSubConfigNamespace(prefix + ".submap2_element_namespace", container);
            registerSubconfig(prefix + ".submap3", &m_subconfig_map);
        }

        ~LocalSubConfig2() override
        {
            delete m_namespace;
        }

        LocalSubConfigNamespace* m_namespace;

        uint m_param;
        AVConfig2Map<LocalSubConfig3> m_subconfig_map;
    };
    class LocalSubConfig1 : public AVConfig2
    {
    public:
        LocalSubConfig1(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param1", &m_param, DEFAULT_HELP);
            registerSubconfig(prefix + ".submap2", &m_subconfig_map);
        }
        uint m_param;
        AVConfig2Map<LocalSubConfig2> m_subconfig_map;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".submap", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig1> m_subconfig_map;
    };

    AVConfig2Container container;
    const QString FILENAME = "subconfig_template_create_test";
    QString path = getTempConfigFileCopy(FILENAME);
    QVERIFY(container.loadConfigAbsolute(path));
    LocalParentConfig config(container);
    container.refreshAllParams();
    LocalSubConfig2 subconfig2("parent.submap.another_name.submap2.another_name", container);

    const AVConfig2LoadedData& loaded_data = container.getLoadedData();

    QStringList expected_template_parameter_names = QStringList()
            << "parent.submap.*.param1"
            << "parent.submap.*.submap2.*.param2"
            << "parent.submap.*.submap2.*.submap2_element_namespace.ns_param1"
            << "parent.submap.*.submap2.*.submap2_element_namespace.ns_param2"
            << "parent.submap.*.submap2.*.submap3.*.param3"
            << "parent.submap.*.submap2.*.submap3.*.submap3_element_namespace.ns_param1";
    QStringList template_parameter_names;
    QCOMPARE(loaded_data.m_loaded_subconfig_template_parameter.size(), 6);
    for (auto& template_param : loaded_data.m_loaded_subconfig_template_parameter)
    {
        QVERIFY2(expected_template_parameter_names.contains(template_param.getName()),
                 ("Unexpected parameter name: " + template_param.getName()).toUtf8());
        template_parameter_names.append(template_param.getName());
    }
    template_parameter_names.removeDuplicates();
    // there should be no duplicates
    QCOMPARE(template_parameter_names.size(), expected_template_parameter_names.size());
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testRegisterSubconfigRecursiveForTemplateCreation()
{
    class LocalSubConfig1;
    class LocalSubConfig2 : public AVConfig2
    {
    public:
        LocalSubConfig2(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param2", &m_param, DEFAULT_HELP);
            // In case of infinite recursion, stop here to avoid running out of memory
            AVASSERTMSG(prefix.size() < 1000, "Infinite recursive subconfig template creation");
            registerSubconfig(prefix + ".submap2", &m_subconfig_map);
        }
        uint m_param;
        AVConfig2Map<LocalSubConfig1> m_subconfig_map;
    };
    class LocalSubConfig1 : public AVConfig2
    {
    public:
        LocalSubConfig1(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param", &m_param, DEFAULT_HELP);
            // In case of infinite recursion, stop here to avoid running out of memory
            AVASSERTMSG(prefix.size() < 1000, "Infinite recursive subconfig template creation");
            registerSubconfig(prefix + ".submap", &m_subconfig_map);
        }
        uint m_param;
        AVConfig2Map<LocalSubConfig2> m_subconfig_map;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".map", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig1> m_subconfig_map;
    };
    AVConfig2Container container;
    LocalParentConfig parent_config(container);
    QCOMPARE(container.m_loaded_data.m_loaded_subconfig_template_parameter.size(), 4);
    QStringList expected_template_parameter_names = QStringList()
            << "parent.map.*.param"
            << "parent.map.*.submap.*.param2"
            << "parent.map.*.submap.*.submap2.*.param"
            << "parent.map.*.submap.*.submap2.*.submap.*.param2";
    QStringList template_parameter_names;
    for (auto& template_param : container.m_loaded_data.m_loaded_subconfig_template_parameter)
    {
        QVERIFY2(expected_template_parameter_names.contains(template_param.getName()),
                 ("Unexpected parameter name: " + template_param.getName()).toUtf8());
        template_parameter_names.append(template_param.getName());
    }
    template_parameter_names.removeDuplicates();
    // there should be no duplicates
    QCOMPARE(template_parameter_names.size(), expected_template_parameter_names.size());
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigTemplateSave()
{
    QString FILENAME = "subconfig_template_save_test";
    QString path = getTempConfigFileCopy(FILENAME);
    AVConfig2Container loaded_container;
    QVERIFY(loaded_container.loadConfigAbsolute(path));

    // Loaded data must have subconfig templates
    QVERIFY(! loaded_container.m_loaded_data.m_loaded_subconfig_template_parameter.empty());

    QVERIFY(QFile(path).remove());

    AVConfig2Saver saver;
    saver.save(loaded_container);

    AVConfig2Container saved_container;
    QVERIFY(saved_container.loadConfigAbsolute(path));

    QCOMPARE(loaded_container.m_loaded_data.m_loaded_subconfig_template_parameter,
             saved_container.m_loaded_data.m_loaded_subconfig_template_parameter);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigTemplateSaveWithSimpleSaver()
{
    QString FILENAME = "subconfig_template_save_test";
    QString path = getTempConfigFileCopy(FILENAME);

    AVConfig2LoadedData loaded_data(true);
    QVERIFY(loaded_data.load(path));

    // Loaded data must have subconfig templates
    QVERIFY(! loaded_data.m_loaded_subconfig_template_parameter.empty());

    QVERIFY(QFile(path).remove());

    AVConfig2SimpleSaver saver;
    saver.save(loaded_data);

    AVConfig2LoadedData saved_data(true);
    QVERIFY(saved_data.load(path));

    QCOMPARE(loaded_data.m_loaded_subconfig_template_parameter, saved_data.m_loaded_subconfig_template_parameter);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSubconfigTemplateDuplicationSave()
{
    using SubconfigTemplateParam = AVConfig2ImporterClient::LoadedSubconfigTemplateParameter;

    AVConfig2LoadedData loaded_data(false);

    const QString FILENAME_1 = "subconfig_template_test";
    QString path1 = getTempConfigFileCopy(FILENAME_1);
    QVERIFY(loaded_data.load(path1));
    QCOMPARE(loaded_data.getElementCount<SubconfigTemplateParam>(), 4u);

    const QString FILENAME_2 = "subconfig_template_duplication_test";
    QString path2 = getTempConfigFileCopy(FILENAME_2);
    QVERIFY(loaded_data.load(path2));
    QCOMPARE(loaded_data.getElementCount<SubconfigTemplateParam>(), 8u);

    const QString FILENAME_3 = "subconfig_template_duplication_test2";
    QString path3 = getTempConfigFileCopy(FILENAME_3);
    QVERIFY(loaded_data.load(path3));
    QCOMPARE(loaded_data.getElementCount<SubconfigTemplateParam>(), 12u);

    QList<AVConfig2LoadedData::SubconfigTemplateSpecification> templates;
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters");
    QCOMPARE(templates.size(), 3);
    templates = loaded_data.listSubconfigTemplates("alertchecker.filters.*.regions");
    QCOMPARE(templates.size(), 2);

    QVERIFY(QFile(path1).remove());
    QVERIFY(QFile(path2).remove());
    QVERIFY(QFile(path3).remove());

    AVConfig2SimpleSaver saver;
    saver.save(loaded_data);

    QString filenames[3] = { path1, path2, path3 };
    QStringList expected_template_parameter_names[3] {
        {
            "alertchecker.filters.*.enabled",
            "alertchecker.filters.*.client.host",
            "alertchecker.filters.*.client.port",
            "alertchecker.filters.*.regions.*.area"
        },
        {
            "alertchecker.filters.*.enabled",
            "alertchecker.filters.*.client.host",
            "alertchecker.filters.*.client.port",
            "alertchecker.filters.*.regions.*.area"
        },
        {
            "alertchecker.filters.*.enabled",
            "alertchecker.filters.*.client.name",
            "alertchecker.filters.*.client.host",
            "alertchecker.filters.*.client.port"
        }
    };
    QString expected_port_values[3] = { "2233", "1122", "8888" };

    // load each config seperately and check if they were saved properly
    for (int i = 0; i < 3; ++i)
    {
        AVConfig2Container container;
        container.loadConfigAbsolute(filenames[i]);
        QCOMPARE(container.m_loaded_data.m_loaded_subconfig_template_parameter.size(), 4);
        QStringList template_parameter_names;
        for (auto& template_param : container.m_loaded_data.m_loaded_subconfig_template_parameter)
        {
            QVERIFY2(expected_template_parameter_names[i].contains(template_param.getName()),
                     ("Unexpected parameter name: " + template_param.getName()).toUtf8());
            template_parameter_names.append(template_param.getName());

            QCOMPARE(template_param.getLocation().getFullFilePath(), filenames[i]);
            if (template_param.getName() == "alertchecker.filters.*.client.port")
            {
                QCOMPARE(template_param.getValue(), expected_port_values[i]);
            }
        }
        template_parameter_names.removeDuplicates();
        // there should be no duplicates
        QCOMPARE(template_parameter_names.size(), expected_template_parameter_names[i].size());
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testMissingSubconfigTemplates()
{
    class LocalSubConfig3 : public AVConfig2
    {
    public:
        LocalSubConfig3(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param3", &m_param, DEFAULT_HELP);
            registerParameter("submap3_element_namespace.ns_param1", &m_ns_param, DEFAULT_HELP);
        }
        uint m_param;
        int  m_ns_param;
    };
    class LocalSubConfigNamespace : public AVConfig2
    {
    public:
        LocalSubConfigNamespace(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("ns_param1", &m_param1, DEFAULT_HELP);
            registerParameter("ns_param2", &m_param2, DEFAULT_HELP);
        }

        QString m_param1;
        QString m_param2;
    };
    class LocalSubConfig2 : public AVConfig2
    {
    public:
        LocalSubConfig2(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param2", &m_param, DEFAULT_HELP);
            m_namespace = new LocalSubConfigNamespace(prefix + ".submap2_element_namespace", container);
            registerSubconfig(prefix + ".submap3", &m_subconfig_map);
        }

        ~LocalSubConfig2() override
        {
            delete m_namespace;
        }

        LocalSubConfigNamespace* m_namespace;

        uint m_param;
        AVConfig2Map<LocalSubConfig3> m_subconfig_map;
    };
    class LocalSubConfig1 : public AVConfig2
    {
    public:
        LocalSubConfig1(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("param1", &m_param, DEFAULT_HELP);
            registerSubconfig(prefix + ".submap2", &m_subconfig_map);
        }
        uint m_param;
        AVConfig2Map<LocalSubConfig2> m_subconfig_map;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".submap", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig1> m_subconfig_map;
    };

    AVConfig2Container container;
    LocalParentConfig config(container);
    const QString FILENAME = "subconfig_template_create_test";
    QString path = getTempConfigFileCopy(FILENAME);

    QVERIFY(!fileContent(path).contains("subconfig_template"));
    QVERIFY(container.loadConfigAbsolute(path));
    QVERIFY(config.refreshParams());

    AVConfig2Saver saver;
    saver.save(container);

    QVERIFY(fileContent(path).contains("subconfig_template"));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testRegisterSameMemberWithDifferentName()
{
    AVLog::getRootLogger()->setFatalSegfault(false);
    AVLog::getRootLogger()->setFatalExit(false);
    QSignalSpy spy(AVLog::getRootLogger(), &AVLog::signalFatalLog);

    class LocalConfig : public AVConfig2
    {
    public:
        LocalConfig(AVConfig2Container& container) :
            AVConfig2("local", container)
        {
            registerParameter("original_name", &m_string, DEFAULT_HELP);
            // the following line causes a fatal error
            registerParameter("different_name", &m_string, DEFAULT_HELP);
        }
        QString m_string;
    };

    AVConfig2Container container;
    LocalConfig config(container);

    QVERIFY(spy.count() > 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testRegisterSubconfigWithSameName()
{
    AVLog::getRootLogger()->setFatalSegfault(false);
    AVLog::getRootLogger()->setFatalExit(false);
    QSignalSpy spy(AVLog::getRootLogger(), &AVLog::signalFatalLog);

    class LocalSubconfig : public AVConfig2
    {
    public:
        LocalSubconfig(const QString& prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("subconfig_parameter", &m_string, DEFAULT_HELP);
        }

        QString m_string;
    };

    class LocalConfig : public AVConfig2
    {
    public:
        LocalConfig(AVConfig2Container& container) :
            AVConfig2("local", container)
        {
            registerSubconfig("subconfig", &m_subconfig_1);
            // Test case 1. The following code line causes a fatal error.
            // Comment the line to test the 2nd test case.
            registerSubconfig("subconfig", &m_subconfig_2);
        }
        AVConfig2Map<LocalSubconfig> m_subconfig_1;
        AVConfig2Map<LocalSubconfig> m_subconfig_2;
    };

    class LocalConfig2 : public AVConfig2
    {
    public:
        LocalConfig2(AVConfig2Container& container) :
            AVConfig2("local", container)
        {
            registerSubconfig("subconfig", &m_subconfig);
        }
        AVConfig2Map<LocalSubconfig> m_subconfig;
    };

    AVConfig2Container container;
    LocalConfig config(container);
    LocalConfig another_config(container);

    // Test case 2. The following line causes a fatal error.
    LocalConfig2 config2(container);

    QVERIFY(spy.count() > 0);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testRemovingParametersFromConfigFile()
{
    class LocalSubConfig : public AVConfig2
    {
    public:
        LocalSubConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            // set the flag to remove registered parameters on this config destruction
            setRemoveRegisteredParamsOnDestruction(true);
            registerParameter("parameter", &m_param, DEFAULT_HELP);
        }
        uint m_param;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".subconfigs", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig> m_subconfig_map;
    };

    const QString FILENAME = "remove_parameters_from_file_test";
    QString path = getTempConfigFileCopy(FILENAME);

    AVConfig2Container original_container;
    QVERIFY(original_container.loadConfigAbsolute(path));
    LocalParentConfig original_config(original_container);
    original_container.refreshAllParams();

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));
    LocalParentConfig config(container);
    container.refreshAllParams();

    QVERIFY(!config.m_subconfig_map.empty());
    // destroy one of the subconfig elements
    config.m_subconfig_map.erase(config.m_subconfig_map.begin());
    // saver should remove the deleted subconfig from the file
    AVConfig2Saver saver;
    saver.save(container);

    QString file_content = fileContent(path);
    AVConfig2Container compare_container;
    QVERIFY(compare_container.loadConfigAbsolute(path));
    LocalParentConfig compare_config(compare_container);
    compare_container.refreshAllParams();

    QCOMPARE(compare_config.m_subconfig_map.size(), config.m_subconfig_map.size());
    QCOMPARE(compare_config.m_subconfig_map.size(), original_config.m_subconfig_map.size() - 1);

    // make sure old behaviour doesn't change
    for (QSharedPointer<LocalSubConfig> subconfig : config.m_subconfig_map)
    {
        subconfig->setRemoveRegisteredParamsOnDestruction(false);
    }
    config.m_subconfig_map.erase(config.m_subconfig_map.begin());
    saver.save(container);

    // the erased subconfig should still be in the config file
    QCOMPARE(file_content, fileContent(path));
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testCStyleVersionPinning()
{
    class LocalSubConfig : public AVConfig2
    {
    public:
        LocalSubConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("parameter", &m_param, DEFAULT_HELP);
        }
        uint m_param;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".subconfigs", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig> m_subconfig_map;
    };

    const QString FILENAME = "saver_pinned_version_test";
    QString path = getTempConfigFileCopy(FILENAME);

    const QString file_content_before = fileContent(path);
    QVERIFY(!file_content_before.contains("subconfig_template"));

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));
    LocalParentConfig config(container);
    container.refreshAllParams();
    QVERIFY(!config.m_subconfig_map.empty());

    // test saver with old (V_2) pinned version
    AVConfig2ExporterFactory factory;
    factory.setPinnedCStyleVersion(2);
    AVConfig2Saver pinned_version_saver;
    pinned_version_saver.setExporterFactory(factory);
    pinned_version_saver.save(container);
    const QString file_content_after = fileContent(path);
    QCOMPARE(file_content_before, file_content_after);

    // test saver without setting AVConfig2ExporterFactory explicitly
    AVConfig2Saver saver;
    saver.save(container);
    const QString file_content_subconfig_templates1 = fileContent(path);
    AVLogInfo << file_content_subconfig_templates1;
    QVERIFY(file_content_subconfig_templates1.contains("subconfig_template"));

    // test saver with the newest (V_CURRENT) pinned version
    factory.setPinnedCStyleVersion(AVConfig2ImporterCstyle::V_CURRENT);
    saver.setExporterFactory(factory);
    saver.save(container);
    const QString file_content_subconfig_templates2 = fileContent(path);
    QCOMPARE(file_content_subconfig_templates1, file_content_subconfig_templates2);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testSavingInvalidValueForParamWithRestrictions()
{
    class LocalConfig : public AVConfig2
    {
    public:
        LocalConfig(AVConfig2Container& container) :
            AVConfig2("parameter", container)
        {
            const QStringList valid_values { "Military", "RequiresNextFix", "RequiresRelease", "RequiresPrnav" };
            registerParameter("flags", &m_flags, "Flags").setValidContainerValues(valid_values);
        }
        QStringList m_flags;
    };

    const QString FILENAME = "invalid_value_param_with_restrictions_test";
    QString path = getTempConfigFileCopy(FILENAME);

    // Load config with invalid value for parameter with restrictions
    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));
    LocalConfig config(container);
    container.refreshAllParams();

    // Check if values were loaded
    const QStringList expected_value { "Military", "RequiresRelease", "InvalidValue" };
    QCOMPARE(config.m_flags, expected_value);

    // Save values back to the file without any changes
    AVConfig2Saver saver;
    saver.save(container);

    // Load the same config to a different container
    AVConfig2Container container2;
    QVERIFY(container2.loadConfigAbsolute(path));
    LocalConfig config2(container2);
    container2.refreshAllParams();

    // Compare if saved values are the same
    QCOMPARE(config.m_flags, config2.m_flags);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2Test::testEnvVarForCStyleVersionPinning()
{
    class LocalSubConfig : public AVConfig2
    {
    public:
        LocalSubConfig(QString prefix, AVConfig2Container& container) :
            AVConfig2(prefix, container)
        {
            registerParameter("parameter", &m_param, DEFAULT_HELP);
        }
        uint m_param;
    };
    class LocalParentConfig : public AVConfig2
    {
    public:
        LocalParentConfig(AVConfig2Container& container) :
            AVConfig2("parent", container)
        {
            registerSubconfig(getPrefix() + ".subconfigs", &m_subconfig_map);
        }
        AVConfig2Map<LocalSubConfig> m_subconfig_map;
    };

    const QString FILENAME = "saver_pinned_version_test";
    QString path = getTempConfigFileCopy(FILENAME);

    const QString file_content_before = fileContent(path);
    QVERIFY(!file_content_before.contains("subconfig_template"));

    AVConfig2Container container;
    QVERIFY(container.loadConfigAbsolute(path));
    LocalParentConfig config(container);
    container.refreshAllParams();
    QVERIFY(!config.m_subconfig_map.empty());

    // test AVCONFIG2_PIN_CSTYLE_VERSION environment variable with invalid values
    // pinned version should fall back to V_CURRENT and subconfig templates should be saved
    QVERIFY(AVEnvironment::setEnv(AVConfig2ExporterFactory::AVCONFIG2_PIN_CSTYLE_VERSION, "invalid"));
    AVConfig2Saver saver;
    saver.save(container);
    QVERIFY(fileContent(path).contains("subconfig_template"));

    QVERIFY(AVEnvironment::setEnv(AVConfig2ExporterFactory::AVCONFIG2_PIN_CSTYLE_VERSION, "-1"));
    saver.save(container);
    QVERIFY(fileContent(path).contains("subconfig_template"));

    QVERIFY(AVEnvironment::setEnv(AVConfig2ExporterFactory::AVCONFIG2_PIN_CSTYLE_VERSION,
                                  QString::number(AVConfig2ImporterCstyle::V_CURRENT + 1)));
    saver.save(container);
    QVERIFY(fileContent(path).contains("subconfig_template"));

    // test saver with old (V_2) pinned version by setting AVCONFIG2_PIN_CSTYLE_VERSION environment variable
    QVERIFY(AVEnvironment::setEnv(AVConfig2ExporterFactory::AVCONFIG2_PIN_CSTYLE_VERSION, "2"));
    saver.save(container);
    const QString file_content_after = fileContent(path);
    // initial file content and saved file content must match
    QCOMPARE(file_content_before, file_content_after);

    // override pinned version by env var with setPinnedCStyleVersion()
    AVConfig2ExporterFactory factory;
    factory.setPinnedCStyleVersion(3);
    saver.setExporterFactory(factory);
    saver.save(container);
    const QString file_content_subconfig_templates1 = fileContent(path);
    // check if subconfig_templates were saved
    QVERIFY(file_content_subconfig_templates1.contains("subconfig_template"));

    QVERIFY(qunsetenv(AVConfig2ExporterFactory::AVCONFIG2_PIN_CSTYLE_VERSION.toLatin1().data()));
}

///////////////////////////////////////////////////////////////////////////////

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVConfig2Test,"avlib/unittests/config")

// End of file
