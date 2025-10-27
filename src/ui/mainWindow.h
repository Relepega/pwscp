#ifndef PWSCP_MAINWINDOW_H
#define PWSCP_MAINWINDOW_H

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSystemTrayIcon>
#include <QWidget>

#include "../audioAPI/alsa.h"
#include "src/audioAPI/Pipewire.h"

namespace userInterface {
    class MainWindow final : public QMainWindow {
        Q_OBJECT

    private:
        QWidget* centralWidget;

        std::unique_ptr<QIcon> appIcon;
        QSystemTrayIcon* sysTray;

        QMenu* trayMenu;
        QAction* tray_Restore;
        QAction* tray_Quit;

        QMenuBar* menuBar;

        QMenu*    menuBar_file;
        QAction*  action_Apply;
        QAction*  action_Reload;
        QAction*  action_Quit;

        QLabel* label_Title;
        QLabel* label_SampleRate;
        QLabel* label_BufferSize;

        QComboBox* comboBox_SampleRate;
        std::vector<int> sampleRates;
        QComboBox* comboBox_BufferSize;
        std::vector<int> bufferSizes;

        QGroupBox*    groupBox_SampleRate;
        QRadioButton* sampleRate_Suggest;
        QRadioButton* sampleRate_Force;
        QRadioButton* sampleRate_SuggestAndForce;

        QGroupBox*    groupBox_BufferSize;
        QRadioButton* bufferSize_Suggest;
        QRadioButton* bufferSize_Force;
        QRadioButton* bufferSize_SuggestAndForce;

        QPushButton* button_Reset;
        QPushButton* button_Apply;
        QPushButton* button_Cancel;

        bool trueCloseSignal = false;
        std::unique_ptr<AudioAPI::audioDevices> adevs;
        AudioAPI::PipewireConnection pwConn;

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        void restoreFromTray();

private slots:
        void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;
        void disableUI() const;
        void enableUI() const;
        void fillSampleRateComboBox();
        void fillBufferSizeComboBox();

        int currSampleRateIndex() const;
        int currBufferSizeIndex() const;
    };
}

#endif