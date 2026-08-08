#include <QApplication>
#include <QQuickStyle>

#include <QCoreApplication>
#include <QStyleFactory>

#include <iostream>

#include "appStrings/AppStrings.h"
#include "runGuard/RunGuard.h"
#include "ui/mainWindow.h"

int main(int argc, char *argv[]) {
    RunGuard guard( "pwscp_app" );
    if ( !guard.tryToRun() )
        return 0;

    try {
        QApplication app(argc, argv);

        app.setObjectName(QAnyStringView(appStrings::appNameWithVersion()));
        QCoreApplication::setApplicationName(appStrings::APP_NAME.data());
        QCoreApplication::setOrganizationName(appStrings::ORG_NAME.data());

        const userInterface::MainWindow mainWindow;

        return QApplication::exec();
    } catch (const std::exception& e) {
        qCritical() << "Initialization error:" << e.what();
        return -1;
    }
}