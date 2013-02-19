#include "audiofile.hpp"
#include "audiooutput.hpp"
#include <QDebug>

AudioFile::AudioFile(std::tr1::shared_ptr <SSHFile> file,
                     QObject *parent) :
    QObject(parent),
    opened_(false),
    format_context_(0),
    codec_context_(0),
    codec_(0),
    audiostream_(-1),
    sshfile_(file),
    decoded_bufferpos_(0),
    decoded_buffer_()
{
}

AudioFile::~AudioFile()
{
}

bool AudioFile::open()
{
    qDebug() << "Opening audio file";

    // Alloc context to make some tweaking
    format_context_ = avformat_alloc_context();
    AVIOContext* aviocontext =
            avio_alloc_context(reinterpret_cast <unsigned char*> (
                                   av_malloc(IO_BUFFER_SIZE
                                             + FF_INPUT_BUFFER_PADDING_SIZE)),
                               IO_BUFFER_SIZE,
                               0,
                               reinterpret_cast <void*> (sshfile_.get()),
                               &AudioFile::av_customread,
                               NULL,
                               &AudioFile::av_customseek);
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

    qDebug() << "Codec: " << codec_context_->codec_name;

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
    qDebug() << "Audio file opened successfully";

    opened_ = true;
    return true;
}

void AudioFile::close()
{
    // Close input
    avformat_close_input(&format_context_);
}

bool AudioFile::read_and_decode_frame()
{
    qDebug() << "Reading frame";

    // Try to read frame packet
    AVPacket* packet = 0;
    if (av_read_frame(format_context_, packet) < 0)
    {
        qDebug() << "Error while reading frame/packet";

        return false;
    }

    // Allocate frame structure
    AVFrame* frame = avcodec_alloc_frame();

    qDebug() << "Frame read, decoding...";

    int got_frame;

    // Try to decode frame
    if (avcodec_decode_audio4(codec_context_, frame, &got_frame, packet) < 0)
    {
        qDebug() << "Error while decoding frame";

        // Free frame after use
        av_free(frame);

        // Free packet after use
        av_free_packet(packet);

        return false;
    }

    qDebug() << "Decoded. Storing to buffer.";

    unsigned int startpos = 0;

    // If reaching end of decode buffer
    if (decoded_bufferpos_ + frame->linesize[0] > DECODED_DATA_BUFFER_SIZE)
    {
        unsigned int maxcount =
                DECODED_DATA_BUFFER_SIZE - decoded_bufferpos_;
        memcpy(&decoded_buffer_[decoded_bufferpos_],
               frame->extended_data[0],
                maxcount);
        startpos = maxcount;
        decoded_bufferpos_ = 0;
    }

    memcpy(&decoded_buffer_[decoded_bufferpos_],
           &(frame->extended_data[0])[startpos],
            frame->linesize[0] - startpos);

    decoded_bufferpos_ += frame->linesize[0] - startpos;

    // Free frame after use
    av_free(frame);

    // Free packet after use
    av_free_packet(packet);

    qDebug() << "Frame read successfully";

    return true;
}

int AudioFile::av_customread(void *opaque, uint8_t *buf, int buf_size)
{
    // Reinterpret cast opaque pointer back to SSHFile
    SSHFile* sshfile = reinterpret_cast <SSHFile*> (opaque);

    // Return number of bytes read
    int retval = sshfile->read(buf, buf_size);

    qDebug() << "av_customread  buf_size: " << buf_size
             << "  retval: " << retval;

    return retval;
}

int64_t AudioFile::av_customseek(
        void *opaque, int64_t offset, int whence)
{
    qDebug() << "av_customseek  offset: " << offset << "  whence :" << whence;

    // Reinterpret cast opaque pointer back to SSHFile
    SSHFile* sshfile = reinterpret_cast <SSHFile*> (opaque);

    if (whence == AVSEEK_SIZE)
    {
        qDebug() << "retval: -1";
        // Case "Size of my handle in bytes"
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
    //
    read_and_decode_frame();
}

void AudioFile::pause()
{
    //
}

void AudioFile::stop()
{
    //
}
