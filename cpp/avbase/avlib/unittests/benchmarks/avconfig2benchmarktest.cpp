#include "avconfig2benchmarktest.h"

#include <iostream>

#include "avunittestmain.h"
#include "avconfig2saver.h"

// the target total number of sub_configs for benchmark testing, can be increased at will
const unsigned int NESTED_SUB_CONFIG_COUNT(5000);

// target file name, loaded for benchmark testing, example: testSavingNestedConfigMap5000
const QString CONFIG_FILE_NAME("testSavingNestedConfigMap" + QString::number(NESTED_SUB_CONFIG_COUNT));

// used to separate namespace names in config file, example: parent_namespace.child_namespace
const QChar DIVIDER_DOT('.');

namespace
{
    const QString DEFAULT_HELP      = "no help";
    const QString NESTED_PREFIX     = "test_config.nested_prefix";
    const QString NESTED_SUB_PREFIX = "nested_sub_prefix";
}

///////////////////////////////////////////////////////////////////////////////

bool loadConfigFileOnly(const QString& full_file_path)
{
    AVConfig2Container container;
    ConfigRoot test_config(container);
    Q_UNUSED(test_config);

    return container.loadConfigAbsolute(full_file_path);
}

///////////////////////////////////////////////////////////////////////////////

bool loadConfigFileAndRefreshParams(const QString& full_file_path)
{
    AVConfig2Container container;
    ConfigRoot test_config(container);

    if (container.loadConfigAbsolute(full_file_path))
    {
        AVConfig2Container::RefreshResult result = container.refreshAllParams();
        return result.refreshSuccessful();
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * This is just proof-of-concept, follow up for displaying this in CDash is tracked in SWE-5671.
 */
void exportBenchmarkResultToCDash(const int& elapsed_time_mili_seconds)
{
    // Because QTest executes a test case twice,
    // (see https://stackoverflow.com/questions/36916962/qtest-executes-test-case-twice)
    // is_called_for_second_time flag is used to export Benchmark result only once upon the second call
    static bool is_called_for_second_time(false);

    if (is_called_for_second_time)
    {
        const double seconds = elapsed_time_mili_seconds / 1000.0;

        /*
           syntax required by CDash server:
                sequence mode
                <DartMeasurement name="Benchmark" type="numeric/double">5.723</DartMeasurement>

           if export multiple names, all <DartMeasurement> tags are in 1 line:
                sequence mode
                <DartMeasurement name="Benchmark1"/><DartMeasurement name="Benchmark2"/>

           expectation: on CDash's test passed page, appears:
                Benchmark 5.723
                Display graphs: dropdown with [Benchmark] choice
                Graphical Chart shows a representation of value 5.723
        */

        const QString output_to_cdash("<DartMeasurement name=\"Benchmark\" type=\"numeric/double\">"
                                      + QString::number(seconds)
                                      + "</DartMeasurement>");
        // std::cout prints Benchmark result in a way that will be processed by Bamboo build-server,
        // and will later on be picked up by CDash server for reporting test results
        std::cout << "sequence mode" << std::endl;
        std::cout << output_to_cdash.toStdString().c_str() << std::endl;
    } else
    {
        is_called_for_second_time = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2BenchmarkTest::saveNestedConfigMap(const unsigned int& nested_maps_count)
{
    const QString temp_dir_path(m_temp_dir.path());
    const QString full_file_path(temp_dir_path + QDir::separator() + CONFIG_FILE_NAME + ".cc");

    // save a config file
    QFile test_file(full_file_path);
    if (! test_file.exists() || test_file.remove())
    {
        AVConfig2Container container;
        ConfigRoot test_config(container);

        QString config_key;
        for (unsigned int i = 1; i <= nested_maps_count; ++i)
        {
            config_key = "new_sub" + QString::number(i);
            QSharedPointer<ConfigNode> nested(new ConfigNode(NESTED_PREFIX + DIVIDER_DOT + config_key, container));

            nested->m_int = 100 + i;
            test_config.m_sub_cfg.insert(config_key, nested);
        }
        container.setStorageLocation(AVConfig2StorageLocation(temp_dir_path, CONFIG_FILE_NAME, CF_CSTYLE));

        AVConfig2Saver saver;
        saver.setSaveValueMode(AVConfig2Saver::SVM_ADOPT_PROCESS_VALUES);
        saver.save(container);
    }
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2BenchmarkTest::initTestCase()
{
    saveNestedConfigMap(NESTED_SUB_CONFIG_COUNT);
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2BenchmarkTest::testBenchmarkLoadingConfigMap_data()
{
    QTest::addColumn<bool>("load_file_only");
    QTest::newRow("load config file only") << true;
    QTest::newRow("load file and refresh params") << false;
}

///////////////////////////////////////////////////////////////////////////////

void AVConfig2BenchmarkTest::testBenchmarkLoadingConfigMap()
{
    const QString full_file_path(m_temp_dir.path() + QDir::separator() + CONFIG_FILE_NAME + ".cc");

    QFETCH(bool, load_file_only);

    if (load_file_only)
    {
        QBENCHMARK
        {
            QVERIFY(loadConfigFileOnly(full_file_path));
        }
    }
    else
    {
        QTime timer;
        timer.start();

        // Benchmark block is between timer.start() and timer.elapsed()
        QVERIFY(loadConfigFileAndRefreshParams(full_file_path));

        const int elapsed_time = timer.elapsed();

        // setBenchmarkResult(), similar to QBENCHMARK macro, prints Benchmark result on Qt console or Terminal
        QTest::setBenchmarkResult(elapsed_time, QTest::WalltimeMilliseconds);

        exportBenchmarkResultToCDash(elapsed_time);
    }
}

///////////////////////////////////////////////////////////////////////////////

ConfigNode::ConfigNode(const QString &prefix, AVConfig2Container &config)
    : AVConfig2(prefix, config)
    , m_int(0)
{
    setHelpGroup("");
    QVERIFY(prefix.startsWith("test_config"));

    registerParameter("string", &m_string, DEFAULT_HELP);
    registerParameter("int", &m_int, DEFAULT_HELP);
    registerSubconfig(m_prefix + DIVIDER_DOT + NESTED_SUB_PREFIX, &m_sub_cfg);
}

///////////////////////////////////////////////////////////////////////////////

ConfigRoot::ConfigRoot(AVConfig2Container &container)
    : AVConfig2("test_config", container)
    , m_port(0)
{
    setHelpGroup("Test");

    registerParameter("test_string", &m_test_string, DEFAULT_HELP);
    registerParameter("port", &m_port, DEFAULT_HELP).setCmdlineSwitch("port");
    registerSubconfig(NESTED_PREFIX, &m_sub_cfg);
}

AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(AVConfig2BenchmarkTest, "avlib/unittests/benchmarks")

// End of file
