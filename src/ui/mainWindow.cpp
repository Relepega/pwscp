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

        this->setFixedSize(500, 375); // height is 400 if you want to add a statusbar
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

        action_Apply = new QAction();
        action_Apply -> setObjectName(QAnyStringView("Apply action"));
        action_Apply -> setText("Apply");
        action_Apply -> setShortcut(QKeySequence(Qt::CTRL|Qt::Key_S));
        action_Apply -> setEnabled(false);
        QObject::connect(action_Apply, &QAction::triggered, this, [this]() -> void { this->applyChanges(); });

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
        fillSampleRateComboBox();
        QObject::connect(comboBox_SampleRate, &QComboBox::currentIndexChanged, [this]() -> void { this->comboboxChanged(); });

        comboBox_BufferSize = new QComboBox(centralWidget);
        comboBox_BufferSize -> setObjectName("Buffer Size combo box");
        comboBox_BufferSize -> setGeometry(300, 120, 100, 32);
        comboBox_BufferSize -> setMaxVisibleItems(5);
        fillBufferSizeComboBox();
        QObject::connect(comboBox_BufferSize, &QComboBox::currentIndexChanged, [this]() -> void { this->comboboxChanged(); });

        // Operation Mode - Row 3

        groupBox_SampleRate = new QGroupBox(centralWidget);
        groupBox_SampleRate -> setGeometry(95, 165, 155, 95);
        groupBox_SampleRate -> setFlat(true);

        sampleRate_Suggest = new QRadioButton(groupBox_SampleRate);
        sampleRate_Suggest -> setObjectName("Suggest Sample Rate");
        sampleRate_Suggest -> setGeometry(5, 5, 137, 23);
        sampleRate_Suggest -> setText("Suggest");
        sampleRate_Suggest -> setChecked(true);

        sampleRate_Force = new QRadioButton(groupBox_SampleRate);
        sampleRate_Force -> setObjectName("Force Sample Rate");
        sampleRate_Force -> setGeometry(5, 35, 137, 23);
        sampleRate_Force -> setText("Force");
        sampleRate_Force -> setChecked(false);

        sampleRate_SuggestAndForce = new QRadioButton(groupBox_SampleRate);
        sampleRate_SuggestAndForce -> setObjectName("Suggest and Force Sample Rate");
        sampleRate_SuggestAndForce -> setGeometry(5, 65, 137, 23);
        sampleRate_SuggestAndForce -> setText("Suggest and Force");
        sampleRate_SuggestAndForce -> setChecked(false);

        groupBox_BufferSize = new QGroupBox(centralWidget);
        groupBox_BufferSize -> setGeometry(295, 165, 155, 95);
        groupBox_BufferSize -> setFlat(true);

        bufferSize_Suggest = new QRadioButton(groupBox_BufferSize);
        bufferSize_Suggest -> setObjectName("Suggest buffer size");
        bufferSize_Suggest -> setGeometry(5, 5, 137, 23);
        bufferSize_Suggest -> setText("Suggest");
        bufferSize_Suggest -> setChecked(true);

        bufferSize_Force = new QRadioButton(groupBox_BufferSize);
        bufferSize_Force -> setObjectName("Force buffer size");
        bufferSize_Force -> setGeometry(5, 35, 137, 23);
        bufferSize_Force -> setText("Force");
        bufferSize_Force -> setChecked(false);

        bufferSize_SuggestAndForce = new QRadioButton(groupBox_BufferSize);
        bufferSize_SuggestAndForce -> setObjectName("Suggest and Force buffer size");
        bufferSize_SuggestAndForce -> setGeometry(5, 65, 137, 23);
        bufferSize_SuggestAndForce -> setText("Suggest and Force");
        bufferSize_SuggestAndForce -> setChecked(false);

        // Control Buttons - Row 4

        button_Reset = new QPushButton("Reset button", centralWidget);
        button_Reset -> setGeometry(154, 297, 140, 34);
        button_Reset -> setText("Reset to Default");
        QObject::connect(button_Reset, &QPushButton::pressed, [this]() ->void {
            this->disableUI();

            this->pwConn.resetOptsToDefault();
            this->enableUI();
            // TODO: track new defaults, set comboboxes to them and update ui actions/buttons accordingly

            this->button_Apply->setEnabled(false);
            this->button_Cancel->setEnabled(false);
            this->action_Apply->setEnabled(false);
        });

        button_Apply = new QPushButton("Apply button", centralWidget);
        button_Apply -> setGeometry(304, 297, 88, 34);
        button_Apply -> setText("Apply");
        QObject::connect(button_Apply, &QPushButton::pressed, [this]() -> void { this->applyChanges(); });
        button_Apply -> setEnabled(false);

        button_Cancel = new QPushButton("Cancel button", centralWidget);
        button_Cancel -> setGeometry(402, 297, 88, 34);
        button_Cancel -> setText("Cancel");
        QObject::connect(button_Cancel, &QPushButton::pressed, [this]() -> void {
            this->disableUI();

            this->comboBox_SampleRate->setCurrentIndex(this->currSampleRateIndex());
            this->comboBox_SampleRate->setCurrentIndex(this->currBufferSizeIndex());

            this->enableUI();
        });
        button_Cancel -> setEnabled(false);


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

        action_Apply -> setEnabled(false);
        action_Quit -> setEnabled(false);
        action_Reload -> setEnabled(false);

        comboBox_BufferSize -> setEnabled(false);
        comboBox_SampleRate -> setEnabled(false);

        sampleRate_Force -> setEnabled(false);
        sampleRate_Suggest -> setEnabled(false);
        sampleRate_SuggestAndForce -> setEnabled(false);

        bufferSize_Force -> setEnabled(false);
        bufferSize_Suggest -> setEnabled(false);
        bufferSize_SuggestAndForce -> setEnabled(false);

        button_Apply -> setEnabled(false);
        button_Cancel -> setEnabled(false);
        button_Reset -> setEnabled(false);
    }

    void MainWindow::enableUI() const {
        tray_Quit -> setEnabled(true);
        tray_Restore -> setEnabled(true);

        action_Apply -> setEnabled(true);
        action_Quit -> setEnabled(true);
        action_Reload -> setEnabled(true);

        comboBox_BufferSize -> setEnabled(true);
        comboBox_SampleRate -> setEnabled(true);

        sampleRate_Force -> setEnabled(true);
        sampleRate_Suggest -> setEnabled(true);
        sampleRate_SuggestAndForce -> setEnabled(true);

        bufferSize_Force -> setEnabled(true);
        bufferSize_Suggest -> setEnabled(true);
        bufferSize_SuggestAndForce -> setEnabled(true);

        button_Apply -> setEnabled(true);
        button_Cancel -> setEnabled(true);
        button_Reset -> setEnabled(true);
    }

    void MainWindow::comboboxChanged() const {
        const std::string sr = this->comboBox_SampleRate->currentText().toStdString();
        const std::string bs = this->comboBox_BufferSize->currentText().toStdString();

        if (sr != this->activeSR || bs != this->activeBS) {
            this->button_Apply->setEnabled(true);
            this->button_Cancel->setEnabled(true);
            this->action_Apply->setEnabled(true);
        } else {
            this->button_Apply->setEnabled(false);
            this->button_Cancel->setEnabled(false);
            this->action_Apply->setEnabled(false);
        }
    }

    void MainWindow::applyChanges() const {
        this->disableUI();

        const std::string sr = this->comboBox_SampleRate->currentText().toStdString();
        const std::string bs = this->comboBox_BufferSize->currentText().toStdString();

        if (this->sampleRate_Force->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).setValue(sr.c_str());
        } else if (this->sampleRate_Suggest->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_RATE).setValue(sr.c_str());
        } else if (this->sampleRate_SuggestAndForce->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_RATE).setValue(sr.c_str());
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_RATE).setValue(sr.c_str());
        }

        if (this->bufferSize_Force->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).setValue(bs.c_str());
        } else if (this->bufferSize_Suggest->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_QUANTUM).setValue(bs.c_str());
        } else if (this->bufferSize_SuggestAndForce->isChecked()) {
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_QUANTUM).setValue(bs.c_str());
            this->pwConn.getOption(AudioAPI::PW_OPT_CLOCK_FORCE_QUANTUM).setValue(bs.c_str());
        }

        this->enableUI();
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

        // search for default index
        for (size_t i = 0; i < shared.size(); i++) {
            if (shared[i] == defaultBS) {
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