#include "alsa.h"

#include <algorithm>
#include <alsa/asoundlib.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace AudioAPI {
    std::vector<int> STD_BUFFER_SIZES() {
        std::vector<int> bsVec = {};

        for (int bs = 2; bs <= 8192; bs *= 2) {
            bsVec.push_back(bs);
        }

        return bsVec;
    };

    audioDevices getAvailableAudioDevices() {
        audioDevices adevs;

        int cardIndex = -1;

        while (snd_card_next(&cardIndex) >= 0 && cardIndex >= 0) {
            std::string cardName = std::to_string(cardIndex);
            std::string hwName = "hw:" + cardName;

            snd_ctl_t* cardHandle = nullptr;

            if (snd_ctl_open(&cardHandle, hwName.c_str(), 0) < 0) {
                std::cerr << "Could not open card at index " << cardIndex << std::endl;
                continue;
            }

            // enumerate devices on current card
            int cardDeviceIndex = -1;
            while (snd_ctl_pcm_next_device(cardHandle, &cardDeviceIndex) >= 0 && cardDeviceIndex >= 0) {
                char devName[128];
                snprintf(devName, sizeof(devName), "hw:%d,%d", cardIndex, cardDeviceIndex);

                auto sc = std::make_unique<audioDevice>();
                sc->cardIndex = cardIndex;
                sc->cardHardwareName = hwName;

                snd_pcm_t* pcmPlayback = nullptr;
                snd_pcm_t* pcmCapture = nullptr;

                sc->isPlaybackDevice = (snd_pcm_open(&pcmPlayback, devName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) == 0);
                sc->isCaptureDevice = (snd_pcm_open(&pcmCapture, devName, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK) == 0);

                // Close the test handles
                if (pcmPlayback) {
                    snd_pcm_close(pcmPlayback);
                    pcmPlayback = nullptr;
                }
                if (pcmCapture) {
                    snd_pcm_close(pcmCapture);
                    pcmCapture = nullptr;
                }

                // get card name (same for all devices on this card)
                char* cstr_name = nullptr;
                if (snd_card_get_name(cardIndex, &cstr_name) >= 0 && cstr_name) {
                    sc->name = cstr_name;
                    // append device index if there are multiple devices
                    if (cardDeviceIndex > 0) {
                        sc->name += " [Device " + std::to_string(cardDeviceIndex) + "]";
                    }
                }

                if (snd_card_get_longname(cardIndex, &cstr_name) >= 0 && cstr_name) {
                    sc->longName = cstr_name;
                    // same here
                    if (cardDeviceIndex > 0) {
                        sc->longName += " [Device " + std::to_string(cardDeviceIndex) + "]";
                    }
                }

                free(cstr_name);

                // probe for supported sample rates and buffer sizes
                std::set<unsigned int> sampleRates;
                std::set<unsigned int> bufferSizes;


                snd_pcm_t* pcm = nullptr;

                if (snd_pcm_open(&pcm, devName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) == 0) { // 0 or SND_PCM_NONBLOCK
                    snd_pcm_hw_params_t* params = nullptr;

                    if (snd_pcm_hw_params_malloc(&params) == 0) {
                        if (snd_pcm_hw_params_any(pcm, params) == 0) {
                            for (const auto rate : STD_SAMPLE_RATES) {
                                if (snd_pcm_hw_params_test_rate(pcm, params, rate, 0) == 0 && !sampleRates.contains(rate)) {
                                    sampleRates.insert(rate);
                                }
                            }

                            for (const auto bs : STD_BUFFER_SIZES()) {
                                if (snd_pcm_hw_params_test_buffer_size(pcm, params, bs) == 0) {
                                    bufferSizes.insert(bs);
                                }
                            }
                        }
                    }

                    if (params) {
                        snd_pcm_hw_params_free(params);
                        params = nullptr;
                    }
                }

                if (pcm) {
                    snd_pcm_close(pcm);
                    pcm = nullptr;
                }

                sc->supportedSampleRates.assign(sampleRates.begin(), sampleRates.end());
                sc->supportedBufferSizes.assign(bufferSizes.begin(), bufferSizes.end());

                adevs.push_back(std::move(sc));
            }

            snd_ctl_close(cardHandle);
        }

        return adevs;
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

    std::vector<int> mapSharedBufferSizes(const audioDevices& adevs) {
        if (adevs.empty()) {
            return { 0, 512, 1024 };
        }

        std::set<int> shared;
        bool isSetInitialized = false;

        for (const auto& dev : adevs) {
            if (dev->supportedBufferSizes.empty()) continue;

            if (!isSetInitialized) {
                // set the buffer sizes from first device
                shared.insert(adevs.front()->supportedBufferSizes.begin(), adevs.front()->supportedBufferSizes.end());
                isSetInitialized = true;
            }

            // intersect with each subsequent device's buffer sizes
            std::set<int> current(dev->supportedBufferSizes.begin(), dev->supportedBufferSizes.end());

            std::set<int> intersection;
            std::ranges::set_intersection(shared, current, std::inserter(intersection, intersection.begin()));

            shared = std::move(intersection);

            // no common buffer sizes
            if (shared.empty()) {
                return {0};
            }
        }

        auto res = std::vector<int>(shared.begin(), shared.end());
        res.emplace(res.begin(), 0);

        return res;
    }

    std::vector<int> mapSharedSampleRates(const audioDevices& adevs) {
        if (adevs.empty()) {
            return { 0, 44100, 48000 };
        }

        std::set<int> shared = {0};

        for (const auto &adev : adevs) {
            shared.insert(adev->supportedSampleRates.begin(), adev->supportedSampleRates.end());
        }

        std::vector<int> setToVec(shared.begin(), shared.end());
        std::ranges::sort(setToVec);

        return setToVec;
    }

    std::vector<int> getAllCardsSupportedSampleRates(const audioDevices& adevs) {
        if (adevs.empty()) {
            return { 0, 44100, 48000 };
        }

        std::set<int> set = {0};

        for (auto &adev : adevs) {
            for (auto val : adev->supportedSampleRates) {
                if (!set.contains(val)) set.insert(val);
            }
        }

        return std::vector<int>(set.begin(), set.end());
    }

    std::vector<int> getAllCardsSupportedBufferSizes(const audioDevices& adevs) {
        if (adevs.empty()) {
            return { 0, 512, 1024 };
        }

        std::set<int> set = {0};

        for (auto &adev : adevs) {
            for (auto val : adev->supportedBufferSizes) {
                if (!set.contains(val)) set.insert(val);
            }
        }

        return std::vector<int>(set.begin(), set.end());
    }
}