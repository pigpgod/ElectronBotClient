/**
 * @file appstyle.cpp
 * @brief 应用程序样式管理类实现
 * 
 * 实现样式字符串生成函数：
 * - 颜色常量定义
 * - 带透明度的颜色生成
 * - CSS 样式字符串生成
 * - 机器人控制软件深色主题样式
 */

#include "appstyle.h"

const QString AppStyle::primaryColor = "#00D4FF";
const QString AppStyle::primaryColorLight = "#5CE1FF";
const QString AppStyle::accentColor = "#00FF88";
const QString AppStyle::successColor = "#00FF88";
const QString AppStyle::warningColor = "#FFB800";
const QString AppStyle::errorColor = "#FF3366";
const QString AppStyle::bgDark = "#0A0E14";
const QString AppStyle::bgPanel = "#111820";
const QString AppStyle::bgCard = "#1A2332";
const QString AppStyle::borderColor = "#2A3A4A";
const QString AppStyle::borderColorLight = "#3A4A5A";
const QString AppStyle::textPrimaryColor = "#E8F4FF";
const QString AppStyle::textSecondaryColor = "#8BA4B8";
const QString AppStyle::textMutedColor = "#5A7088";

QString AppStyle::primaryColorAlpha(double alpha)
{
    return QString("rgba(0, 212, 255, %1)").arg(alpha);
}

QString AppStyle::accentColorAlpha(double alpha)
{
    return QString("rgba(0, 255, 136, %1)").arg(alpha);
}

QString AppStyle::successColorAlpha(double alpha)
{
    return QString("rgba(0, 255, 136, %1)").arg(alpha);
}

const QString AppStyle::fontFamily = "Microsoft YaHei";

QString AppStyle::font(int size, int weight)
{
    return QString("font-family: %1; font-size: %2px; font-weight: %3;")
        .arg(fontFamily).arg(size).arg(weight);
}

QString AppStyle::dialogBackground()
{
    return "background: #111820; border: 1px solid #2A3A4A;";
}

QString AppStyle::mainContainerBackground()
{
    return "background: #0A0E14;";
}

QString AppStyle::panelBackground()
{
    return "background: #111820; border: 1px solid #2A3A4A;";
}

QString AppStyle::cardBackground()
{
    return "background: #1A2332; border: 1px solid #2A3A4A;";
}

QString AppStyle::videoDisplayBackground()
{
    return "background: #050810;";
}

QString AppStyle::labelStyle(const QString &color, int size, int weight)
{
    return QString("color: %1; %2").arg(color).arg(font(size, weight));
}

QString AppStyle::buttonStyle(const QString &color)
{
    return QString(
        "QPushButton {"
        "    background: transparent;"
        "    border: 1px solid %1;"
        "    border-radius: 4px;"
        "    color: %1;"
        "    font-family: %2;"
        "    font-size: 12px;"
        "    font-weight: 500;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "    background: %1;"
        "    color: #0A0E14;"
        "}"
        "QPushButton:disabled {"
        "    border-color: #3A4A5A;"
        "    color: #5A7088;"
        "}"
    ).arg(color).arg(fontFamily);
}

QString AppStyle::statusBadgeStyle(const QString &color, const QString &bgColor, int size)
{
    return QString(
        "color: %1;"
        "font-family: %2;"
        "font-size: %4px;"
        "font-weight: 600;"
        "padding: 3px 10px;"
        "background: %3;"
        "border: 1px solid %1;"
        "border-radius: 3px;"
    ).arg(color).arg(fontFamily).arg(bgColor).arg(size);
}
