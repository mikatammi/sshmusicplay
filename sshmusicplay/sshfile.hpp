#ifndef SSHFILE_HPP
#define SSHFILE_HPP

#include <QObject>

// Include libssh SFTP header
extern "C"
{
#include <libssh/sftp.h>
}

class SSHFile : public QObject
{
    Q_OBJECT
public:
    /// Constructor
    explicit SSHFile(sftp_session session,
                     QObject *parent = 0);

    /// Destructor
    virtual ~SSHFile();

    /// Open file. After this call, read functions become accessible.
    /// @return True if success, false if not.
    bool open(QString filename);

    /// Close file. After this call, file is not accessible anymore.
    void close();

    /// Tells if file is open.
    /// @return True if file is open, false if not.
    bool is_open();

    /// Read count bytes to buffer buf.
    /// @param buf Pointer to buffer
    /// @param count Count of bytes to read
    /// @return Number of bytes written to buffer. Less than 0 on error.
    ssize_t read(void *buf, size_t count);

    /// Rewind to beginning of file
    void rewind();

    /// Seek file to offset
    /// @return True if success, false if fails
    bool seek(uint32_t new_offset);

    /// Tells current position in file
    /// @return Current position in file
    unsigned long tell();

signals:
    
public slots:

private:
    // Variable which holds if file is open or not
    bool file_opened_;

    // Structs for sftp file and session (libssh)
    sftp_file sftp_file_;
    sftp_session sftp_session_;
    
};

#endif // SSHFILE_HPP
