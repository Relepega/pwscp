package userInterface

import (
	qt "github.com/mappu/miqt/qt6"

	"github.com/relepega/pwscp/internal/appStrings"
)

func NewSystemTray(ui *MainWindowUi) {
	trayIcon := qt.NewQIcon4("")
	tray := qt.NewQSystemTrayIcon4(trayIcon, ui.MainWindow.QObject)
	trayMenu := qt.NewQMenu2()

	trayMenu_action_restore := qt.NewQAction2("Restore")
	trayMenu.AddAction(trayMenu_action_restore)

	trayMenu_action_quit := qt.NewQAction2("Quit")
	trayMenu.AddAction(trayMenu_action_quit)

	tray.SetContextMenu(trayMenu)

	trayMenu_action_restore.OnTriggeredWithChecked(func(checked bool) {
		ui.MainWindow.Show()
		ui.MainWindow.ActivateWindow()
		ui.MainWindow.Raise()
	})

	trayMenu_action_quit.OnTriggeredWithChecked(func(checked bool) {
		tray.Hide()
		ui.MainWindow.Hide()
	})

	tray.Show()

	ui.MainWindow.OnCloseEvent(func(super func(event *qt.QCloseEvent), event *qt.QCloseEvent) {
		ui.MainWindow.Hide()
		tray.ShowMessage3(
			"Minimized",
			appStrings.APP_NAME+" is running in the tray. Right-click to quit or restore.",
			trayIcon,
			3000,
		)
		event.Ignore()
	})

	tray.OnActivated(func(reason qt.QSystemTrayIcon__ActivationReason) {
		if reason == qt.QSystemTrayIcon__Trigger || reason == qt.QSystemTrayIcon__DoubleClick {
			if ui.MainWindow.IsVisible() {
				ui.MainWindow.Hide()
			} else {
				ui.MainWindow.Show()
				ui.MainWindow.ActivateWindow()
				ui.MainWindow.Raise()
			}
		}
	})
}
