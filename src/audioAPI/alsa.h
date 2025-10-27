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
    constexpr int DEFUALT_BUFFER_SIZE = 2048;

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

    audioDevices getAvailableAudioDevices();

    std::string vectorValuesToString(const std::vector<int>& vec);
    std::vector<std::string> vectorValuesToStringsVec(const std::vector<int>& vec);

    std::vector<int> mapSharedBufferSizes(const audioDevices& adevs);
    std::vector<int> mapSharedSampleRates(const audioDevices& adevs);

    std::vector<int> getAllCardsSupportedSampleRates(const audioDevices& adevs);
    std::vector<int> getAllCardsSupportedBufferSizes(const audioDevices& adevs);

}

#endif
