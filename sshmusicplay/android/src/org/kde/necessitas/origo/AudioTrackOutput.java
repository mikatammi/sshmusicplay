package org.kde.necessitas.origo;

import android.media.AudioFormat;
import android.media.AudioTrack;
import android.media.AudioManager;

/// AudioTrack Output
public class AudioTrackOutput
{
    AudioTrack audiotrack_;

    /// Constructor for AudioTrackOutput, a wrapper class for JNI calls.
    public AudioTrackOutput()
    {
        // Set sample rate, channel config, audio format and streaming mode
        int samplerate = 44100;
        int channelconfig = AudioFormat.CHANNEL_OUT_STEREO;
        int audioformat = AudioFormat.ENCODING_PCM_16BIT;
        int mode = AudioTrack.MODE_STREAM;

        // Get minimum buffer size in bytes
        int minbuffersize = 
            AudioTrack.getMinBufferSize(samplerate, channelconfig, audioformat);
        
        // Set target buffer size
        int targetbuffersize = 44100;

        // Initialize buffer size to target buffer size
        int buffersize = targetbuffersize;

        // If buffer if smaller than minimum buffer size
        if (buffersize < minbuffersize)
        {
            // Set buffer size to minimum buffer size
            buffersize = minbuffersize;
        }

        // Create AudioTrack object
        audiotrack_ = new AudioTrack(AudioManager.STREAM_MUSIC, samplerate,
                channelconfig, audioformat, buffersize, mode);
    }

    /// Start playing AudioTrack.
    public void play()
    {
        audiotrack_.play();
    }

    /// Pause playback of the audio data.
    public void pause()
    {
        audiotrack_.pause();
    }

    /// Stops playing the audio data after buffer has reached end.
    public void stop()
    {
        audiotrack_.stop();
    }

    /// Write audio data to audio hardware for playback.
    /// @param audioData Audio data
    /// @param offsetInShorts Offset in audioData where to start reading
    /// @param sizeInShorts How many values to read following offset
    public void write(short[] audioData, int offsetInShorts, int sizeInShorts)
    {
        audiotrack_.write(audioData, offsetInShorts, sizeInShorts);
    }
}
