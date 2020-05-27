///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// Copyright: AviBit data processing GmbH, 2001-2012
//
// Module:    TESTS - AVCOMMON Unit Tests
//
///////////////////////////////////////////////////////////////////////////////
  
/*! \file
    \author  Bichler Christoph, c.bichler@avibit.com
    \brief   Defines for QTestlib unittestcases
 */
  
#if !defined TESTDEFINES_INCLUDED
#define TESTDEFINES_INCLUDED
  
#include <QTest>

#include "avdaemon.h"
#include "avconfig.h" // TODO: remove support for AVConfig1 if all tests are ported
#include "avconfig2.h"
#include "avpackageinfo.h"

#include "avunittesthelpers.h"

#include "avunittesthelperlib_export.h"

// system includes
#include <cstring>

namespace
{
const QString APPLICATION_NAME("AVIBIT");
}

///////////////////////////////////////////////////////////////////////////////
  
#define AV_QTEST_MAIN(x) AV_QTEST_MAIN_IMPL(x, false)
  
#define AV_QTEST_MAIN_GUI(x) AV_QTEST_MAIN_IMPL(x, true)
  
#define AV_QTEST_MAIN_IMPL(TestObject, IS_GUI) \
int main(int argc, char *argv[]) \
{ \
    return qtestMainAvDaemonInit<TestObject>(argc, argv, IS_GUI, #TestObject); \
}
  
///////////////////////////////////////////////////////////////////////////////
  
#define AV_QTEST_MAIN_AVLIB(x) AV_QTEST_MAIN_AVCONFIG2_IMPL(x, false, QString("/src") + qVersion()[0] + "/avlib/unittests/config")
#define AV_QTEST_MAIN_AVCONFIG2(x) AV_QTEST_MAIN_AVCONFIG2_IMPL(x, false, "")
#define AV_QTEST_MAIN_AVCONFIG2_WITH_CONFIGDIR(x, CONFIG_DIR) AV_QTEST_MAIN_AVCONFIG2_IMPL(x, false, CONFIG_DIR)
#define AV_QTEST_MAIN_AVCONFIG2_GUI(x) AV_QTEST_MAIN_AVCONFIG2_IMPL(x, true, QString("/src") + qVersion()[0] + "/unittests/config")
  
//! runs a qtestlib test initialized using AVConfig2
#define AV_QTEST_MAIN_AVCONFIG2_IMPL(TestObject, IS_GUI, CONFIG_DIR) \
    int main(int argc, char *argv[]) \
{ \
    return qtestMainAvconfig2<TestObject>(argc, argv, IS_GUI, #TestObject, CONFIG_DIR); \
}

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief setModuleTestConfigDirectory is a helper method to set the unit test config search
 *        path relative to the module's RESOURCE path. It is useful only for modules
 *        (not for unit tests in SVN).
 *
 * *ATTENTION* The Qt3 implementation is in AVUnitTestLauncher::setModuleTestConfigDirectory.
 *
 * Ensures that the directory actually exists and is readable.
 *
 * @param package_name The name of the package as obtained from AVPackageInfo::getName().
 * @param dir The relative config directory, e.g. "avlib/unittests/config".
 */
void AVUNITTESTHELPERLIB_EXPORT setModuleTestConfigDirectory(const QString& package_name, const QString &dir);

///////////////////////////////////////////////////////////////////////////////

template<typename TestClass>
int qtestMainAvconfig2(int argc, char *argv[], bool is_gui, const QString& process_name, const QString& config_dir)
{
    AVEnvironment::setEnv("ENABLE_AVCONSOLE_STDIN","0"); /*disabled to avoid scrambled output*/
    AVApplicationInitParams init_params(0, nullptr, APPLICATION_NAME, process_name);
    init_params.disableTimeReferenceAdapter();

    // If QtCreator runs the test, stdout has to be disabled (otherwise xml output is scrambled)
    if (argc >= 2 && !std::strcmp(argv[1], "-xml"))
        init_params.disableStdoutLogging();

    AVUnittestHelpers::setTemporaryHome(APPLICATION_NAME);

    AVApplicationPreInit(init_params);
    AVLogInfo << APPLICATION_NAME << "_HOME and current working directory are " << AVEnvironment::getApplicationHome();
    if(!config_dir.isEmpty())
    {
        setModuleTestConfigDirectory(AVPackageInfo::getName(), QString(config_dir));
    }
    int ret = 0;
    QCoreApplication *app = nullptr;
    if(is_gui)
        app = new QApplication(argc, argv);
    else
        app = new QCoreApplication(argc, argv);
    AVApplicationInit(init_params);
    if (AVConfig2Global::singleton().refreshCoreParameters())
    {
        TestClass tc;
        qDebug("CTEST_FULL_OUTPUT");
        ret = QTest::qExec(&tc, argc, argv);
    }
    AVDaemonDeinit();
    delete app;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template<typename TestClass>
int qtestMainAvDaemonInit(int argc, char *argv[], bool is_gui, const QString& process_name)
{
    AVEnvironment::setEnv("ENABLE_AVCONSOLE_STDIN","0"); /*disabled to avoid scrambled output*/
    QCoreApplication *app = nullptr;
    if(is_gui)
        app = new QApplication(argc, argv);
    else
        app = new QCoreApplication(argc, argv);
    AVEnvironment::setApplicationName(APPLICATION_NAME);
    QString home = AVEnvironment::getApplicationHome();
    AVEnvironment::setApplicationName(APPLICATION_NAME + "_UNITTEST");
    // If QtCreator runs the test, stdout has to be disabled (otherwise xml output is scrambled)
    const bool log_to_stdout = argc < 2 || std::strcmp(argv[1], "-xml");
    AVDaemonInit(UseEventLoop, process_name, "", log_to_stdout);
    QChar QT_MAJOR_VERSION = qVersion()[0];
    AVEnvironment::setApplicationHome(home + "/src" + QT_MAJOR_VERSION + "/unittests");
    AVDaemonProcessStateInit();
    int ret;
    { /* This is in own scope so TestObject cleanup happens with logger etc still present */
        TestClass tc;
        qDebug("CTEST_FULL_OUTPUT");
        ret = QTest::qExec(&tc, argc, argv);
    }
    AVDaemonDeinit();
    delete app;
    return ret;
}

#endif // TESTDEFINES_INCLUDED
  
// End of file
