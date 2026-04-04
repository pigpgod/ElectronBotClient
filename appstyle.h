/**
 * @file appstyle.h
 * @brief 应用程序样式管理类声明
 * 
 * 集中管理应用程序的视觉样式：
 * - 颜色定义（主色、成功色、警告色等）
 * - 字体样式
 * - 背景渐变样式
 * - 标签和按钮样式
 * - 状态徽章样式
 */

#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>

class AppStyle
{
public:
    static const QString primaryColor;
    static const QString successColor;
    static const QString warningColor;
    static const QString whiteColor;
    static const QString textPrimaryColor;
    static const QString textSecondaryColor;
    
    static QString primaryColorAlpha(double alpha);
    static QString successColorAlpha(double alpha);
    static QString whiteColorAlpha(double alpha);
    
    static const QString fontFamily;
    static QString font(int size, int weight = 400);
    
    static QString dialogBackground();
    static QString mainContainerBackground();
    static QString headerGradient();
    static QString videoDisplayBackground();
    static QString controlPanelBackground();
    static QString statusPanelBackground();
    
    static QString labelStyle(const QString &color, int size, int weight = 400);
    static QString buttonStyle(const QString &color);
    static QString statusBadgeStyle(const QString &color, const QString &bgColor);
};

#endif
