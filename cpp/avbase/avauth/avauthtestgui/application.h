#pragma once

#include <avauthlib/avauthenticator.h>


class MainWindow;

//--------------------------------------------------------------------------------------------------
class Application
{
public:
    Application();
    ~Application();

    void start(std::unique_ptr<av::auth::AuthenticatorConfig> config);

private:
    void login();
    void logout();

    std::unique_ptr<av::auth::Authenticator> m_authenticator;
    QScopedPointer<MainWindow> m_main_window;

};  // class MainWindow
