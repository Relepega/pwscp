#ifndef PWSCP_UTILS_H
#define PWSCP_UTILS_H

#include <string>
#include <memory>

namespace utils {
    struct processResult {
        bool isError;
        std::string error;
        std::string combinedOutput;
    };

   std::unique_ptr<processResult> runSystemCommand(const std::string& command);
}

#endif // PWSCP_UTILS_H
