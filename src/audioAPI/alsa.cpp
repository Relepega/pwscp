#include "alsa.h"

#include <alsa/asoundlib.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace AudioAPI {
    std::unique_ptr<Soundcards> getAvailableSoundcards() {
        auto scs = std::make_unique<Soundcards>();

        int cardIndex= -1;

        while (snd_card_next(&cardIndex) >= 0 && cardIndex >= 0) {
            std::string cardName = std::to_string(cardIndex);
            std::string hwName = "hw:"+cardName;

            snd_ctl_t* cardHandle = nullptr;

            if (snd_ctl_open(&cardHandle, hwName.c_str(), 0) < 0) {
                continue;
            }

            // enumerate soundcard devices
            int cardDeviceIndex = -1;
            if (snd_ctl_pcm_next_device(cardHandle, &cardDeviceIndex) < 0 || cardDeviceIndex < 0) {
                snd_ctl_close(cardHandle);
                continue;
            }

            // probe for name and index
            auto sc = std::make_unique<Soundcard>();
            sc->cardIndex = cardIndex;

            char* cstr_name = nullptr;
            if (snd_card_get_name(cardIndex, &cstr_name) >= 0 && cstr_name) {
                sc->name = cstr_name;
            }

            if (snd_card_get_longname(cardIndex, &cstr_name) >= 0 && cstr_name) {
                sc->longName = cstr_name;
            }

            // probe for supported sr and bs
            std::set<unsigned int> sampleRates;
            std::set<unsigned int> bufferSizes;

            std::vector<unsigned int> probeSR = {8000, 16000, 32000, 44100, 48000, 64000, 88200, 96000, 128000, 176400, 192000, 256000, 352800, 384000, 512000, 705600, 768000};
            std::vector<snd_pcm_uframes_t> probeBS = {};

            for (snd_pcm_uframes_t bs = 2; bs <= 8192; bs *= 2) {
                probeBS.push_back(bs);
            }

            char devName[128]; // you never know...
            snprintf(devName, sizeof(devName), "hw:%d,%d", cardIndex, cardDeviceIndex);

            snd_pcm_t* pcm = nullptr;

            if (snd_pcm_open(&pcm, devName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) == 0) { // SND_PCM_NONBLOCK or 0
                snd_pcm_hw_params_t* params = nullptr;

                if (snd_pcm_hw_params_malloc(&params) == 0) {
                    if (snd_pcm_hw_params_any(pcm, params) == 0) {
                        for (const auto rate : probeSR) {
                            if (snd_pcm_hw_params_test_rate(pcm, params, rate, 0) == 0 && !sampleRates.contains(rate)) {
                                sampleRates.insert(rate);
                            }
                        }

                        for (const auto bs : probeBS) {
                            if (snd_pcm_hw_params_test_buffer_size(pcm, params, bs) == 0) {
                                bufferSizes.insert(bs);
                            }
                        }
                    }
                }

                snd_pcm_hw_params_free(params);
            }

            snd_pcm_close(pcm);

            sc->supportedSampleRates.assign(sampleRates.begin(), sampleRates.end());
            sc->supportedBufferSizes.assign(bufferSizes.begin(), bufferSizes.end());

            scs->push_back(std::move(sc));
            snd_ctl_close(cardHandle);
        }

        return scs;
    }


    std::string vectorValuesToString(const std::vector<int>& vec) {
        std::string out;

        for (unsigned i = 0; i < vec.size(); i++) {
            if (i != 0) {
                out += ", ";
            }

            out += std::to_string(vec[i]);
        }

        return out;
    }

    std::vector<std::string> vectorValuesToStringsVec(const std::vector<int>& vec) {
        std::vector<std::string> out;

        for (const auto e : vec) {
            out.push_back(std::to_string(e));
        }

        return out;
    }

    // TODO: Implement it
    std::vector<int> mapSharedBufferSizes(const std::unique_ptr<Soundcards>& soundcards) {
        return {};
    }

    // TODO: Implement it
    std::vector<int> mapSharedSampleRates(const std::unique_ptr<Soundcards>& soundcards) {
        return {};
    }

    std::vector<int> getAllCardsSupportedSampleRates(const std::unique_ptr<Soundcards>& soundcards) {
        std::set<int> set;

        for (auto &card : *soundcards) {
            for (auto val : card->supportedSampleRates) {
                if (!set.contains(val)) set.insert(val);
            }
        }

        return std::vector<int>(set.begin(), set.end());
    }

    std::vector<int> getAllCardsSupportedBufferSizes(const std::unique_ptr<Soundcards>& soundcards) {
        std::set<int> set;

        for (auto &card : *soundcards) {
            for (auto val : card->supportedBufferSizes) {
                if (!set.contains(val)) set.insert(val);
            }
        }

        return std::vector<int>(set.begin(), set.end());
    }
}