#pragma once

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QScopedPointer>

namespace av
{
namespace auth
{
class LogModel;
struct IdToken;
}
}

class MainWindow : public QObject
{
    Q_OBJECT
public:
    MainWindow();

    void show();

    QMainWindow*    getWidget();
    Ui::MainWindow* getUI() { return mUIMainWindow.data(); }

    void updateTokenInfo(av::auth::IdToken const& id_token);

Q_SIGNALS:
    void loginClicked();
    void logoutClicked();
    void exitClicked();

private:
    QMainWindow*                   mMainWindow;
    QScopedPointer<Ui::MainWindow> mUIMainWindow;
    av::auth::LogModel*            mLogModel;

};  // class MainWindow
