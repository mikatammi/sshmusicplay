#ifndef AUDIOFILE_HPP
#define AUDIOFILE_HPP

#include <vector>
#include <tr1/memory>
#include <QThread>

#include "downloadbuffer.hpp"
#include "libav.hpp"

class AudioOutput;

// Class for decoding audio file. Uses SSHFile for reading.
class AudioFile
{

public:

    /// Constructor
    /// @param file Opened SSHFile to use for reading
    explicit AudioFile(std::tr1::shared_ptr <DownloadBuffer> file);

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


private:
    // Function for reading and decoding one frame from input stream
    static bool read_and_decode_frame(std::vector <qint8>& buf,
                                      AVFormatContext* format_context,
                                      AVCodecContext* codec_context);

    // Custom read and seek callback functions for libavformat,
    // which can read File.
    static int av_file_read(void* opaque, uint8_t* buf, int buf_size);
    static int64_t av_file_seek(void* opaque, int64_t offset, int whence);

    // Internal class for Player thread
    class PlayerThread : public QThread
    {
    public:
        // Constructor
        PlayerThread(std::tr1::shared_ptr <AudioOutput> output,
                   AVFormatContext* format_context,
                   AVCodecContext* codec_context,
                   QObject* parent = 0);

        // Destructor
        virtual ~PlayerThread();

        // Is playing?
        bool is_playing() const;

        // Stop playing. Blocking until thread stops.
        void stop_play();

    protected:
        // thread code
        void run();

    private:
        // Controls if thread main loop is running or not
        bool playing_;

        // Audio output
        std::tr1::shared_ptr <AudioOutput> output_;

        // libav contexes, needed for decoding
        AVFormatContext* format_context_;
        AVCodecContext* codec_context_;

        // buffer
        std::vector <qint8> buffer_;
    };

    // File open status
    bool opened_;

    // AV Format Context demuxer structure
    AVFormatContext* format_context_;

    // AV Codec
    AVCodecContext* codec_context_;
    AVCodec* codec_;

    // Audiostream id
    int audiostream_;

    // Pointer to File
    std::tr1::shared_ptr <DownloadBuffer> file_;

    // Player thread
    std::tr1::shared_ptr <PlayerThread> playerthread_;

    // Input data buffer size.
    static const unsigned int IO_BUFFER_SIZE = 1024 * 64; // 64 KiB

    // Audio buffer size.
    // If you change this, remember to change it also in java class.
    // TODO: Write some kind of audio buffer size getter mechanism
    //       for AudioOutput/AudioTrackOutput classes, to make
    //       PlayerThread able to automatically detect buffer size.
    static const unsigned int AUDIO_BUFFER_SIZE = 1024 * 512; // 512 KiB
};

#endif // AUDIOFILE_HPP
