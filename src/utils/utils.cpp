#include "utils.h"

namespace utils {
       std::unique_ptr<processResult> runSystemCommand(const std::string& command) {
        char buffer[128];
        std::string output = "";

        // Open pipe to file
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return std::make_unique<processResult>(true, "Popen failed", "");
        }

        // read till end of process:
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }

        // Close the pipe and check if there was an error
        int status = pclose(pipe);
        if (status == -1) {
            return std::make_unique<processResult>(true, "Error closing the pipe!", "");
        }

        return std::make_unique<processResult>(false, "", output);
    }
}