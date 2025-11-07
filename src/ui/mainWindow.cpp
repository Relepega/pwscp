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
        this->adevs = AudioAPI::getAvailableAudioDevices();
        this->pwConn = AudioAPI::PipewireConnection();

        this->activeSR = this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).getValue() != "0" ?
                            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).getValue() :
                            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_RATE).getValue();

        this->activeBS = this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).getValue() != "0" ?
                            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).getValue() :
                            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_QUANTUM).getValue();

        this->setFixedSize(500, 315); // height is +25 if you want to add a statusbar
        this->setWindowTitle(QString(appStrings::appNameWithVersion().data()));

        /*
         * SYSTRAY MENU AND MENUBAR CREATION
         *
         */

        // std::unique_ptr<QIcon> appIcon = std::make_unique<QIcon>("");
        appIcon = std::make_unique<QIcon>("");
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

        action_Reload = new QAction();
        action_Reload -> setObjectName(QAnyStringView("Reload action"));
        action_Reload -> setText("Reload");
        action_Reload -> setShortcut(QKeySequence(Qt::CTRL|Qt::Key_R));
        QObject::connect(action_Reload, &QAction::triggered, [this]() -> void {
            this->disableUI();

            // free(this->adevs.release());
            // this->adevs = AudioAPI::getAvailableAudioDevices();
            this->adevs = AudioAPI::getAvailableAudioDevices();
            this->pwConn = AudioAPI::PipewireConnection();

            // pwConn.updateAllOpts();

            this->fillSampleRateComboBox();
            this->fillBufferSizeComboBox();

            this->enableUI();
        });

        action_Quit = new QAction();
        action_Quit -> setObjectName(QAnyStringView("Quit action"));
        action_Quit -> setText("Quit");
        action_Quit -> setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Q));
        QObject::connect(action_Quit, &QAction::triggered, [this]() -> void {
            trueCloseSignal = true;
            QCoreApplication::quit();
        });

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
        label_Title -> setGeometry(94, 10, 320, 35);

        label_SampleRate = new QLabel("Sample Rate label", centralWidget);
        label_SampleRate -> setText("Sample Rate");
        label_SampleRate -> setGeometry(114, 70, 85, 25);

        label_BufferSize = new QLabel("Buffer Size label", centralWidget);
        label_BufferSize -> setText("Buffer Size");
        label_BufferSize -> setGeometry(318, 70, 75, 25);

        // ComboBox - Row 2

        comboBox_SampleRate = new QComboBox(centralWidget);
        comboBox_SampleRate -> setObjectName("Sample Rate combo box");
        comboBox_SampleRate -> setGeometry(100, 120, 100, 32);
        comboBox_SampleRate -> setMaxVisibleItems(5);
        fillSampleRateComboBox();
        QObject::connect(comboBox_SampleRate, &QComboBox::currentIndexChanged, [this]() -> void { this->comboboxChanged(); });

        comboBox_BufferSize = new QComboBox(centralWidget);
        comboBox_BufferSize -> setObjectName("Buffer Size combo box");
        comboBox_BufferSize -> setGeometry(300, 120, 100, 32);
        comboBox_BufferSize -> setMaxVisibleItems(5);
        fillBufferSizeComboBox();
        QObject::connect(comboBox_BufferSize, &QComboBox::currentIndexChanged, [this]() -> void { this->comboboxChanged(); });

        // Operation Mode - Row 3

        forceOptions = new QCheckBox(centralWidget);
        forceOptions -> setObjectName("Force Options");
        forceOptions -> setGeometry(100, 190, 137, 23);
        forceOptions -> setText("Force Options");
        if (pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).hasValue() || pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).hasValue()) {
            forceOptions -> setChecked(true);
        } else {
            forceOptions -> setChecked(false);
        }
        QObject::connect(forceOptions, &QCheckBox::clicked, [this] -> void {
            this->applyChanges();
        });

        // Control Buttons - Row 4

        button_Reset = new QPushButton("Reset button", centralWidget);
        button_Reset -> setGeometry(10, 240, 140, 34);
        button_Reset -> setText("Reset to Default");
        QObject::connect(button_Reset, &QPushButton::pressed, [this]() ->void {
            this->disableUI();

            this->pwConn.resetOptsToDefault();

            this->activeSR = this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_RATE).getValue();
            this->activeBS = this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_QUANTUM).getValue();

            this->comboBox_SampleRate->setCurrentIndex(this->currSampleRateIndex());
            this->comboBox_BufferSize->setCurrentIndex(this->currBufferSizeIndex());

            this->enableUI(true);

            this->forceOptions->setChecked(false);
        });

        // add main widget to window
        this->setCentralWidget(centralWidget);

        show();
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::fillSampleRateComboBox() {
        comboBox_SampleRate -> clear();
        this->sampleRates = AudioAPI::getAllCardsSupportedSampleRates(adevs);

        for (const auto sc : this->sampleRates) {
            comboBox_SampleRate -> addItem(QString::fromStdString(std::to_string(sc)));
        }

        comboBox_SampleRate -> setCurrentIndex(this->currSampleRateIndex());
    }

    void MainWindow::fillBufferSizeComboBox() {
        comboBox_BufferSize -> clear();
        this->bufferSizes = AudioAPI::mapSharedBufferSizes(this->adevs);

        for (const auto sc : this->bufferSizes) {
            if (sc == 0 || (
                        sc >= std::stoi(pwConn.getOption(AudioAPI::PW_OPT_CLOCK_MIN_QUANTUM).getValue()) &&
                        sc <= std::stoi(pwConn.getOption(AudioAPI::PW_OPT_CLOCK_MAX_QUANTUM).getValue())
                    )
                ) {
                comboBox_BufferSize -> addItem(QString::fromStdString(std::to_string(sc)));
                }
        }

        comboBox_BufferSize -> setCurrentIndex(this->currBufferSizeIndex());
    }

    void MainWindow::disableUI() const {
        tray_Quit -> setEnabled(false);
        tray_Restore -> setEnabled(false);

        action_Quit -> setEnabled(false);
        action_Reload -> setEnabled(false);

        comboBox_BufferSize -> setEnabled(false);
        comboBox_SampleRate -> setEnabled(false);

        forceOptions -> setEnabled(false);

        button_Reset -> setEnabled(false);
    }

    void MainWindow::enableUI(const bool keepActionsDisabled) const {
        tray_Quit -> setEnabled(true);
        tray_Restore -> setEnabled(true);

        action_Quit -> setEnabled(true);
        action_Reload -> setEnabled(true);

        comboBox_BufferSize -> setEnabled(true);
        comboBox_SampleRate -> setEnabled(true);

        forceOptions -> setEnabled(true);

        button_Reset -> setEnabled(true);
    }

    void MainWindow::comboboxChanged() {
        std::cout << "changed\n";
        this->applyChanges();
    }

    void MainWindow::applyChanges() {
        this->disableUI();

        const std::string sr = this->comboBox_SampleRate->currentText().toStdString();
        const std::string bs = this->comboBox_BufferSize->currentText().toStdString();

        if (this->forceOptions->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).setValue(sr.c_str(), "");
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).setValue(bs.c_str(), "");
        } else {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).setValue("0", "");
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).setValue("0", "");
        }

        this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_RATE).setValue(sr.c_str(), "");
        this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_QUANTUM).setValue(bs.c_str(), "");

        this->activeSR = sr;
        this->activeBS = bs;

        this->enableUI(true);
    }

    int MainWindow::currSampleRateIndex() const {
        const std::vector<std::string> shared = AudioAPI::vectorValuesToStringsVec(this->sampleRates);
        const std::string defaultSR = std::to_string(AudioAPI::DEFAULT_SAMPLE_RATE);

        // search for active
        for (size_t i = 0; i < shared.size(); i++) {
            if (shared[i] == this->activeSR) {
                return i;
            }
        }

        // search for default index
        for (size_t i = 0; i < shared.size(); i++) {
            if (shared[i] == defaultSR) {
                return i;
            }
        }

        // if the default is not in the shared sample rates then use the first element in list
        return 0;
    }

    int MainWindow::currBufferSizeIndex() const {
        const std::vector<std::string> shared = AudioAPI::vectorValuesToStringsVec(this->bufferSizes);
        const std::string defaultBS = std::to_string(AudioAPI::DEFUALT_BUFFER_SIZE);

        // search for active
        for (size_t i = 0; i < shared.size(); i++) {
            if (shared[i] == this->activeBS) {
                return i;
            }
        }

        // if the default is not in the shared sample rates then use the first element in list
        return 0;
    }

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

    void MainWindow::onTrayActivated(const QSystemTrayIcon::ActivationReason reason) {
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