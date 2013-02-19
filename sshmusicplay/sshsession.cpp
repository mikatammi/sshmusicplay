#include "sshsession.hpp"
#include "sshfile.hpp"

#include <QDebug>

SSHSession::SSHSession() :
    connected_(false),
    authenticated_(false),
    sftp_(false),
    indexed_(false),
    directory_(),
    ssh_session_(),
    sftp_session_()
{
    ssh_session_ = ssh_new();
}

SSHSession::~SSHSession()
{
    disconnect();
    ssh_free(ssh_session_);
}

bool SSHSession::connect(const QString& hostname,
                         int port,
                         const QString& username)
{
    qDebug() << "Connecting...";

    // Set username
    username_ = username;

    // Check if session was successfully created
    if (ssh_session_ == NULL)
    {
        qDebug() << "Fail. Session was not created successfully";
        return false;
    }

    // Set host address to connect to
    ssh_options_set(ssh_session_, SSH_OPTIONS_HOST,
                    hostname.toStdString().c_str());

    // Set log verbosity level
    int verbosity = SSH_LOG_PROTOCOL;
    ssh_options_set(ssh_session_, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    // Set port to connect
    ssh_options_set(ssh_session_, SSH_OPTIONS_PORT, &port);

    // Set username
    ssh_options_set(ssh_session_, SSH_OPTIONS_USER,
                    username_.toStdString().c_str());

    int rc = ssh_connect(ssh_session_);
    if (rc != SSH_OK)
    {
        qDebug() << "Failed to connect: "
                 << ssh_get_error(ssh_session_);

        return false;
    }

    // TODO: Authenticate server
    // TODO: Show question regarding correct host key
    //QMessageBox::question(this, "HahaaTitle", "HahaaTeksti",
    //    QMessageBox::Yes || QMessageBox::No || QMessageBox::Cancel);

    qDebug() << "Connection successful";

    connected_ = true;

    // Success
    return true;
}

bool SSHSession::authenticate_try_password_methods(const QString &password)
{
    qDebug() << "Authenticating...";

    // Get host supported methods
    int method = ssh_userauth_list(ssh_session_, NULL);

    // If host ssh supports password method
    if (method & SSH_AUTH_METHOD_PASSWORD)
    {
        qDebug() << "Trying password method...";

        // Try Password Authentication Method
        if (authenticate_password(password))
        {
            // Success
            return true;
        }
    }

    // If host ssh supports keyboard-interactive method
    if (method & SSH_AUTH_METHOD_INTERACTIVE)
    {
        qDebug() << "Trying keyboard-interactive method...";

        // Try Keyboard-Interactive Method,
        // this tries to answer password field.
        if (authenticate_keyboard_interactive_password(password))
        {
            // Success
            return true;
        }
    }

    qDebug() << "Authentication failed";

    // Authentication did not succeed
    return false;
}

bool SSHSession::authenticate_password(const QString& password)
{
    // Try to authenticate using password method
    int rc = ssh_userauth_password(ssh_session_,
                               username_.toStdString().c_str(),
                               password.toStdString().c_str());

    // If authentication did not succeed
    if (rc != SSH_AUTH_SUCCESS)
    {
        qDebug() << "Authentication error: "
                 << ssh_get_error(ssh_session_);
        return false;
    }

    qDebug() << "Authenticated";

    // Success
    return true;
}

bool SSHSession::authenticate_keyboard_interactive_password(
        const QString& password)
{
    // initialize keyboard interactive
    int rc = ssh_userauth_kbdint(ssh_session_,
                                 username_.toStdString().c_str(),
                                 NULL);

    if (rc != SSH_AUTH_INFO)
    {
        qDebug() << "No SSH_AUTH_INFO returned, rc = " << rc;
        qDebug() << ssh_get_error(ssh_session_);
        return false;
    }

    // Some variables
    const char *name;
    const char *instruction;
    int nprompts;

    // Get variables from host
    name = ssh_userauth_kbdint_getname(ssh_session_);
    instruction = ssh_userauth_kbdint_getinstruction(ssh_session_);
    nprompts = ssh_userauth_kbdint_getnprompts(ssh_session_);

    // Check that number of prompts is 1 (only the password)
    if (nprompts != 1)
    {
        qDebug() << "first nprompts " <<  nprompts <<" !=1";
        return false;
    }

    const char *prompt;
    char echo;

    // Get prompt from host
    prompt = ssh_userauth_kbdint_getprompt(ssh_session_, 0, &echo);
    qDebug() << prompt;

    // Check if prompt is "Password: ", otherwise fail
    if(QString(prompt) == "Password: ")
    {
        qDebug() << "Answering to Password prompt";

        // Try to answer to password prompt, otherwise fail
        if (ssh_userauth_kbdint_setanswer(
                    ssh_session_, 0,
                    password.toStdString().c_str()) < 0)
        {
            qDebug() << "Password prompt failed";

            return false;
        }
    }
    else
    {
        qDebug() << "Failing. Not password asked.";
        return false;
    }

    // Do keyboard interactive again
    rc = ssh_userauth_kbdint(ssh_session_,
                             username_.toStdString().c_str(),
                             NULL);

    // Fetch info rows from host
    while (rc == SSH_AUTH_INFO)
    {
        name = ssh_userauth_kbdint_getname(ssh_session_);
        instruction = ssh_userauth_kbdint_getinstruction(ssh_session_);
        nprompts = ssh_userauth_kbdint_getnprompts(ssh_session_);

        qDebug() << "Name Instruction Nprompts";
        qDebug() << name << instruction << nprompts;

        // There should not be prompts asked
        if (nprompts != 0)
        {
            qDebug() << "Failing. SSH Server is asking for more "
                     << "keyboard-interactive input";

            return false;
        }

        rc = ssh_userauth_kbdint(ssh_session_,
                                 username_.toStdString().c_str(),
                                 NULL);
    }

    // Check if auth was not success
    if (rc != SSH_AUTH_SUCCESS)
    {
        qDebug() << "Failing. SSH Auth not successful.";
        qDebug() << "rc = " << rc;

        return false;
    }

    qDebug() << "Authenticated";

    // Authentication was success
    authenticated_ = true;

    return true;
}

bool SSHSession::ssh_sftp_init()
{
    // Create new sftp session object
    sftp_session_ = sftp_new(ssh_session_);
    if (sftp_session_ == NULL)
    {
        qDebug() << "Error allocating SFTP session."
                 << ssh_get_error(ssh_session_);
        return false;
    }

    // Initialize sftp session
    int rc = sftp_init(sftp_session_);

    // If session was not initialized correctly
    if (rc != SSH_OK)
    {
        qDebug() << "Error initializing SFTP session."
                 << ssh_get_error(ssh_session_);

        // Free sftp session object
        sftp_free(sftp_session_);

        return false;
    }

    qDebug() << "SFTP initialized";

    // Success
    sftp_ = true;
    return true;
}

QStringList SSHSession::collectFilelist(const QString &directory)
{
    // Get file list using recursion
    QStringList filelist = recurseCollectFileList(directory, "");

    // Sort file list
    filelist.sort();

    // Return sorted file list
    return filelist;
}

QStringList SSHSession::recurseCollectFileList(
        const QString& basedir, const QString& subdir)
{
    sftp_dir dir;
    sftp_attributes attributes;
    int rc;

    QStringList result;

    dir = sftp_opendir(sftp_session_,
                       QString(basedir + "/" + subdir)
                       .toStdString().c_str());

    if (!dir)
    {
        qDebug() << "Directory not opened: "
                 << ssh_get_error(ssh_session_);

        return QStringList();
    }

    while ((attributes = sftp_readdir(sftp_session_, dir)) != NULL)
    {
        QString name = attributes->name;
        if (name != "." && name != "..")
        {
            qDebug() << basedir << "/" << subdir << "/" << name
                     << " " << attributes->type;

            // If directory
            if (attributes->type == 2)
            {
                result += recurseCollectFileList(
                            basedir + "/" + subdir, name);
            }
            // Else if regular file
            else if (attributes->type == 1)
            {
                result += basedir + "/" + subdir + "/" + name;
            }
        }

        sftp_attributes_free(attributes);
    }

    if (!sftp_dir_eof(dir))
    {
        qDebug() << "Can't list directory: "
                 << ssh_get_error(ssh_session_);
    }

    rc = sftp_closedir(dir);
    if (rc != SSH_OK)
    {
        qDebug() << "Can't close directory: "
                 << ssh_get_error(ssh_session_);
    }

    return result;
}

void SSHSession::disconnect()
{
    qDebug() << "Disconnecting...";

    // If sftp session has been created, free it
    if (sftp_)
    {
        sftp_free(sftp_session_);
        sftp_ = false;
    }

    // If connected, disconnect
    if (connected_)
    {
        ssh_disconnect(ssh_session_);

        ssh_free(ssh_session_);
        ssh_session_ = ssh_new();

        authenticated_ = false;
        connected_ = false;
    }

    indexed_ = false;
}

std::tr1::shared_ptr <SSHFile> SSHSession::getFile()
{
    // Create new SSHFile and return it
    return std::tr1::shared_ptr <SSHFile> (new SSHFile(sftp_session_));
}
