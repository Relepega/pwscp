#include <QApplication>
#include <QStyleFactory>
#include <QString>
#include <cstdlib>

#include "platformTheme.h"

#include <iostream>

namespace userInterface {
    QString detectDesktopEnvironment() {
        QString de = "fusion";

        if (const char *xdgCurrentDesktop = std::getenv("XDG_CURRENT_DESKTOP")) {
            QString desktop = QString(xdgCurrentDesktop).toLower();

            if (desktop.contains("gnome") || desktop.contains("ubuntu")) {
                de = "gnome";
            } else if (desktop.contains("kde") || desktop.contains("plasma")) {
                de = "kde";
            } else if (desktop.contains("xfce")) {
                de = "xfce";
            } else if (desktop.contains("lxqt")) {
                de = "lxqt";
            }
        }

        std::cout << "desktop: " << de.toStdString() << '\n';

        return de;
    }

    void setupPlatformTheme() {
        const QString de = detectDesktopEnvironment();

        // Don't override if already set
        // if (std::getenv("QT_QPA_PLATFORMTHEME")) {
        //     return;
        // }

        // if (de == "gnome" || de == "xfce") {
        //     qputenv("QT_QPA_PLATFORMTHEME", "gtk3");
        // } else
        if (de == "kde") {
            qputenv("QT_QPA_PLATFORMTHEME", "kde");
        } else {
            qputenv("QT_QPA_PLATFORMTHEME", "fusion");
        }

        // For Wayland sessions
        if (QString(std::getenv("XDG_SESSION_TYPE")).toLower() == "wayland") {
            // Qt6 should auto-detect, but put it into venv just to be sure
            if (!std::getenv("QT_QPA_PLATFORM")) {
                qputenv("QT_QPA_PLATFORM", "wayland");
            }
        }
    }
}
