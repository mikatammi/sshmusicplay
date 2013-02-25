#ifndef DOWNLOADBUFFER_HPP
#define DOWNLOADBUFFER_HPP

#include <QThread>
#include <QMutex>
#include <tr1/memory>

class SSHFile;

/// Download Buffer
/// Class which downloads file from source and buffers it
class DownloadBuffer : public QThread
{

public:

    /// Constructor
    /// @param file Pointer to file
    /// @param parent QObject parent
    DownloadBuffer(std::tr1::shared_ptr <SSHFile> file,
                   QObject* parent = 0);

    /// Destructor
    virtual ~DownloadBuffer();

    size_t bufferSpaceFilled();

    size_t read(char *buf, size_t count);

    size_t tell();

    bool seek(size_t pos);

    /// Stop
    void stop_download();

protected:

    /// Thread code
    void run();

private:

    // Copy constructor and assignment operator forbidden
    DownloadBuffer(const DownloadBuffer&);
    DownloadBuffer& operator= (const DownloadBuffer&);

    // Variable to control download
    bool do_download_;

    // File to download has reached eof
    bool eof_;

    // Variable to tell if thread has stopped
    bool stopped_;

    // Mutex lock for download control variables
    QMutex download_vars_mutex_;

    // Pointer to SSHFile
    std::tr1::shared_ptr <SSHFile> file_;

    // File read position
    size_t file_read_position_;

    // Download position
    size_t download_position_;

    // Buffer size
    static const uint32_t BUFFER_SIZE = 1024 * 1024 * 20; // 20 MiB

    // Buffer
    char* buf_;

    // Buffer read position
    size_t buf_read_position_;

    // Buffer write position
    size_t buf_write_position_;

    // Buffer mutex
    QMutex buf_mutex_;

};

#endif // DOWNLOADBUFFER_HPP
