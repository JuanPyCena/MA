
// Qt includes
#include <QCoreApplication>
#include <avconsole.h>

// AviBit common includes
#include "avconfig2.h"
#include "avdaemon.h"
#include "avenvironment.h"
#include "avlog.h"

#include "avauthenticator.h"



//--------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    AVApplicationInitParams init_params(argc, argv, AVEnvironment::APP_AVCOMMON, "avauthtestcli");
    init_params.disableTimeReferenceAdapter();
    AVApplicationPreInit(init_params);

    QApplication app(argc, argv);
    AVApplicationInit(init_params);
    AVLogger->setMinLevel(AVLog::LOG__DEBUG);

    auto config = std::make_unique<av::auth::AuthenticatorConfig>("avauthtest");

    AVConfig2Global::singleton().loadConfig("avauthtest");
    AVConfig2Global::singleton().setApplicationHelpText("This application is used for (manual) testing of the "
                                                        "avauth library functionality.");

    int result = 0;
    if (AVConfig2Global::singleton().refreshCoreParameters())
    {
        auto authenticator = std::make_unique<av::auth::Authenticator>(std::move(config));

        AVConsole::singleton().registerCallback("logout", authenticator.get(),   [auth = authenticator.get()](auto&, auto&) {auth->logout();},
        "Logs out", [](auto&,auto&){} );
        AVConsole::singleton().registerCallback("authenticate", authenticator.get(),   [auth = authenticator.get()](auto&, auto&) {auth->authenticate();},
        "Start authentication", [](auto&,auto&){} );
        AVConsole::singleton().registerCallback("getState", authenticator.get(),   [auth = authenticator.get()](auto& cc, auto&) { cc.printSuccess("State: " + AVToString(auth->getState()));},
        "Prints current authenticator state.", [](auto&,auto&){} );

        QTimer::singleShot(0, authenticator.get(), &av::auth::Authenticator::authenticate);
        QObject::connect(authenticator.get(), &av::auth::Authenticator::authenticated, [&]() {

            AVLogInfo << "id token roles: " << authenticator->getIdToken().roles;
        });

        QObject::connect(authenticator.get(), &av::auth::Authenticator::error, [](auto error) {
            AVLogError << "Error: " << error;
        });

        result = app.exec();
    }

    AVDaemonDeinit();
    return result;
}

// End of file
