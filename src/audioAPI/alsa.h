#ifndef PWSCP_ALSA_H
#define PWSCP_ALSA_H

#include <alsa/asoundlib.h>
#include <memory>
#include <string>
#include <vector>

namespace AudioAPI {
    const std::vector<unsigned int> STD_SAMPLE_RATES = {8000, 16000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000, 256000, 352800, 384000, 512000, 705600, 768000};
    std::vector<int> STD_BUFFER_SIZES();
    constexpr int DEFAULT_SAMPLE_RATE = 48000;
    constexpr int DEFUALT_BUFFER_SIZE = 1024;

    struct audioDevice {
        int cardIndex;
        bool isPlaybackDevice;
        bool isCaptureDevice;
        std::string name;
        std::string longName;
        std::string cardHardwareName;
        std::vector<int> supportedSampleRates;
        std::vector<int> supportedBufferSizes;
    };

    typedef std::vector<std::unique_ptr<audioDevice>> audioDevices;

    std::unique_ptr<audioDevices> getAvailableAudioDevices();

    std::string vectorValuesToString(const std::vector<int>& vec);
    std::vector<std::string> vectorValuesToStringsVec(const std::vector<int>& vec);

    std::vector<int> mapSharedBufferSizes(const std::unique_ptr<audioDevices>& audioDevices);
    std::vector<int> mapSharedSampleRates(const std::unique_ptr<audioDevices>& audioDevices);

    std::vector<int> getAllCardsSupportedSampleRates(const std::unique_ptr<audioDevices>& audioDevices);
    std::vector<int> getAllCardsSupportedBufferSizes(const std::unique_ptr<audioDevices>& audioDevices);

}

#endif
