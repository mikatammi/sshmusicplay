#include "audiooutput.hpp"
#include <QDebug>

// Pointer to JavaVM
static JavaVM* javavm__ = 0;

// Class id and methods for AudioTrackOutput class
static jclass audiotrackoutputclassid__ = 0;
static jmethodID audiotrackoutput_constructor__ = 0;
static jmethodID audiotrackoutput_play__ = 0;
static jmethodID audiotrackoutput_pause__ = 0;
static jmethodID audiotrackoutput_stop__ = 0;
static jmethodID audiotrackoutput_write__ = 0;

AudioOutput::AudioOutput(QObject *parent) :
    QObject(parent)
{
    // Attach Java VM to current thread and get environment
    JNIEnv* env;
    if (javavm__->AttachCurrentThread(&env, NULL) < 0)
    {
        qCritical() << "Could not Attach Java VM to Current Thread";
        return;
    }

    audiotrackoutputobject_ =
            env->NewGlobalRef(env->NewObject(audiotrackoutputclassid__,
                                             audiotrackoutput_constructor__));
    if (!audiotrackoutputobject_)
    {
        qCritical() << "Cannot create AudioTrackOutput object";
        return;
    }

    // Detach Java VM from current thread after use
    javavm__->DetachCurrentThread();
}

AudioOutput::~AudioOutput()
{
}

void AudioOutput::play()
{
    // Attach Java VM to current thread and get environment
    JNIEnv* env;
    if (javavm__->AttachCurrentThread(&env, NULL) < 0)
    {
        qCritical() << "Could not Attach Java VM to Current Thread";
        return;
    }

    // Call Java AudioTrackOutput play function
    env->CallVoidMethod(audiotrackoutputobject_, audiotrackoutput_play__);

    // Detach Java VM from current thread after use
    javavm__->DetachCurrentThread();
}

void AudioOutput::pause()
{
    // Attach Java VM to current thread and get environment
    JNIEnv* env;
    if (javavm__->AttachCurrentThread(&env, NULL) < 0)
    {
        qCritical() << "Could not Attach Java VM to Current Thread";
        return;
    }

    // Call Java AudioTrackOutput pause function
    env->CallVoidMethod(audiotrackoutputobject_, audiotrackoutput_pause__);

    // Detach Java VM from current thread after use
    javavm__->DetachCurrentThread();
}

void AudioOutput::stop()
{
    // Attach Java VM to current thread and get environment
    JNIEnv* env;
    if (javavm__->AttachCurrentThread(&env, NULL) < 0)
    {
        qCritical() << "Could not Attach Java VM to Current Thread";
        return;
    }

    // Call Java AudioTrackOutput stop function
    env->CallVoidMethod(audiotrackoutputobject_, audiotrackoutput_stop__);

    // Detach Java VM from current thread after use
    javavm__->DetachCurrentThread();
}

void AudioOutput::write(qint16* data, size_t offset, size_t count)
{
    // Attach Java VM to current thread and get environment
    JNIEnv* env;
    if (javavm__->AttachCurrentThread(&env, NULL) < 0)
    {
        qCritical() << "Could not Attach Java VM to Current Thread";
        return;
    }

    // Create jshortArray for data
    jshortArray data_array = env->NewShortArray(count);
    env->SetShortArrayRegion(data_array, offset, count, data);

    // Call Java AudioTrackOutput write function
    env->CallVoidMethod(audiotrackoutputobject_, audiotrackoutput_write__,
                        data_array, 0, count);

    // Detach Java VM from current thread after use
    javavm__->DetachCurrentThread();
}

// This function will be run when JNI loads.
// This function initalizes static variables introduced in beginning
// of the file.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    // Initalize JavaVM pointer
    javavm__ = vm;

    // Get JNI environment
    JNIEnv* env;
    if (javavm__->GetEnv(reinterpret_cast<void**>(&env),
                         JNI_VERSION_1_6) != JNI_OK)
    {
        qCritical() << "Cannot get the environment";
        return -1;
    }

    // Get AudioTrackOutput class
    jclass clazz = env->FindClass("org/kde/necessitas/origo/AudioTrackOutput");
    if (!clazz)
    {
        qCritical() << "Cannot get AudioTrackOutput class";
        return -1;
    }

    // Make new global refernce to AudioTrackOutput class
    audiotrackoutputclassid__ = (jclass)env->NewGlobalRef(clazz);

    // Get AudioTrackOutput constructor
    audiotrackoutput_constructor__ = env->GetMethodID(
                audiotrackoutputclassid__, "<init>", "()V");
    if (!audiotrackoutput_constructor__)
    {
        qCritical() << "Cannot get AudioTrackOutput constructor";
    }

    // Get AudioTrackOutput play function
    audiotrackoutput_play__ = env->GetMethodID(
                audiotrackoutputclassid__, "play", "()V");
    if (!audiotrackoutput_constructor__)
    {
        qCritical() << "Cannot get AudioTrackOutput play function";
    }

    // Get AudioTrackOutput pause function
    audiotrackoutput_pause__ = env->GetMethodID(
                audiotrackoutputclassid__, "pause", "()V");
    if (!audiotrackoutput_constructor__)
    {
        qCritical() << "Cannot get AudioTrackOutput pause function";
    }

    // Get AudioTrackOutput stop function
    audiotrackoutput_stop__ = env->GetMethodID(
                audiotrackoutputclassid__, "stop", "()V");
    if (!audiotrackoutput_constructor__)
    {
        qCritical() << "Cannot get AudioTrackOutput stop function";
    }

    // Get AudioTrackOutput write function
    audiotrackoutput_write__ = env->GetMethodID(
                audiotrackoutputclassid__, "write", "([SII)V");
    if (!audiotrackoutput_constructor__)
    {
        qCritical() << "Cannot get AudioTrackOutput write function";
    }

    qDebug() << "AudioOutput JNI_OnLoad complete";

    return JNI_VERSION_1_6;
}
