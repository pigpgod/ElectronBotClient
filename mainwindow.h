#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QBuffer>
#include <QImage>
#include <QTime>
#include <QFileInfo>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QMouseEvent>
#include <QDialog>
#include <QProgressBar>
#include <QMessageBox>

#include "ffmpegvideoplayer.h"
#include "electron_low_level.h"

class CustomTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget *parent = nullptr);
    void setTitle(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onMinimizeClicked();
    void onMaximizeClicked();
    void onCloseClicked();

private:
    QLabel *titleLabel;
    QPushButton *btnMinimize;
    QPushButton *btnMaximize;
    QPushButton *btnClose;
    QPoint dragPosition;
    QWidget *parentWindow;
};

class GlowingButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor)

public:
    explicit GlowingButton(const QString &text, QWidget *parent = nullptr);
    QColor glowColor() const { return m_glowColor; }
    void setGlowColor(const QColor &color);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_glowColor;
    bool isHovered;
    int animValue;
    QTimer *animationTimer;
};

class StatusIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor ledColor READ ledColor WRITE setLedColor)

public:
    explicit StatusIndicator(QWidget *parent = nullptr);
    void setStatus(bool connected);
    QColor ledColor() const { return m_ledColor; }
    void setLedColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_ledColor;
    bool m_isConnected;
    QTimer *blinkTimer;
    bool showOn;
    int animValue;
};

class LoadingIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingIndicator(QWidget *parent = nullptr);
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
    int m_rotation;
    QTimer *m_timer;
};

class WaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitingDialog(const QString &title, const QString &message, QWidget *parent = nullptr);
    void setMessage(const QString &message);
    void setSuccess(const QString &message);
    void setFailed(const QString &message);
    void setProgressText(const QString &text);

private:
    QLabel *messageLabel;
    QLabel *statusLabel;
    LoadingIndicator *loadingIndicator;
    int animValue;
    QTimer *animationTimer;
    QTimer *scanLineTimer;

protected:
    void paintEvent(QPaintEvent *event) override;
};

class CustomMessageBox : public QDialog
{
    Q_OBJECT

public:
    enum IconType { Information, Warning, Error, Success };

    explicit CustomMessageBox(const QString &title, const QString &message, IconType icon, QWidget *parent = nullptr);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title, const QString &message);
    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title, const QString &message);
    static QMessageBox::StandardButton error(QWidget *parent, const QString &title, const QString &message);
    static QMessageBox::StandardButton question(QWidget *parent, const QString &title, const QString &message);

private:
    void setupUI(const QString &title, const QString &message, IconType icon);
    QPushButton *createButton(const QString &text, const QColor &color);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void openFile();
    void playVideo();
    void pauseVideo();
    void stopVideo();
    void connectToBot();
    void disconnectFromBot();
    void sendScreenData();
    void startScreenCapture();
    void stopScreenCapture();
    void onFrameReady(const QImage &image);
    void onConnectionStatusChanged(bool connected);
    void onConnectFinished(bool success);
    void onWaitingDialogClosed();

private:
    void setupUI();
    void setupConnections();
    void applyStyleSheet();
    void showWaitingDialog(const QString &title, const QString &message);
    void hideWaitingDialog();
    void showErrorDialog(const QString &title, const QString &message);
    void showSuccessDialog(const QString &title, const QString &message);

    CustomTitleBar *titleBar;
    QWidget *centralWidget;

    FFmpegVideoPlayer *videoPlayer;
    QLabel *videoDisplayLabel;

    GlowingButton *btnOpen;
    GlowingButton *btnPlay;
    GlowingButton *btnPause;
    GlowingButton *btnStop;
    GlowingButton *btnConnect;
    GlowingButton *btnStartCapture;
    GlowingButton *btnStopCapture;

    QLabel *labelStatus;
    StatusIndicator *statusLed;

    ElectronLowLevel *robot;
    bool isUsbConnected;
    QTimer *captureTimer;
    bool isCapturing;
    int captureInterval;
    bool isConnecting;

    WaitingDialog *waitingDialog;
};

#endif
