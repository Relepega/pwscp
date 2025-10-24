#include <QApplication>
#include <QCoreApplication>

#include "appStrings/AppStrings.h"
#include "audioAPI/alsa.h"
#include "ui/mainWindow.h"
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    a.setObjectName(QAnyStringView(appStrings::appNameWithVersion()));
    QCoreApplication::setApplicationName(appStrings::APP_NAME.data());
    QCoreApplication::setOrganizationName(appStrings::ORG_NAME.data());

    const userInterface::MainWindow mainWindow;

    return QApplication::exec();

    // auto soundcards = AudioAPI::getAvailableSoundcards();
    //
    // for (const auto & soundcard : *soundcards) {
    //     std::cout << soundcard->longName << "\n";
    //     std::cout << AudioAPI::vectorValuesToString(soundcard->supportedSampleRates) << "\n";
    //     std::cout << AudioAPI::vectorValuesToString(soundcard->supportedBufferSizes) << "\n";
    //
    //     std::cout << "\n";
    // }

}