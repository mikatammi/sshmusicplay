#ifndef SSHSESSION_HPP
#define SSHSESSION_HPP

#include <QObject>
#include <QString>
#include <tr1/memory>

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
    void connect(QString hostname, QString username, QString password);

    /// Disconnects from host.
    void disconnect();

    /// Get pointer to SSHFile object which gives access to file over ssh
    /// with C-like api.
    std::tr1::shared_ptr<SSHFile> getFile(QString filename);
    
signals:
    
public slots:
    
};

#endif // SSHSESSION_HPP
