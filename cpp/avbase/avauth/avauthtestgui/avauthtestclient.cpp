
///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON
// Copyright: AviBit data processing GmbH, 2001-2010
// QT-Version: QT4
//
// Module:    HOUSEKEEPER - Modular Housekeeper
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author   Christian Muschick, c.muschick@avibit.com
    \author   QT4-PORT: Christoph Bichler, c.bichler@avibit.com
    \brief    Modular housekeeper.
*/

// Qt includes
#include <QCoreApplication>

// AviBit common includes
#include "avconfig2.h"
#include "avdaemon.h"
#include "avenvironment.h"
#include "avlog.h"

#include "application.h"
#include "avauthenticator.h"

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    AVApplicationInitParams init_params(argc, argv, AVEnvironment::APP_AVCOMMON, "avauthtestclient");
    init_params.disableTimeReferenceAdapter();
    AVApplicationPreInit(init_params);

    QApplication app(argc, argv);

    AVApplicationInit(init_params);

    AVConfig2Global::singleton().loadConfig("avauthtest");
    auto config = std::make_unique<av::auth::AuthenticatorConfig>("avauthtest");

    Application application;

    int result = 0;
    if (AVConfig2Global::singleton().refreshCoreParameters())
    {
        application.start(std::move(config));
        result = app.exec();
    }

    AVDaemonDeinit();
    return result;
}

// End of file
