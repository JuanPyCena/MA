#ifndef AVCONFIG2_BENCHMARK_TEST
#define AVCONFIG2_BENCHMARK_TEST

#include <QTest>
#include <QTemporaryDir>

#include "avconfig2.h"

//---------------------------------------------------------------------------

class AVConfig2BenchmarkTest : public QObject
{
    Q_OBJECT
public:

private slots:
    void initTestCase();

    void testBenchmarkLoadingConfigMap_data();
    void testBenchmarkLoadingConfigMap();

private:
    void saveNestedConfigMap(const unsigned int& nested_maps_count);

private:
    QTemporaryDir m_temp_dir;

};

//---------------------------------------------------------------------------

class ConfigNode : public AVConfig2
{
public:
    explicit ConfigNode(const QString& prefix, AVConfig2Container& config);

    QString                  m_string;
    int                      m_int;
    AVConfig2Map<ConfigNode> m_sub_cfg;
};

//---------------------------------------------------------------------------

class ConfigRoot : public AVConfig2
{
public:
    explicit ConfigRoot(AVConfig2Container& container);

    QString                  m_test_string;
    uint                     m_port;
    AVConfig2Map<ConfigNode> m_sub_cfg;
};

//---------------------------------------------------------------------------

#endif // AVCONFIG2_BENCHMARK_TEST
