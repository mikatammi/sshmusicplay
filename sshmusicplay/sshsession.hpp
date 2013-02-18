#ifndef SSHSESSION_HPP
#define SSHSESSION_HPP

#include <QObject>
#include <QString>
#include <QStringList>
#include <tr1/memory>

extern "C"
{
#include <libssh/libssh.h>
#include <libssh/sftp.h>
}

class SSHFile;

class SSHSession
{
public:
    /// Constructor
    SSHSession();

    /// Destructor
    virtual ~SSHSession();

    /// Connect to host
    /// @param hostname Hostname
    /// @return True if success, false if not
    bool connect(const QString& hostname,
                 int port,
                 const QString& username);

    /// Try both password authentication methods,
    /// depending on which methods host supports.
    /// @param username Username
    /// @param password Plaintext password
    /// @return True if success, false if fails
    bool authenticate_try_password_methods(
            const QString& username, const QString& password);

    /// Password -authentication method
    /// @param username Username
    /// @param password Plaintext password
    /// @return True if success, false if fails
    bool authenticate_password(const QString& username,
                               const QString& password);

    /// Keyboard-interactive method, tries to answer to password field
    /// @param username Username
    /// @param password Plaintext password
    /// @return True if success, false if fails
    bool authenticate_keyboard_interactive_password(
            const QString& username, const QString& password);

    /// Initialize sftp subsystem
    /// @return True if success, false if fails
    bool ssh_sftp_init();

    /// Collect file list from directory.
    /// @param directory Directory to list.
    /// @return List of file paths relative to directory in QStringList
    QStringList collectFilelist(const QString& directory);

    /// Disconnects from host.
    void disconnect();

    /// Get pointer to SSHFile object which gives access to file over ssh
    /// with C-like api.
    /// @return Shared Pointer to SSHFile object
    std::tr1::shared_ptr <SSHFile> getFile();
    
private:
    // Forbid copy constructor and assignment operator
    SSHSession(const SSHSession&);
    SSHSession& operator= (const SSHSession&);

    // File List Collect recursion
    QStringList recurseCollectFileList(
            const QString& basedir, const QString& subdir);

    // THis will be set to true if connected
    bool connected_;

    // This will be set to true if authentication succeeds
    bool authenticated_;

    // This will be set to true if SFTP subsystem is successfully opened
    bool sftp_;

    // If indexing is completed
    bool indexed_;

    // Directory to be indexed
    QString directory_;

    // Username
    QString username_;

    // SSH and SFTP session objects (libssh)
    ssh_session ssh_session_;
    sftp_session sftp_session_;

};

#endif // SSHSESSION_HPP
