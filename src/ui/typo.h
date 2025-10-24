#ifndef PWSCP_TYPO_H
#define PWSCP_TYPO_H

#include <QFont>
#include <QString>

namespace typo {
    constexpr int Qfont_Thin       = 0 + 10;
    constexpr int Qfont_ExtraLight = 12 * 10;
    constexpr int Qfont_Light      = 25 * 10;
    constexpr int Qfont_Normal     = 50 * 10;
    constexpr int Qfont_Medium     = 57 * 10;
    constexpr int Qfont_SemiBold   = 63 * 10;
    constexpr int Qfont_Bold       = 75 * 10;
    constexpr int Qfont_ExtraBold  = 81 * 10;
    constexpr int Qfont_Black      = 87 * 10;

    const QFont TitleFont = QFont(QString("Noto Sans"), 16, Qfont_Bold);
}

#endif
