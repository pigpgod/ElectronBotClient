#include "appstyle.h"

const QString AppStyle::primaryColor = "#E63946";
const QString AppStyle::successColor = "#4CAF50";
const QString AppStyle::warningColor = "#FF9800";
const QString AppStyle::whiteColor = "#ffffff";
const QString AppStyle::textPrimaryColor = "#e8e8e8";
const QString AppStyle::textSecondaryColor = "rgba(255,255,255,0.5)";

QString AppStyle::primaryColorAlpha(double alpha)
{
    return QString("rgba(230, 57, 70, %1)").arg(alpha);
}

QString AppStyle::successColorAlpha(double alpha)
{
    return QString("rgba(76, 175, 80, %1)").arg(alpha);
}

QString AppStyle::whiteColorAlpha(double alpha)
{
    return QString("rgba(255, 255, 255, %1)").arg(alpha);
}

const QString AppStyle::fontFamily = "'Segoe UI', Arial";

QString AppStyle::font(int size, int weight)
{
    return QString("font-family: %1; font-size: %2px; font-weight: %3;")
        .arg(fontFamily).arg(size).arg(weight);
}

QString AppStyle::dialogBackground()
{
    return "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #16161a, stop:1 #111114);";
}

QString AppStyle::mainContainerBackground()
{
    return "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #121214, stop:0.3 #151518, stop:0.7 #131315, stop:1 #0e0e10);";
}

QString AppStyle::headerGradient()
{
    return QString("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 %2);")
        .arg(primaryColorAlpha(0.18)).arg(primaryColorAlpha(0.06));
}

QString AppStyle::videoDisplayBackground()
{
    return "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0d0d0d, stop:0.5 #101010, stop:1 #141414);";
}

QString AppStyle::controlPanelBackground()
{
    return "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(22, 22, 26, 0.95), stop:1 rgba(16, 16, 19, 0.95));";
}

QString AppStyle::statusPanelBackground()
{
    return "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(28, 28, 33, 0.95), stop:1 rgba(19, 19, 23, 0.95));";
}

QString AppStyle::labelStyle(const QString &color, int size, int weight)
{
    return QString("color: %1; %2").arg(color).arg(font(size, weight));
}

QString AppStyle::buttonStyle(const QString &color)
{
    return QString(R"(
        QPushButton {
            background: rgba(%1, 0.15);
            border: 2px solid rgba(%1, 0.7);
            border-radius: 8px;
            color: rgb(%1);
            font-family: %2;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover {
            background: rgba(%1, 0.3);
            border-color: rgb(%1);
        }
    )").arg(color).arg(fontFamily);
}

QString AppStyle::statusBadgeStyle(const QString &color, const QString &bgColor)
{
    return QString(R"(
        color: %1;
        font-family: %2;
        font-size: 12px;
        font-weight: 700;
        padding: 4px 12px;
        background: %3;
        border-radius: 4px;
    )").arg(color).arg(fontFamily).arg(bgColor);
}
