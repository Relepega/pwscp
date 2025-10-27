#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Pipewire.h"

#include "alsa.h"
#include "src/utils/utils.h"

namespace AudioAPI {
     PipewireOption::PipewireOption(const int id, const std::string& key, const std::string& value, const std::string& type) {
         this->id = id;
         this->key = key;
         this->value = value;
         this->type = type;
     }

    int PipewireOption::getId() const { return this->id; }
    std::string PipewireOption::getKey() const { return this->key; }
    std::string PipewireOption::getValue() const { return this->value; }
    std::string PipewireOption::getType() const { return this->type; }

    int PipewireOption::setValue(const char* value, const char* type) {
        const std::string v = value ? value : this->value;
        const std::string t = type ? type : this->type;

         char cmd[512];
         sprintf(cmd, "pw-metadata -n settings 0 %s '%s' '%s'", this->key.c_str(), v.c_str(), t.c_str());

         auto cmdResult = utils::runSystemCommand(cmd);
         if (cmdResult->isError) {
             return -1;
         }

         this->value = v;
         this->type = t;

         return 0;
    }

    std::vector<std::string> pwOptClockAllowedRatesToStringArr(std::string s) {
         s.erase(0, 2);
         s.erase(s.length()-2, 2);

         unsigned l = 0;
         unsigned r = 0;

         std::vector<std::string> rates;

         while (r <= s.length()) {
             if (s[r] == ',' || r == s.length()) {
                 rates.push_back(s.substr(l, r-l));

                 r += 2;
                 l = r;

                 continue;
             }

             r++;
         }

         return rates;
    }

    // strips the `Found "settings" metadata XX` or eventual terminators from base output
    void PipewireConnection::pwMetadataStringStripUselessData(std::string& metadataCmdOut) {
         while (true) {
             if (metadataCmdOut[0] == '\0' || metadataCmdOut.empty()) {
                 break;
             }

             if (metadataCmdOut[0] == '\n') {
                 metadataCmdOut.erase(0, 1);
                 break;
             }

             metadataCmdOut.erase(0, 1);
         }
     }

    // f*** pipewire api, we're raw dogging
    std::vector<std::string> PipewireConnection::pwFetchMetadata(const std::string &key) {
         auto cmdResult = utils::runSystemCommand("pw-metadata -n settings 0 " + key);
         // std::cout << (cmdResult->isError ? cmdResult->error : cmdResult->combinedOutput) << "\n";

         if (cmdResult->isError) {
             std::cerr << "CMD: " << cmdResult->error;
             throw std::runtime_error("CMD: " + cmdResult->error);
             return {};
         }

         this->pwMetadataStringStripUselessData(cmdResult->combinedOutput);

         // std::cout << metadataString;

         // std::cout << "metadata string length: " << cmdResult->combinedOutput.length() << " chars\n";
         std::string line = "";
         std::vector<std::string> lines;

         for (unsigned i = 0; i < cmdResult->combinedOutput.length(); i++) {
             if (cmdResult->combinedOutput[i] == '\n' || cmdResult->combinedOutput[i] == '\0') {
                 // std::cout << i << "\n";
                 lines.push_back(line.erase(0, 8));
                 line = "";
             } else {
                 line += cmdResult->combinedOutput[i];
             }
         }

         return lines;
    }

    pwMetadataKV PipewireConnection::parseMetadataKV(const std::string& metadataLine, const unsigned start) {
         unsigned l = start;
         unsigned r = l;

         std::string k = "";
         std::string v = "";

         while (r < metadataLine.length()) {
             char currChar = metadataLine[r];

             if (currChar == '\0' || currChar == '\n') {
                 break;
             }

             if (currChar == ':') {
                 k = metadataLine.substr(l, r-l);
                 l = ++r;
                 continue;
             }

             if (currChar == '\'') {
                 l = ++r;
                 currChar = metadataLine[l];

                 while (currChar != '\'') {
                     currChar = metadataLine[r++];
                 }

                 v = metadataLine.substr(l, r-l-1);

                 if (v == "'") v = " ";

                 return pwMetadataKV {
                     .hasNext = l < metadataLine.length(),
                     .start = start,
                     .end = r,
                     .k = k,
                     .v = v,
                 };
             }

             if (currChar == ' ') {
                 v = metadataLine.substr(l, r-l);

                 return pwMetadataKV {
                     .hasNext = l < metadataLine.length(),
                     .start = start,
                     .end = r,
                     .k = k,
                     .v = v,
                 };
             }

             r++;
         }

         return pwMetadataKV {
             .hasNext = l < metadataLine.length(),
             .start = start,
             .end = r,
             .k = k,
             .v = "",
         };
    }

    PipewireConnection::PipewireConnection() {
         this->updateAllOpts();
    }

    PipewireOption PipewireConnection::getOption(const std::string &key) const {
         if (metadata.contains(key)) {
             return metadata.find(key)->second;
         }

         return PipewireOption(-1, key, "", "");
    }

    void PipewireConnection::updateAllOpts() {
         // fetch all metadata
         const auto metadataString = this->pwFetchMetadata("");

         // parse them
         for (const auto& l : metadataString) {
             // std::cout << "metadata line: " << l << "\n";

             auto kv = this->parseMetadataKV(l, 0);

             std::vector<std::string> keys = {};
             std::vector<std::string> values = {};

             keys.push_back(kv.k);
             values.push_back(kv.v);

             while (kv.hasNext) {
                 // std::cout << kv.k << ":" << kv.v << "\n";
                 kv = parseMetadataKV(l, kv.end+1);
                 keys.push_back(kv.k);
                 values.push_back(kv.v);
             }

             auto opt = PipewireOption(std::stoi(values[0]), values[1], values[2], values[3]);
             metadata.insert_or_assign(values[1], opt);

             // std::cout << "\n\n";
         }
    }

    void PipewireConnection::resetOptsToDefault() const {
         std::string allowedRates = "[ ";

         for (const auto sr : STD_SAMPLE_RATES) {
             allowedRates.append(std::to_string(sr) + ", ");
         }

         allowedRates = allowedRates.substr(0, allowedRates.length()-2);
         allowedRates.append(" ]");

         this->getOption(PW_OPT_LOG_LEVEL).setValue("2", "");
         this->getOption(PW_OPT_CLOCK_RATE).setValue("48000", "");
         this->getOption(PW_OPT_CLOCK_ALLOWED_RATES).setValue(allowedRates.c_str(), "");
         this->getOption(PW_OPT_CLOCK_QUANTUM).setValue("2048", "");
         this->getOption(PW_OPT_CLOCK_MIN_QUANTUM).setValue("32", "");
         this->getOption(PW_OPT_CLOCK_MAX_QUANTUM).setValue("8192", "");
         this->getOption(PW_OPT_CLOCK_FORCE_QUANTUM).setValue("0", "");
         this->getOption(PW_OPT_CLOCK_FORCE_RATE).setValue("0", "");
     }
}