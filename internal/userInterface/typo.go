package userInterface

import qt "github.com/mappu/miqt/qt6"

const (
	QFont_Thin       = 0 + 10
	QFont_ExtraLight = 12 * 10
	QFont_Light      = 25 * 10
	QFont_Normal     = 50 * 10
	QFont_Medium     = 57 * 10
	QFont_DemiBold   = 63 * 10
	QFont_Bold       = 75 * 10
	QFont_ExtraBold  = 81 * 10
	QFont_Black      = 87 * 10
)

func TitleFont() *qt.QFont {
	return qt.NewQFont7("Noto Sans", 16, QFont_Bold)
}
