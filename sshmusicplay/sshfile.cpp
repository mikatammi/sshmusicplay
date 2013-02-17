#include "sshfile.hpp"

extern "C"
{
#include <fcntl.h> // Needed for O_RDONLY
}

SSHFile::SSHFile(sftp_session session,
                 QObject *parent) :
    QObject(parent),
    file_opened_(false),
    sftp_session_(session)
{
}

SSHFile::~SSHFile()
{
    // Close file if it is open
    if (file_opened_)
    {
        close();
    }
}

bool SSHFile::open(QString filename)
{
    Q_ASSERT(!file_opened_);

    // Try to open file
    sftp_file_ = sftp_open(sftp_session_, filename.toStdString().c_str(),
                           O_RDONLY, 0);

    // Check if file didn't open
    if (sftp_file_ == NULL)
    {
        file_opened_ = false;
        return false;
    }

    file_opened_ = true;
    return true;
}

void SSHFile::close()
{
    // Close file if it has been opened
    if (file_opened_)
    {
        // Close file
        sftp_close(sftp_file_);

        // Set file open status to closed
        file_opened_ = false;
    }
}

ssize_t SSHFile::read(void *buf, size_t count)
{
    Q_ASSERT(file_opened_);

    // Read file
    return sftp_read(sftp_file_, buf, count);
}

void SSHFile::rewind()
{
    Q_ASSERT(file_opened_);

    // Rewind file
    sftp_rewind(sftp_file_);
}

bool SSHFile::seek(uint32_t new_offset)
{
    Q_ASSERT(file_opened_);

    // Seek file
    return sftp_seek(sftp_file_, new_offset) == 0;
}

unsigned long SSHFile::tell()
{
    Q_ASSERT(file_opened_);

    // Tell file position
    return sftp_tell(sftp_file_);
}
