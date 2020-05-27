#include "application.h"
#include "mainwindow.h"

#include "avlog.h"

#include <QApplication>
#include <QMessageBox>

//--------------------------------------------------------------------------------------------------

Application::Application()
{
    // MainWindow: Create and setup connections
    m_main_window.reset(new MainWindow);
    QObject::connect(m_main_window.get(), &MainWindow::loginClicked, qApp, [this] { login(); });
    QObject::connect(m_main_window.get(), &MainWindow::logoutClicked, qApp, [this] { logout(); });
    QObject::connect(m_main_window.get(), &MainWindow::exitClicked, qApp, []() { qApp->exit(0); });

    AVLogInfo << "ADB Safegate: Authentication PoC Application starting";
}

//--------------------------------------------------------------------------------------------------

Application::~Application()
{
}

//--------------------------------------------------------------------------------------------------

void Application::start(std::unique_ptr<av::auth::AuthenticatorConfig> config)
{
    m_authenticator = std::make_unique<av::auth::Authenticator>(std::move(config));

    QObject::connect(m_authenticator.get(), &av::auth::Authenticator::stateChanged, m_authenticator.get(),
                     [this](av::auth::Authenticator::State state) {
                         AVLogInfo << "Authenticator status changed to: " <<state;

                         switch (state)
                         {
                             case av::auth::Authenticator::State::Invalid:
                                 // TEMPORARY
                                 m_main_window->updateTokenInfo(m_authenticator->getIdToken());
                                 break;
                             case av::auth::Authenticator::State::Authenticated:
                                 m_main_window->updateTokenInfo(m_authenticator->getIdToken());
                                 break;
                             default:
                                 break;
                         }
    });

    m_main_window->show();
}

//--------------------------------------------------------------------------------------------------

void Application::login()
{
    if (m_authenticator->getState() > av::auth::Authenticator::State::ServicesRead)
    {
        QMessageBox::information(m_main_window->getWidget(), QObject::tr("Authentication processing"),
                                 QObject::tr("Already authenticated or in the process of authenticating."),
                                 QMessageBox::Ok);
        return;
    }

    m_authenticator->authenticate();

    // TODO: Show some message or status bar depending on outcome results;
}

//--------------------------------------------------------------------------------------------------

void Application::logout()
{
    AVLogFatal << "not yet implemented";

    if (m_authenticator->getState() < av::auth::Authenticator::State::Authenticated)
    {
        QMessageBox::information(m_main_window->getWidget(), QObject::tr("Not logged in"),
                                 QObject::tr("No valid User ID are currently present."), QMessageBox::Ok);
        return;
    }

//    bool result = m_authenticator->deauthenticate();
//    (void)result;
}
