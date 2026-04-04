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
