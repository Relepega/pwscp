#ifndef PWSCP_PIPEWIRE_H
#define PWSCP_PIPEWIRE_H

#include <filesystem>
#include <map>
#include <string>

#include "Pipewire.h"


namespace AudioAPI {
    const std::string PW_OPT_LOG_LEVEL = "log.level";
    const std::string PW_OPT_CLOCK_RATE = "clock.rate";
    const std::string PW_OPT_CLOCK_ALLOWED_RATES = "clock.allowed-rates";
    const std::string PW_OPT_CLOCK_QUANTUM = "clock.quantum";
    const std::string PW_OPT_CLOCK_MIN_QUANTUM = "clock.min-quantum";
    const std::string PW_OPT_CLOCK_MAX_QUANTUM = "clock.max-quantum";
    const std::string PW_OPT_CLOCK_FORCE_QUANTUM = "clock.force-quantum";
    const std::string PW_OPT_CLOCK_FORCE_RATE = "clock.force-rate";

    // as of libpipewire 1.4.8
    class PipewireOption {
        int id;
        std::string key, value, type;

    public:
        PipewireOption(int id, const std::string &key, const std::string &value, const std::string &type);

        int getId() const;
        std::string getKey() const;
        std::string getValue() const;
        std::string getType() const;
        bool hasValue() const;
        int setValue(const char* value = nullptr, const char* type = nullptr);
    };

    std::vector<std::string> pwOptClockAllowedRatesToStringArr(std::string s);

    struct pwMetadataKV {
        bool hasNext;
        unsigned start, end;
        std::string k, v;
    };


    class PipewireConnection {
    private:
        std::map<std::string, PipewireOption> metadata;

        std::vector<std::string, std::allocator<std::string>> pwFetchMetadata(const std::string &key);
        static void pwMetadataStringStripUselessData(std::string& metadataCmdOut);
        pwMetadataKV parseMetadataKV(const std::string &metadataLine, unsigned start);

    public:
        PipewireConnection();
        ~PipewireConnection() = default;

        [[nodiscard]] PipewireOption getOption(const std::string& key) const;
        void updateAllOpts();
        void resetOptsToDefault() const;
    };
}

#endif // PWSCP_PIPEWIRE_H
