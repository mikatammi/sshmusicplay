#include "connectwindow.hpp"
#include "ui_connectwindow.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>

extern "C"
{
#include <libssh/libssh.h>
#include <libssh/sftp.h>
}

#include <cstdlib>

ConnectWindow::ConnectWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);

    connect(ui->btnConnect, SIGNAL(clicked()), SLOT(doConnect()));
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}

void ConnectWindow::setOrientation(ScreenOrientation orientation)
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

void ConnectWindow::showExpanded()
{
#if defined(Q_WS_SIMULATOR)
    showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    showMaximized();
#else
    show();
#endif
}

void ConnectWindow::doConnect()
{
    // TODO: After testing, move this stuff to ssh session class

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


    // TODO: Authenticate server
    // TODO: Show question regarding correct host key
    //QMessageBox::question(this, "HahaaTitle", "HahaaTeksti",
    //    QMessageBox::Yes || QMessageBox::No || QMessageBox::Cancel);

    /*
    rc = ssh_userauth_password(my_ssh_session,
                               ui->txtUsername->text().toStdString().c_str(),
                               ui->txtPassword->text().toStdString().c_str());
    if (rc != SSH_AUTH_SUCCESS)
    {
        ui->lblStatus->setText(QString("Authentication error:")
                               + ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }
    */

    rc = ssh_userauth_kbdint(my_ssh_session,
                       ui->txtUsername->text().toStdString().c_str(),
                       NULL);
    while (rc == SSH_AUTH_INFO)
    {
        const char *name;
        const char *instruction;
        int nprompts, iprompt = 0;

        name = ssh_userauth_kbdint_getname(my_ssh_session);
        instruction = ssh_userauth_kbdint_getinstruction(my_ssh_session);
        nprompts = ssh_userauth_kbdint_getnprompts(my_ssh_session);

        qDebug() << "Name Instruction Nprompts";
        qDebug() << name << instruction << nprompts;

        if (nprompts > 0)
        {
            const char *prompt;
            char echo;

            prompt = ssh_userauth_kbdint_getprompt(my_ssh_session, 0, &echo);

            qDebug() << prompt;

            if(QString(prompt) == "Password: ")
            {
                if (ssh_userauth_kbdint_setanswer(
                            my_ssh_session, iprompt,
                            ui->txtPassword->text().toStdString().c_str()) < 0)
                {
                    ssh_disconnect(my_ssh_session);
                    ssh_free(my_ssh_session);
                    return;
                }
            }
            else
            {
                ssh_disconnect(my_ssh_session);
                ssh_free(my_ssh_session);
                return;
            }
        }

        rc = ssh_userauth_kbdint(my_ssh_session,
                           ui->txtUsername->text().toStdString().c_str(),
                           NULL);
    }

    // TODO: Maybe some other stuff
    sftp_session sftp;
    sftp = sftp_new(my_ssh_session);
    if (sftp == NULL)
    {
        ui->lblStatus->setText(QString("Error allocating SFTP session.")
                               + ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    rc = sftp_init(sftp);
    if (rc != SSH_OK)
    {
        ui->lblStatus->setText(QString("Error initializing SFTP session.")
                               + ssh_get_error(my_ssh_session));
        sftp_free(sftp);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    sftp_dir dir;
    sftp_attributes attributes;

    dir = sftp_opendir(sftp, "/tmp");
    if (!dir)
    {
        ui->lblStatus->setText(QString("Directory not opened: ")
                               + ssh_get_error(my_ssh_session));
        sftp_free(sftp);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return;
    }

    attributes = sftp_readdir(sftp, dir);
    attributes = sftp_readdir(sftp, dir);
    attributes = sftp_readdir(sftp, dir);
    attributes = sftp_readdir(sftp, dir);

    ui->lblStatus->setText(QString("Success! Disconnecting.")
                           + attributes->name);

    sftp_closedir(dir);
    if (rc != SSH_OK)
    {
        ui->lblStatus->setText(QString("Cannot close dir.")
                               + ssh_get_error(my_ssh_session));
    }

    sftp_free(sftp);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
}
