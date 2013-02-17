#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QMessageBox>

#include <libssh/libssh.h>
#include <cstdlib>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnConnect, SIGNAL(clicked()), SLOT(doConnect()));
}

MainWindow::~MainWindow()
{
    delete ui;
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
    ui->lblStatus->setText("Connecting...");

    ssh_session my_ssh_session = ssh_new();

    if(my_ssh_session == NULL)
    {
        ui->lblStatus->setText("Failed to create session");
        return;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST,
                    ui->txtHostname->text().toStdString().c_str());

    int verbosity = SSH_LOG_PROTOCOL;
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    int port = 22;
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);

    int rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK)
    {
        ui->lblStatus->setText(
                    QString("Failed to connect:")
                    + ssh_get_error(my_ssh_session));

        ssh_free(my_ssh_session);
        return;
    }

    ui->lblStatus->setText("Success! Disconnecting.");

    // TODO: Show question regarding correct host key
    //QMessageBox::question(this, "HahaaTitle", "HahaaTeksti",
    //    QMessageBox::Yes || QMessageBox::No || QMessageBox::Cancel);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
}
