#ifndef AUDIOOUTPUT_HPP
#define AUDIOOUTPUT_HPP

#include <QObject>
#include <jni.h>

/// JNI Wrapper class for Java AudioTrackOutput
class AudioOutput : public QObject
{
    Q_OBJECT
public:

    /// Constructor
    explicit AudioOutput(QObject *parent = 0);

    /// Destructor
    virtual ~AudioOutput();

    /// Start playing AudioTrack.
    void play();

    /// Pause playback of the audio data.
    void pause();

    /// Stops playing the audio data after buffer has reached end.
    void stop();

    /// Write audio data to audio hardware for playback.
    /// @param data Audio data
    /// @param offset (in shorts) Offset in audioData where to start reading
    /// @param count (in shorts) How many values to read following offset
    void write(qint16* data, size_t offset, size_t count);
    
signals:
    
public slots:

private:

    // JNI jobject for AudioTrackOutput class
    jobject audiotrackoutputobject_;
    
};

#endif // AUDIOOUTPUT_HPP
