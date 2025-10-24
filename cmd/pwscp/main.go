package main

import (
	"log"
	"os"

	qt "github.com/mappu/miqt/qt6"

	"github.com/relepega/pwscp/internal/appStrings"
	"github.com/relepega/pwscp/internal/pipewireInterface"
	"github.com/relepega/pwscp/internal/userInterface"
)

func main() {
	app := qt.NewQApplication(os.Args)

	app.SetObjectName(*qt.NewQAnyStringView3(appStrings.APP_NAME))
	qt.QCoreApplication_SetApplicationName(appStrings.APP_NAME)
	qt.QCoreApplication_SetOrganizationName(appStrings.ORG_NAME)

	mainWindow := userInterface.NewMainWindowUi()

	userInterface.NewSystemTray(mainWindow)

	mainWindow.MainWindow.Show()

	qt.QApplication_Exec()

	_, err := pipewireInterface.NewPipewireOptions()
	if err != nil {
		log.Fatalln(err)
	}
}
