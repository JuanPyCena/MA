#include "mainwindow.h"
#include "avlogmodel.h"

#include <avauthlib/avidtoken.h>

#include <sstream>

MainWindow::MainWindow() : mMainWindow(new QMainWindow), mUIMainWindow(new Ui::MainWindow), mLogModel()
{
    mUIMainWindow->setupUi(mMainWindow);

    QObject::connect(mUIMainWindow->actionLogin, &QAction::triggered, mMainWindow,
                     [this](bool) { emit this->loginClicked(); }, Qt::AutoConnection);
    QObject::connect(mUIMainWindow->actionLogout, &QAction::triggered, mMainWindow,
                     [this](bool) { emit this->logoutClicked(); }, Qt::AutoConnection);
    QObject::connect(mUIMainWindow->actionExit, &QAction::triggered, mMainWindow,
                     [this](bool) { emit this->exitClicked(); }, Qt::AutoConnection);

    mLogModel = new av::auth::LogModel(mUIMainWindow->logTableView);
    mUIMainWindow->logTableView->setModel(mLogModel);

    /*
    auto log = av::auth::Log::defaultInstance().get();
    QObject::connect(log, &av::auth::Log::messageSent,
                     mMainWindow,
                     [this](QString const & message, av::auth::LogSeverity severity)
                     { mLogModel->addMessage(message, severity); },
                     Qt::AutoConnection);

    QObject::connect(
        mUIMainWindow->fatalToggle, &QCheckBox::stateChanged, mLogModel,
        [this](int state) {
            mLogModel->setSeverityState(av::auth::LogSeverity::Fatal, state > 0);
        });
    QObject::connect(
        mUIMainWindow->errorToggle, &QCheckBox::stateChanged, mLogModel,
        [this](int state) {
            mLogModel->setSeverityState(av::auth::LogSeverity::Error, state > 0);
        });
    QObject::connect(
        mUIMainWindow->warningToggle, &QCheckBox::stateChanged, mLogModel,
        [this](int state) {
            mLogModel->setSeverityState(av::auth::LogSeverity::Warn, state > 0);
        });
    QObject::connect(
        mUIMainWindow->infoToggle, &QCheckBox::stateChanged, mLogModel,
        [this](int state) {
            mLogModel->setSeverityState(av::auth::LogSeverity::Info, state > 0);
        });
    QObject::connect(
        mUIMainWindow->debugToggle, &QCheckBox::stateChanged, mLogModel,
        [this](int state) {
            mLogModel->setSeverityState(av::auth::LogSeverity::Debug, state > 0);
        });

    */

    QObject::connect(mUIMainWindow->clearButton, &QPushButton::clicked, mLogModel, [this]() { mLogModel->clear(); });
}

void MainWindow::show()
{
    mMainWindow->show();
}

QMainWindow *MainWindow::getWidget()
{
    return mMainWindow;
}

void MainWindow::updateTokenInfo(const av::auth::IdToken &id_token)
{
    getUI()->userNameText->setText(id_token.preferred_username);
    getUI()->rolesText->setText(AVToString(id_token.roles));
}


