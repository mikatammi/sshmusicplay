#include "audiofile.hpp"
#include "audiooutput.hpp"
#include <QDebug>
#include <QThread>

AudioFile::AudioFile(std::tr1::shared_ptr <SSHFile> file) :
    opened_(false),
    format_context_(0),
    codec_context_(0),
    codec_(0),
    audiostream_(-1),
    sshfile_(file)
{
}

AudioFile::~AudioFile()
{
    // Stop playerthread if created
    if (playerthread_.use_count() > 0)
    {
        playerthread_->stop_play();
    }

    // Close file if opened
    if (opened_)
    {
        close();
    }
}

bool AudioFile::open()
{
    Q_ASSERT(!opened_);

    qDebug() << "Opening audio file";

    // Alloc context to allow tweaking of callback functions for read and seek
    format_context_ = avformat_alloc_context();
    AVIOContext* aviocontext =
            avio_alloc_context(reinterpret_cast <unsigned char*> (
                                   av_malloc(IO_BUFFER_SIZE
                                             + FF_INPUT_BUFFER_PADDING_SIZE)),
                               IO_BUFFER_SIZE,
                               0,
                               reinterpret_cast <void*> (sshfile_.get()),
                               &AudioFile::av_sshfile_read,
                               NULL,
                               &AudioFile::av_sshfile_seek);
    format_context_->pb = aviocontext;

    // Try to open file
    if(avformat_open_input(&format_context_, "dummy_filename", NULL, NULL) < 0)
    {
        qDebug() << "Could not open file";
        return false;
    }


    // Find first audio stream from file
    for (unsigned int i = 0;
         i < format_context_->nb_streams && audiostream_ < 0;
         ++i)
    {
        // If this stream is audio?
        if (format_context_->streams[i]->codec->codec_type
                == AVMEDIA_TYPE_AUDIO)
        {
            audiostream_ = i;
        }
    }

    // Check if audio stream was found
    if (audiostream_ < 0)
    {
        qDebug() << "Didn't find audio stream";
        return false;
    }

    // Get pointer to codec context of audio stream
    codec_context_ = format_context_->streams[audiostream_]->codec;

    // Find decoder for audio stream
    codec_ = avcodec_find_decoder(codec_context_->codec_id);
    if (codec_ == NULL)
    {
        qDebug() << "Codec not found";
        return false; // Codec not found
    }

    AVDictionary *options = NULL;

    // Open codec
    if(avcodec_open2(codec_context_, codec_, &options) < 0)
    {
        qDebug() << "Could not open codec";

        // Could not open codec
        return false;
    }

    av_dict_free(&options);

    // File opened
    qDebug() << "Codec:" << codec_->name << ":" << codec_->long_name;
    qDebug() << "Audio file opened successfully";

    opened_ = true;
    return true;
}

void AudioFile::close()
{
    Q_ASSERT(opened_);

    // Close input
    avformat_close_input(&format_context_);

    opened_ = false;
}

// read and decode audio frame
bool AudioFile::read_and_decode_frame(std::vector <qint8>& buf,
                                      AVFormatContext* format_context,
                                      AVCodecContext* codec_context)
{
    // Try to read frame packet
    AVPacket packet;
    if (av_read_frame(format_context, &packet) < 0)
    {
        qDebug() << "Error while reading frame/packet";

        return false;
    }

    // Allocate frame structure
    AVFrame* frame = avcodec_alloc_frame();
    int got_frame = 0;

    // Try to decode frame
    if (avcodec_decode_audio4(codec_context, frame, &got_frame, &packet) < 0)
    {
        qDebug() << "Error while decoding frame";

        // Free frame after use
        av_free(frame);

        // Free packet after use
        av_free_packet(&packet);

        return false;
    }

    // If empty frame decoded?
    if(frame->linesize[0] == 0)
    {
        buf.assign(1, 0);
        return true;
    }

    // TODO: Maybe some better way to do this?
    buf.assign(frame->linesize[0], 0);
    memcpy(&buf[0], frame->extended_data[0], frame->linesize[0]);

    // Free frame after use
    av_free(frame);

    // Free packet after use
    av_free_packet(&packet);

    return true;
}

int AudioFile::av_sshfile_read(void *opaque, uint8_t *buf, int buf_size)
{
    // Reinterpret cast opaque pointer back to SSHFile
    SSHFile* sshfile = reinterpret_cast <SSHFile*> (opaque);

    // Return number of bytes read
    int retval = sshfile->read(buf, buf_size);

    qDebug() << "av_sshfile_read  buf_size: " << buf_size
             << "  retval: " << retval;

    return retval;
}

int64_t AudioFile::av_sshfile_seek(
        void *opaque, int64_t offset, int whence)
{
    qDebug() << "av_sshfile_seek  offset: " << offset << "  whence :" << whence;

    // Reinterpret cast opaque pointer back to SSHFile
    SSHFile* sshfile = reinterpret_cast <SSHFile*> (opaque);

    if (whence == AVSEEK_SIZE)
    {
        qDebug() << "retval: -1";
        return -1;
    }

    if (!sshfile->seek(offset))
    {
        qDebug() << "retval: -1";
        return -1;
    }

    // Return current position in file
    qDebug() << "retval: " << sshfile->tell();
    return sshfile->tell();
}

void AudioFile::play(std::tr1::shared_ptr <AudioOutput> output)
{
    // Create new player thread
    playerthread_.reset(
                new PlayerThread(output, format_context_, codec_context_));

    // Start player thread
    playerthread_->start();
}

void AudioFile::stop()
{
    // Stop player thread
    playerthread_->stop_play();
}

// Constructor for PlayerThread
AudioFile::PlayerThread::PlayerThread(
        std::tr1::shared_ptr <AudioOutput> output,
        AVFormatContext* format_context,
        AVCodecContext* codec_context,
        QObject* parent) :
    QThread(parent),
    playing_(false),
    output_(output),
    format_context_(format_context),
    codec_context_(codec_context),
    buffer_()
{
}

AudioFile::PlayerThread::~PlayerThread()
{
    // If playing, stop playing thread
    if (playing_)
    {
        stop_play();
    }
}

void AudioFile::PlayerThread::run()
{

    // Fill buffer block
    {
        // Temporary block
        std::vector <qint8> buf;

        // Fill buffer to be at least size of audio output buffer
        while (buffer_.size() < AUDIO_BUFFER_SIZE)
        {
            read_and_decode_frame(buf, format_context_, codec_context_);
            buffer_.insert(buffer_.end(),
                           buf.begin(), buf.end());
        }
    }

    // First write. Android Java AudioTrack -class required
    // buffer to be completely filled before play can be
    // called.
    output_->write(reinterpret_cast <qint16*> (&buffer_[0]),
            0,
            AUDIO_BUFFER_SIZE / 2);

    // Start audiotrack output play.
    output_->play();

    // Clear first values
    buffer_.erase(buffer_.begin(), buffer_.begin() + AUDIO_BUFFER_SIZE);

    // Write remaining data from buffer to audio output
    output_->write(reinterpret_cast <qint16*> (&buffer_[0]),
            0,
            buffer_.size() / 2);

    buffer_.clear();

    // Loop while playing
    playing_ = true;
    while (playing_)
    {
        // Try to decode frames.
        if (AudioFile::read_and_decode_frame(
                    buffer_, format_context_, codec_context_))
        {
            // Write decoded frames to audio output
            output_->write(reinterpret_cast <qint16*> (&buffer_[0]),
                    0,
                    buffer_.size() / 2);
        }
        else
        {
            // If decode fails. Playing should stop.
            playing_ = false;
        }
    }
}

void AudioFile::PlayerThread::stop_play()
{
    // If not playing, don't try to stop playing
    if (!playing_)
    {
        return;
    }

    // Stop playing and wait for thread to stop
    playing_ = false;
    wait();
    output_->pause();
}

bool AudioFile::PlayerThread::is_playing() const
{
    // Return if thread is playing or not
    return playing_;
}
