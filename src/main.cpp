#include <QApplication>
#include <QCoreApplication>

#include "appStrings/AppStrings.h"
#include "audioAPI/Pipewire.h"
#include "audioAPI/alsa.h"
#include "ui/mainWindow.h"

#include <iostream>

int main(int argc, char *argv[]) {
    try {
        QApplication a(argc, argv);

        a.setObjectName(QAnyStringView(appStrings::appNameWithVersion()));
        QCoreApplication::setApplicationName(appStrings::APP_NAME.data());
        QCoreApplication::setOrganizationName(appStrings::ORG_NAME.data());

        const userInterface::MainWindow mainWindow;

        return QApplication::exec();

        // auto adevs = AudioAPI::getAvailableAudioDevices();
        //
        // for (const auto & soundcard : *adevs) {
        //     std::cout << soundcard->longName << "\n";
        //     std::cout << "playback: " << soundcard->isPlaybackDevice << "\n";
        //     std::cout << "capture: " << soundcard->isCaptureDevice << "\n";
        //     std::cout << AudioAPI::vectorValuesToString(soundcard->supportedSampleRates) << "\n";
        //     std::cout << AudioAPI::vectorValuesToString(soundcard->supportedBufferSizes) << "\n";
        //
        //     std::cout << "\n";
        // }

        // const AudioAPI::PipewireConnection pwConnection = AudioAPI::PipewireConnection();
        // AudioAPI::pwOptClockAllowedRatesToStringArr(pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_ALLOWED_RATES).getValue());
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_LOG_LEVEL << ": " << pwConnection.getOption(AudioAPI::PW_OPT_LOG_LEVEL).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_RATE << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_RATE).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_ALLOWED_RATES << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_ALLOWED_RATES).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_QUANTUM << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_QUANTUM).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_MIN_QUANTUM << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_MIN_QUANTUM).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_MAX_QUANTUM << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_MAX_QUANTUM).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).getValue() << "\n";
        // std::cout << "Invoke from main: " << AudioAPI::PW_OPT_CLOCK_FORCE_RATE << ": " << pwConnection.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).getValue() << "\n";

        return 0;
    } catch (const std::exception& e) {
        qCritical() << "Initialization error:" << e.what();
        return -1;
    }
}