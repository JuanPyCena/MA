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
#pragma once
#include <QTest>
#include <QTemporaryDir>

#include <avunittestmain.h>


#include "avconfig2.h"

namespace
{
const QString DEFAULT_HELP      = "no help";
const QString NESTED_PREFIX     = "test_config.nested_prefix";
const QString NESTED_SUB_PREFIX = "nested_sub_prefix";
}

//---------------------------------------------------------------------------

class AVConfig2Test : public QObject
{
   Q_OBJECT

private:
    /**
     * @brief getTempConfigFileCopy copies a given config file (located via AVConfig2 mechanisms) to a temporary location.
     * @param name The config file name, as understood by AVConfig2Container::locateConfig()
     * @return The path to the copy.
     */
    QString getTempConfigFileCopy(const QString& name);
    /**
     * @brief verifyEqualFileContent verifies that two config files are identical.
     * @param actual As understood by AVConfig2Container::locateConfig()
     * @param expected As understood by AVConfig2Container::locateConfig()
     */
    void verifyEqualFileContent(const QString& actual, const QString& expected);

    /**
     * @brief checkedRefresh is a minor helper method to print out any error which occured while refreshing a container.
     */
    Q_REQUIRED_RESULT static bool checkedRefresh(AVConfig2Container& container);

    //! Minor helper method to avoid metadata / storage location headaches when simply adding a test parameter.
    template <typename T>
    void addParameterToContainer(const QString& name, const T& value, AVConfig2Container& container);

private slots:
    void init();
    void cleanup();

    void testNestedConfigs();
    void testCmdlineSwitch();
    void testCmdlineSwitchExtraParams_data();
    void testCmdlineSwitchExtraParams();
    void testReference();
    void testAggregateReference();
    //! - reference to optional param
    //! - reference to param which was not yet registered, but specified on the commandline
    void testReference2();
    void testCommandLineOverride();
    void testSimpleSaver();
    void testSubconfigSave();
    //! Bug#5894
    void testMultipleInstances();
    //! SWE-4807
    void testConfigSave();

    void testDeprecatedNames();
    //! AC-108
    void testOptionalParamWithoutFile();
    //! SWE-3222 (register optional parameter first, then load configuration file)
    void testOptionalRegistrationFirst();
    //! SWE-2570 - setOptional breaks setDeprecatedName functionality
    //! SWE-4838 - Saving Config Entries with Deprecated Names Results in FATAL
    void testOptionalDeprecatedParameters();
    //! SWE-5652
    void testOptionalChangedInApplication();
    void testPureCmdlineSwitch();

    void testChangeSuggestedValues();

    void testEmptyDefines();
    void testNestedLevels();
    //! SWE-1452
    void testEmptyLinesInParameterValues();

    /**
     * This is a manual test because there is no way to prevent an application from crashing if there
     * is an error while loading the config file (see SWE-637).
     *
     * Uncomment the implementation to execute the test.
     */
    void testSuddenEnd();

    void testInheritance();
    void testInheritanceWithFacts();
    //! SWE-5787
    void testInheritanceWithCmdlineOverrides();
    void testInheritanceMetadata();
    //! SWE-3419 - fix references and inheritance
    //! SWE-3465 - fix overriding an inherited parameter (via test data)
    //! SWE-3496 - bug with references and inheritance
    void testInheritanceWithReferences();
    //! SWE-6220 - avconfig2 reference to reference incorrectly resolved
    void testInheritanceWithReferences2();

    //! SWE-5537 - subconfig map refresh failure
    //! Calling refreshParams() in the config constructors led to an empty config map after refreshing.
    void testSubconfigs();

    //! Reproduces SWE-2111
    void testSubconfigWithOnlyReferences();

    //! SWE-2406
    void testSavingNewConfigMap();

    //! SWE-3442
    void testReferenceToken();

    //! SWE-3946
    void testCommandlineWithMissingParam();

    void testEnumConfig();

    //! SWE-4447
    void testGetTypeName();

    //! SWE-5315
    void testUtf8Encoding();

    //! SWE-5437. Verify that config object creation in postRefresh() and config constructor implementations don't
    //! mess up the refreshAll() implementation.
    void testCreateDuringRefreshAll();

    //! SWE-5465
    void testNamespaceOrderOnSave();

    void testUnitTestConstructor();

    void testConfigSingleton();

    //! SWE-5739
    void testRefreshParameters();

    //! ELSIS-631
    void testSubconfigTemplate();
    void testSubconfigTemplateInstantiation();
    /**
     * Verifies that handling of multiple subconfig template instances in different files works correctly.
     * This can be the case when editing files for different process instances in the parameter editor, e.g.
     * fdp2_diflis vs fdp2_acemax or different instances of radar DAQs.
     */
    void testSubconfigTemplateDuplication();

    //! ELSIS-632
    void testRegisterSubconfigForTemplateCreation();
    void testRegisterSubconfigRecursiveForTemplateCreation();

    //! ELSIS-633
    void testSubconfigTemplateSave();
    void testSubconfigTemplateSaveWithSimpleSaver();
    void testSubconfigTemplateDuplicationSave();
    void testMissingSubconfigTemplates();

    //! ELSIS-634
    void testRegisterSameMemberWithDifferentName();
    void testRegisterSubconfigWithSameName();

    //! ELSIS-1038
    void testRemovingParametersFromConfigFile();

    //! ELSIS-1338
    void testCStyleVersionPinning();

    //! ELSIS-1358
    void testSavingInvalidValueForParamWithRestrictions();

    //! ELSIS-1421
    void testEnvVarForCStyleVersionPinning();

private:

    //! Recreate temporary directory for each test to avoid conflicts.
    //! Alternatively, we could work with temporary files...
    QTemporaryDir* m_temp_dir = nullptr;
};

//---------------------------------------------------------------------------

class TestParamImporterBase : public AVConfig2ImporterBase
{
public:
    explicit TestParamImporterBase(AVConfig2Container& container);
protected:
    template<typename T>
    void addParameter(const QString& name,const T& value,
                      const QString& cmdline_switch = QString::null)
    {
        QString type_string = AVConfig2Types::getTypeName<T>();
        QVERIFY(!type_string.isEmpty());
        AVConfig2Metadata metadata(name,type_string, DEFAULT_HELP);
        metadata.m_command_line_switch = cmdline_switch;
        m_client->addParameter(AVToString(value),metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
    }
};

//---------------------------------------------------------------------------
//! This importer fills up the AVConfig2Container object with some parameters
//! required for subsequent tests. Avoids having to create real config files for the tests...
class TestParamImporter : public TestParamImporterBase
{

public:
    explicit TestParamImporter(AVConfig2Container& container);

    void import() override;
};

//---------------------------------------------------------------------------

class TestReference2ParamImporter : public TestParamImporterBase
{

public:
    explicit TestReference2ParamImporter(AVConfig2Container& container);

    void import() override;
};

//---------------------------------------------------------------------------

class OptionalParamTestImporter : public TestParamImporterBase
{

public:
    explicit OptionalParamTestImporter(AVConfig2Container& container);

    void import() override;
};

//---------------------------------------------------------------------------

class NestedSubConfig : public AVConfig2
{
public:

    NestedSubConfig(const QString& prefix, AVConfig2Container& config);

    double m_double;
    QString m_pure_cmdline;
};

//---------------------------------------------------------------------------

class NestedConfig : public AVConfig2
{
public:

    NestedConfig(const QString& prefix, AVConfig2Container& config);

    QString                    m_string;
    int                        m_int;
    AVConfig2Map<NestedSubConfig> m_sub_cfg;
};

//---------------------------------------------------------------------------

class TestConfig : public AVConfig2
{
public:
    explicit TestConfig(AVConfig2Container& container);

    QString                 m_test_string;
    uint                    m_port;
    AVConfig2Map<NestedConfig> m_sub_cfg;
};

//---------------------------------------------------------------------------

class CmdlineTestConfig : AVConfig2
{
public:
    explicit CmdlineTestConfig(AVConfig2Container& config);

    uint m_port;
};

//---------------------------------------------------------------------------

class OptionalCmdlineTestConfig : AVConfig2
{
public:
    explicit OptionalCmdlineTestConfig(AVConfig2Container& config);

    void changeSuggestedValues();

    QString m_optional_param;
    QString m_optional_param_dev_override;
};

//---------------------------------------------------------------------------

class DeprecatedTestConfig : AVConfig2
{
public:
    explicit DeprecatedTestConfig(AVConfig2Container& config);

    uint m_port1;
    uint m_port2;
    uint m_port3;
    uint m_port4;
};

//---------------------------------------------------------------------------

namespace avconfig2_enum_test_namespace
{

#define AVConfig2TestEnumValues(enum_value) \
    enum_value(E1) \
    enum_value(E2)
DEFINE_TYPESAFE_ENUM_IN_NAMESPACE(TestEnum, AVConfig2TestEnumValues)
}

class EnumTestConfig : public AVConfig2
{
    Q_GADGET
public:
    explicit EnumTestConfig(AVConfig2Container& config);

    enum class ValueQEnum {
        Value1,
        Value2,
        Value3
    };
    Q_ENUM(ValueQEnum)

    ValueQEnum m_qenum_value;

#define AVENUM_DEF(v) \
    v(TestValue1,) \
    v(TestValue2,)

    DEFINE_TYPESAFE_ENUM(ValueAVEnum, AVENUM_DEF)
    ValueAVEnum m_avenum_value;

    avconfig2_enum_test_namespace::TestEnum m_namespace_avenum_value;
};

//---------------------------------------------------------------------------

template <typename T>
void AVConfig2Test::addParameterToContainer(const QString &name, const T &value, AVConfig2Container &container)
{
    AVConfig2Metadata metadata(name, AVConfig2Types::getTypeName<T>(), DEFAULT_HELP);
    container.addParameter(AVToString(value), metadata, AVConfig2StorageLocation(AVConfig2StorageLocation::PS_FILE_ORDINARY));
}

// End of file
