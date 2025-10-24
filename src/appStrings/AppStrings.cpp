#include "AppStrings.h"

#include <sstream>

std::string appStrings::appNameWithVersion() {
    // return std::format("{} - Version {}", APP_NAME, VERSION); // throws error

    std::ostringstream stream;
    stream << appStrings::APP_NAME << " - Version " << appStrings::VERSION;
    return stream.str();
}
