package appStrings

import "fmt"

const (
	APP_NAME = "Pipewire Simple Control Panel"
	VERSION  = "0.1.0"
	ORG_NAME = ""
)

func AppNameWithVersion() string {
	return fmt.Sprintf("%s - Version %s", APP_NAME, VERSION)
}
