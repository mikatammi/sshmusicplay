#include "downloadbuffer.hpp"
#include "sshfile.hpp"
#include <QDebug>

DownloadBuffer::DownloadBuffer(std::tr1::shared_ptr <SSHFile> file,
                               QObject* parent) :
    QThread(parent),
    do_download_(false),
    eof_(false),
    stopped_(true),
    download_vars_mutex_(),
    file_(file),
    file_read_position_(0),
    download_position_(0),
    buf_(0),
    buf_read_position_(0),
    buf_write_position_(0),
    buf_mutex_()
{
    buf_ = new char[BUFFER_SIZE];
}

DownloadBuffer::~DownloadBuffer()
{
    stop_download();

    delete[] buf_;
    buf_ = 0;
}

size_t DownloadBuffer::bufferSpaceFilled()
{
    if (buf_read_position_ < buf_write_position_)
    {
        return buf_write_position_ - buf_read_position_;
    }
    else if (buf_read_position_ > buf_write_position_)
    {
        return buf_write_position_ +
                (BUFFER_SIZE - buf_read_position_);
    }

    // buf_read_position_ == buf_write_position_
    return 0;
}

size_t DownloadBuffer::read(char* buf, size_t count)
{
    // Start download if it is not running
    if (!isRunning() && !eof_)
    {
        qDebug() << "Download thread not running, starting";

        // Set variables to state which tell download is running.
        // Lock mutex while setting.
        download_vars_mutex_.lock();
        stopped_ = false;
        do_download_ = true;
        download_vars_mutex_.unlock();

        start();
    }

    buf_mutex_.lock();

    if (count > BUFFER_SIZE)
    {
        count = BUFFER_SIZE;
    }

    while(bufferSpaceFilled() < count && isRunning())
    {
        buf_mutex_.unlock();

        qDebug() << "Buffer drained";
        QThread::msleep(500);

        buf_mutex_.lock();
    }

    size_t space_until_end = BUFFER_SIZE - buf_read_position_;
    size_t buf_cpy_start_pos = 0;
    size_t read_count = qMin(count, bufferSpaceFilled());

    if (read_count == 0)
    {
        buf_mutex_.unlock();
        return 0;
    }

    if (space_until_end < read_count)
    {
        memcpy(buf, &buf_[buf_read_position_], space_until_end);
        buf_cpy_start_pos = space_until_end;
        buf_read_position_ = 0;
    }

    memcpy(&buf[buf_cpy_start_pos], &buf_[buf_read_position_],
           read_count - buf_cpy_start_pos);
    buf_read_position_ += read_count - buf_cpy_start_pos;

    file_read_position_ += read_count;

    buf_mutex_.unlock();

    return read_count;
}

size_t DownloadBuffer::tell()
{
    qDebug() << "DownloadBuffer tell";
    return file_read_position_;
}

bool DownloadBuffer::seek(uint32_t offset)
{
    qDebug() << "DownloadBuffer seek";

    // Stop downloading if it is running
    if (isRunning())
    {
        stop_download();
    }

    // Seek file
    bool seek_status = file_->seek(offset);

    if(!seek_status)
    {
        return false;
    }

    // Set buffer read and write position to 0
    buf_read_position_ = 0;
    buf_write_position_ = 0;

    // Set download position to offset
    file_read_position_ = offset;
    download_position_ = offset;

    return true;
}

void DownloadBuffer::run()
{
    qDebug() << "Downloader thread starting";

    // How many bytes to read from file per one read
    unsigned int const READ_BLOCK_SIZE = 1024 * 64; // 64 KiB

    // Loop until download has stopped, either by user or by
    // finishing download or error.
    while(do_download_)
    {
        // Buffer to read bytes from file
        char readbuf[READ_BLOCK_SIZE];

        // Read bytes from file
        ssize_t readbytes = file_->read(readbuf, READ_BLOCK_SIZE);

        if (readbytes < 0)
        {
            qDebug() << "Error while downloading, stopping";

            // Error has happened in read. Stop thread.
            do_download_ = false;
        }
        else if (readbytes == 0)
        {
            qDebug() << "DownloadBuffer EOF";

            // End of file reached, stop thread.
            eof_ = true;
            do_download_ = false;
        }
        else
        {
            buf_mutex_.lock();

            // Wait until there is enough free space in the buffer
            while (BUFFER_SIZE - bufferSpaceFilled() - 1 <
                   static_cast <size_t> (readbytes))
            {
                buf_mutex_.unlock();

                QThread::msleep(100);

                buf_mutex_.lock();
            }

            size_t buf_cpy_start_pos = 0;
            size_t space_until_end = BUFFER_SIZE - buf_write_position_;

            if (space_until_end < static_cast <size_t> (readbytes))
            {
                memcpy(&buf_[buf_write_position_], readbuf, space_until_end);
                buf_cpy_start_pos = space_until_end;
                buf_write_position_ = 0;
            }

            memcpy(&buf_[buf_write_position_],
                   &readbuf[buf_cpy_start_pos],
                   readbytes - buf_cpy_start_pos);

            buf_write_position_ += readbytes - buf_cpy_start_pos;
            download_position_ += readbytes;

            buf_mutex_.unlock();
        }
    }

    // Set downloading thread state to stopped.
    download_vars_mutex_.lock();
    stopped_ = true;
    download_vars_mutex_.unlock();
}

void DownloadBuffer::stop_download()
{
    qDebug() << "Stopping download";

    download_vars_mutex_.lock();
    do_download_ = false;
    download_vars_mutex_.unlock();

    wait();

    qDebug() << "Stopped";
}
