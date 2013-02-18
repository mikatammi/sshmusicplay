#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>

extern "C"
{
#include <libssh/libssh.h>
#include <libssh/sftp.h>
}

#include <cstdlib>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow),
      sshsession_(),
      filelist_model_(this)
{
    ui_->setupUi(this);
    ui_->treeView->setModel(&filelist_model_);

    connect(ui_->btnConnect, SIGNAL(clicked()), SLOT(doConnect()));
}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::setOrientation(ScreenOrientation orientation)
{
    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION < 0x040702
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif // QT_VERSION < 0x040702
    };
    setAttribute(attribute, true);
}

void MainWindow::showExpanded()
{
#if defined(Q_WS_SIMULATOR)
    showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    showMaximized();
#else
    show();
#endif
}

void MainWindow::doConnect()
{
    // Username
    QString username = ui_->txtUsername->text();

    qDebug() << "Connecting with username: " << username;

    if (!sshsession_.connect(ui_->txtHostname->text(),
                             atoi(ui_->txtPort->text().
                                  toStdString().c_str()),
                             username))
    {
        ui_->lblStatus->setText("Connect failed");
        return;
    }

    if(!sshsession_.authenticate_try_password_methods(
                ui_->txtPassword->text()))
    {
        ui_->lblStatus->setText("Authentication failed");
        sshsession_.disconnect();
        return;
    }

    if(!sshsession_.ssh_sftp_init())
    {
        ui_->lblStatus->setText("SFTP init failed");
        sshsession_.disconnect();
        return;
    }

    QStringList filelist =
            sshsession_.collectFilelist(ui_->txtDirectory->text());

    ui_->stackedWidget->setCurrentWidget(ui_->pageDirectory);

    foreach(const QString& filename, filelist)
    {
        filelist_model_.appendRow(new QStandardItem(filename));
    }


    ui_->lblStatus->setText("Connection success");
}
