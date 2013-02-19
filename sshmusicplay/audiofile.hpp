#ifndef AUDIOFILE_HPP
#define AUDIOFILE_HPP

#include <tr1/memory>
#include <QObject>

#include "sshfile.hpp"
#include "libav.hpp"

class AudioOutput;

class AudioFile : public QObject
{
    Q_OBJECT
public:
    // Constructor
    explicit AudioFile(std::tr1::shared_ptr <SSHFile> file,
                       QObject *parent = 0);

    // Destructor
    virtual ~AudioFile();

    // Try to open file
    // @return True if success, false if not
    bool open();

    // Close file
    void close();

    // Start to play audio
    // @param output Pointer to audio output
    void play(std::tr1::shared_ptr <AudioOutput> output);

    // Pause audio playing
    void pause();

    // Stop audio playing after buffer has reached end
    void stop();
    
signals:
    
public slots:

private:

    static int av_customread(void* opaque, uint8_t* buf, int buf_size);
    static int64_t av_customseek(void* opaque, int64_t offset, int whence);

    bool read_and_decode_frame();

    // File open status
    bool opened_;

    // AV Format Context demuxer structure
    AVFormatContext* format_context_;

    // AV Codec
    AVCodecContext* codec_context_;
    AVCodec* codec_;

    // Audiostream id
    int audiostream_;

    // Pointer to SSH File
    std::tr1::shared_ptr <SSHFile> sshfile_;

    static const unsigned int IO_BUFFER_SIZE = 65536;
    static const unsigned int DECODED_DATA_BUFFER_SIZE = 44100*20*3; //30sec
            //4096*1024; // 4MB

    unsigned int decoded_bufferpos_;
    char decoded_buffer_[DECODED_DATA_BUFFER_SIZE];

};

#endif // AUDIOFILE_HPP
