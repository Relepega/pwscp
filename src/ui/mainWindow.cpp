#include "mainWindow.h"

#include <QSystemTrayIcon>
#include <QCloseEvent>

#include <csignal>
#include <iostream>
#include <sstream>
#include <string>

#include "../appStrings/AppStrings.h"
#include "typo.h"

namespace userInterface {
    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
        soundcards = AudioAPI::getAvailableSoundcards();

        this->setFixedSize(500, 375); // height is 400 if you want to add a statusbar
        this->setWindowTitle(QString(appStrings::appNameWithVersion().data()));

        /*
         * SYSTRAY MENU AND MENUBAR CREATION
         *
         */

        appIcon = new QIcon("");
        sysTray = new QSystemTrayIcon(*appIcon, this);
        trayMenu = new QMenu(this);

        tray_Restore = new QAction("Restore", trayMenu);
        QObject::connect(tray_Restore, &QAction::triggered, this, &MainWindow::restoreFromTray);
        trayMenu->addAction(tray_Restore);

        tray_Quit = new QAction("Quit", trayMenu);
        QObject::connect(tray_Quit, &QAction::triggered, [this]() -> void {
            trueCloseSignal = true;
            sysTray->hide();
            this->hide();
            QCoreApplication::quit();
        });
        trayMenu->addAction(tray_Quit);

        sysTray->setContextMenu(trayMenu);
        QObject::connect(sysTray, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
        sysTray->setVisible(false);

        /*
         * MAIN MENU AND MENUBAR CREATION
         *
         */

        menuBar = QMainWindow::menuBar();

        menuBar_file = menuBar -> addMenu("File");
        menuBar_file -> setObjectName("File menu");

        action_Apply = new QAction();
        action_Apply -> setObjectName(QAnyStringView("Apply action"));
        action_Apply -> setText("Apply");
        action_Apply -> setShortcut(QKeySequence(Qt::CTRL|Qt::Key_S));
        action_Apply -> setEnabled(false);
        QObject::connect(action_Apply, &QAction::triggered, this, [=]() -> void {
            std::cout << "action_Apply placeholder\n";
        });

        action_Reload = new QAction();
        action_Reload -> setObjectName(QAnyStringView("Apply action"));
        action_Reload -> setText("Reload");
        action_Reload -> setShortcut(QKeySequence(Qt::CTRL|Qt::Key_R));
        QObject::connect(action_Reload, &QAction::triggered, [=]() -> void {
            std::cout << "action_Reload placeholder\n";
        });

        action_Quit = new QAction();
        action_Quit -> setObjectName(QAnyStringView("Apply action"));
        action_Quit -> setText("Quit");
        action_Quit -> setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Q));
        QObject::connect(action_Quit, &QAction::triggered, [this]() -> void {
            trueCloseSignal = true;
            QCoreApplication::quit();
        });

        menuBar_file -> addAction(action_Apply);
        menuBar_file -> addAction(action_Reload);
        menuBar_file -> addSeparator();
        menuBar_file -> addAction(action_Quit);

        /*
         * CREATE MAIN UI WIDGET
         *
         */

        centralWidget = new QWidget(this);
        centralWidget -> setObjectName(QAnyStringView("main window"));

        // Main Labels - Row 1

        label_Title = new QLabel(appStrings::APP_NAME.data(), centralWidget);
        label_Title -> setObjectName("App title label");
        label_Title -> setFont(typo::TitleFont);
        label_Title -> setGeometry(94, 10, 312, 29);

        label_SampleRate = new QLabel("Sample Rate label", centralWidget);
        label_SampleRate -> setText("Sample Rate");
        label_SampleRate -> setGeometry(114, 70, 78, 18);

        label_BufferSize = new QLabel("Buffer Size label", centralWidget);
        label_BufferSize -> setText("Buffer Size");
        label_BufferSize -> setGeometry(318, 70, 68, 18);

        // ComboBox - Row 2

        comboBox_SampleRate = new QComboBox(centralWidget);
        comboBox_SampleRate -> setObjectName("Sample Rate combo box");
        comboBox_SampleRate -> setGeometry(100, 120, 100, 32);
        comboBox_SampleRate -> setMaxVisibleItems(5);
        for (const auto sc : AudioAPI::getAllCardsSupportedSampleRates(soundcards)) {
            comboBox_SampleRate -> addItem(QString::fromStdString(std::to_string(sc)));
        }
        QObject::connect(comboBox_SampleRate, &QComboBox::currentIndexChanged, []() -> void {
            std::cout << "combobox sample rate index changed placeholder\n";
        });

        comboBox_BufferSize = new QComboBox(centralWidget);
        comboBox_BufferSize -> setObjectName("Buffer Size combo box");
        comboBox_BufferSize -> setGeometry(300, 120, 100, 32);
        comboBox_BufferSize -> setMaxVisibleItems(5);
        for (const auto sc : AudioAPI::getAllCardsSupportedBufferSizes(soundcards)) {
            comboBox_BufferSize -> addItem(QString::fromStdString(std::to_string(sc)));
        }
        QObject::connect(comboBox_BufferSize, &QComboBox::currentIndexChanged, []() -> void {
            std::cout << "combobox buffer size index changed placeholder\n";
        });

        // Operation Mode - Row 3
        groupBox_SampleRate = new QGroupBox(centralWidget);
        groupBox_SampleRate -> setGeometry(95, 165, 155, 95);
        groupBox_SampleRate -> setFlat(true);

        sampleRate_Suggest = new QRadioButton(groupBox_SampleRate);
        sampleRate_Suggest -> setObjectName("Suggest Sample Rate");
        sampleRate_Suggest -> setGeometry(5, 5, 137, 23);
        sampleRate_Suggest -> setText("Suggest");

        sampleRate_Force = new QRadioButton(groupBox_SampleRate);
        sampleRate_Force -> setObjectName("Force Sample Rate");
        sampleRate_Force -> setGeometry(5, 35, 137, 23);
        sampleRate_Force -> setText("Force");

        sampleRate_SuggestAndForce = new QRadioButton(groupBox_SampleRate);
        sampleRate_SuggestAndForce -> setObjectName("Suggest and Force Sample Rate");
        sampleRate_SuggestAndForce -> setGeometry(5, 65, 137, 23);
        sampleRate_SuggestAndForce -> setText("Suggest and Force");

        groupBox_BufferSize = new QGroupBox(centralWidget);
        groupBox_BufferSize -> setGeometry(295, 165, 155, 95);
        groupBox_BufferSize -> setFlat(true);

        bufferSize_Suggest = new QRadioButton(groupBox_BufferSize);
        bufferSize_Suggest -> setObjectName("Suggest buffer size");
        bufferSize_Suggest -> setGeometry(5, 5, 137, 23);
        bufferSize_Suggest -> setText("Suggest");

        bufferSize_Force = new QRadioButton(groupBox_BufferSize);
        bufferSize_Force -> setObjectName("Force buffer size");
        bufferSize_Force -> setGeometry(5, 35, 137, 23);
        bufferSize_Force -> setText("Force");

        bufferSize_SuggestAndForce = new QRadioButton(groupBox_BufferSize);
        bufferSize_SuggestAndForce -> setObjectName("Suggest and Force buffer size");
        bufferSize_SuggestAndForce -> setGeometry(5, 65, 137, 23);
        bufferSize_SuggestAndForce -> setText("Suggest and Force");

        // Control Buttons - Row 4

        button_Reset = new QPushButton("Reset button", centralWidget);
        button_Reset -> setGeometry(154, 297, 140, 34);
        button_Reset -> setText("Reset to Default");
        QObject::connect(button_Reset, &QPushButton::pressed, [=]() {
            std::cout << "Reset button onPressed placeholder\n";
        });

        button_Apply = new QPushButton("Reset button", centralWidget);
        button_Apply -> setGeometry(304, 297, 88, 34);
        button_Apply -> setText("Apply");
        QObject::connect(button_Apply, &QPushButton::pressed, [=]() {
            std::cout << "Reset button onPressed placeholder\n";
        });
        button_Apply -> setEnabled(false);

        button_Cancel = new QPushButton("Reset button", centralWidget);
        button_Cancel -> setGeometry(402, 297, 88, 34);
        button_Cancel -> setText("Cancel");
        QObject::connect(button_Cancel, &QPushButton::pressed, [=]() {
            std::cout << "Reset button onPressed placeholder\n";
        });
        button_Cancel -> setEnabled(false);


        // add main widget to window
        this->setCentralWidget(centralWidget);

        show();
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::closeEvent(QCloseEvent* event) {

        if (!trueCloseSignal) {
            sysTray->setVisible(true);
            this->hide();

            sysTray->showMessage(
                QStringLiteral("Minimized"),
                QString::fromStdString(appStrings::APP_NAME+" is running in the tray. Right-click to quit or restore."),
                QSystemTrayIcon::Information,
                3000
            );

            event->ignore();
        } else {
            event->accept();
        }
    }

    void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            restoreFromTray();
        }
    }

    void MainWindow::restoreFromTray() {
        this->showNormal();
        this->raise();
        this->activateWindow();

        sysTray->setVisible(false);
    }
}