#pragma once

#include <QList>
#include <QString>

#include "audio/types.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevicestatus.h"
#include "soundio/soundmanagerutil.h"
#include "util/types.h"

class SoundManager;
class AudioOutputBuffer;
class AudioInputBuffer;

const QString kNetworkDeviceInternalName = "Network stream";

class SoundDeviceDescriptor {
  public:
    SoundDeviceDescriptor();
    virtual ~SoundDeviceDescriptor() = default;

    /// Get the name of the audio API used by this device.
    inline const QString& getHostAPI() const {
        return m_hostAPI;
    }
    /// Get the name of the soundcard, as displayed to the user.
    inline const QString& getDisplayName() const {
        return m_strDisplayName;
    }
    /// Get the ID of this sound device.
    inline const SoundDeviceId& getDeviceId() const {
        return m_deviceId;
    }
    /// Get the number of input channels that the soundcard has
    inline mixxx::audio::ChannelCount getNumInputChannels() const {
        return m_numInputChannels;
    }
    /// Get the number of output channels that the soundcard has
    inline mixxx::audio::ChannelCount getNumOutputChannels() const {
        return m_numOutputChannels;
    }
    /// Get the default sample rate for this soundcard
    inline mixxx::audio::SampleRate getDefaultSampleRate() const {
        return m_defaultSampleRate;
    }

    // The name of the audio API used by this device.
    QString m_hostAPI;
    // The name of the soundcard, as displayed to the user
    QString m_strDisplayName;
    // The identifier for the device in the underlying API.
    SoundDeviceId m_deviceId;
    // The number of output channels that the soundcard has
    mixxx::audio::ChannelCount m_numOutputChannels;
    // The number of input channels that the soundcard has
    mixxx::audio::ChannelCount m_numInputChannels;
    // The default samplerate for the sound device.
    mixxx::audio::SampleRate m_defaultSampleRate;
};

class SoundDevice : public SoundDeviceDescriptor {
  public:
    SoundDevice(UserSettingsPointer config, SoundManager* sm);
    ~SoundDevice() override;

    void setSampleRate(mixxx::audio::SampleRate sampleRate);
    void setConfigFramesPerBuffer(unsigned int framesPerBuffer);
    virtual SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) = 0;
    virtual bool isOpen() const = 0;
    virtual SoundDeviceStatus close() = 0;
    virtual void readProcess(SINT framesPerBuffer) = 0;
    virtual void writeProcess(SINT framesPerBuffer) = 0;
    virtual QString getError() const = 0;
    SoundDeviceStatus addOutput(const AudioOutputBuffer& out);
    SoundDeviceStatus addInput(const AudioInputBuffer& in);
    const QList<AudioInputBuffer>& inputs() const {
        return m_audioInputs;
    }
    const QList<AudioOutputBuffer>& outputs() const {
        return m_audioOutputs;
    }

    void clearOutputs();
    void clearInputs();
    bool operator==(const SoundDevice &other) const;
    bool operator==(const QString &other) const;

  protected:
    void composeOutputBuffer(CSAMPLE* outputBuffer,
                             const SINT iFramesPerBuffer,
                             const SINT readOffset,
                             const int iFrameSize);

    void composeInputBuffer(const CSAMPLE* inputBuffer,
                            const SINT framesToPush,
                            const SINT framesWriteOffset,
                            const int iFrameSize);

    void clearInputBuffer(const SINT framesToPush,
                          const SINT framesWriteOffset);

    UserSettingsPointer m_pConfig;
    // Pointer to the SoundManager object which we'll request audio from.
    SoundManager* m_pSoundManager;
    // The current samplerate for the sound device.
    mixxx::audio::SampleRate m_sampleRate;
    // The **configured** number of frames per buffer. We'll tell PortAudio we
    // want this many frames in a buffer, but PortAudio may still give us have a
    // differently sized buffers. As such this value should only be used for
    // configuring the audio devices. The actual runtime buffer size should be
    // used for any computations working with audio.
    SINT m_configFramesPerBuffer;
    QList<AudioOutputBuffer> m_audioOutputs;
    QList<AudioInputBuffer> m_audioInputs;
};

typedef QSharedPointer<SoundDeviceDescriptor> SoundDeviceDescriptorPointer;
typedef QSharedPointer<SoundDevice> SoundDevicePointer;
