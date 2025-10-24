#ifndef PWSCP_ALSA_H
#define PWSCP_ALSA_H

#include <memory>
#include <string>
#include <vector>

namespace AudioAPI {
    struct Soundcard {
        int cardIndex;
        std::string name;
        std::string longName;
        std::vector<int> supportedSampleRates;
        std::vector<int> supportedBufferSizes;
    };

    typedef std::vector<std::unique_ptr<Soundcard>> Soundcards;

    std::unique_ptr<Soundcards> getAvailableSoundcards();

    std::string vectorValuesToString(const std::vector<int>& vec);
    std::vector<std::string> vectorValuesToStringsVec(const std::vector<int>& vec);

    std::vector<int> mapSharedBufferSizes(const std::unique_ptr<Soundcards>& soundcards);
    std::vector<int> mapSharedSampleRates(const std::unique_ptr<Soundcards>& soundcards);

    std::vector<int> getAllCardsSupportedSampleRates(const std::unique_ptr<Soundcards>& soundcards);
    std::vector<int> getAllCardsSupportedBufferSizes(const std::unique_ptr<Soundcards>& soundcards);

}

#endif
