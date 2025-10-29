#include <QApplication>
#include <QCoreApplication>
#include <QStyleFactory>

#include <iostream>

#include "appStrings/AppStrings.h"

#include "audioAPI/Pipewire.h"
#include "audioAPI/alsa.h"

#include "ui/mainWindow.h"

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);

        // QApplication::setStyle(QStyleFactory::create("fusion"));
        QApplication::setStyle(QStyleFactory::create("gtk4"));

        app.setObjectName(QAnyStringView(appStrings::appNameWithVersion()));
        QCoreApplication::setApplicationName(appStrings::APP_NAME.data());
        QCoreApplication::setOrganizationName(appStrings::ORG_NAME.data());

        const userInterface::MainWindow mainWindow;

        return app.exec();
    } catch (const std::exception& e) {
        qCritical() << "Initialization error:" << e.what();
        return -1;
    }
}