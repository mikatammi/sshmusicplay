#ifndef SSHSESSION_HPP
#define SSHSESSION_HPP

#include <QObject>
#include <QString>
#include <tr1/memory>

extern "C"
{
#include <libssh/libssh.h>
#include <libssh/sftp.h>
}

class SSHFile;

class SSHSession : public QObject
{
    Q_OBJECT
public:
    explicit SSHSession(QObject *parent = 0);

    /// Connect to host using username and password.
    /// @param hostname Hostname
    /// @param username Username
    /// @param password Password
    void connect(const QString& hostname,
                 const QString& username,
                 const QString& password);

    /// Disconnects from host.
    void disconnect();

    /// Get pointer to SSHFile object which gives access to file over ssh
    /// with C-like api.
    std::tr1::shared_ptr <SSHFile> getFile();
    
private:
    // Forbid copy constructor and assignment operator
    SSHSession(const SSHSession&);
    SSHSession& operator= (const SSHSession&);

    // SSH and SFTP session objects (libssh)
    ssh_session ssh_session_;
    sftp_session sftp_session_;

};

#endif // SSHSESSION_HPP
