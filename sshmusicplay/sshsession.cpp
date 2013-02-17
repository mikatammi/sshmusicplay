#include "sshsession.hpp"
#include "sshfile.hpp"

SSHSession::SSHSession(QObject *parent) :
    QObject(parent)
{
}

void SSHSession::connect(const QString& /*hostname*/,
                         const QString& /*username*/,
                         const QString& /*password*/)
{
}

void SSHSession::disconnect()
{
}

std::tr1::shared_ptr <SSHFile> SSHSession::getFile()
{
    return std::tr1::shared_ptr <SSHFile> (new SSHFile(sftp_session_));
}
