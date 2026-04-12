/**
 * @file mainwindow.h
 * @brief 主窗口类声明
 * 
 * 定义应用程序的主界面组件：
 * - CustomTitleBar: 自定义标题栏（支持拖拽、最小化、最大化、关闭）
 * - GlowingButton: 发光效果按钮
 * - StatusIndicator: 连接状态指示灯
 * - LoadingIndicator: 加载动画指示器
 * - WaitingDialog: 等待对话框
 * - CustomMessageBox: 自定义消息框
 * - CameraCapture: 摄像头捕获类
 * - MainWindow: 主窗口类
 */

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
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QCameraViewfinderSettings>

#include "ffmpegvideoplayer.h"
#include "electron_low_level.h"
#include "appstyle.h"
#include "voskrecognizer.h"

class CustomTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget *parent = 0);
    void setTitle(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

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
    explicit GlowingButton(const QString &text, QWidget *parent = 0);
    QColor glowColor() const { return m_glowColor; }
    void setGlowColor(const QColor &color);

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void onAnimationTimeout();

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
    explicit StatusIndicator(QWidget *parent = 0);
    void setStatus(bool connected);
    QColor ledColor() const { return m_ledColor; }
    void setLedColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onBlinkTimeout();

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
    explicit LoadingIndicator(QWidget *parent = 0);
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onRotationTimeout();

private:
    QColor m_color;
    int m_rotation;
    QTimer *m_timer;
};

class WaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitingDialog(const QString &title, const QString &message, QWidget *parent = 0);
    void setMessage(const QString &message);
    void setSuccess(const QString &message);
    void setFailed(const QString &message);
    void setProgressText(const QString &text);

private slots:
    void onAnimationTimeout();
    void onScanLineTimeout();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QLabel *messageLabel;
    QLabel *statusLabel;
    LoadingIndicator *loadingIndicator;
    int animValue;
    QTimer *animationTimer;
    QTimer *scanLineTimer;
    QWidget *containerWidget;
};

class CustomMessageBox : public QDialog
{
    Q_OBJECT

public:
    enum IconType { Information, Warning, Error, Success };

    explicit CustomMessageBox(const QString &title, const QString &message, IconType icon, QWidget *parent = 0);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title, const QString &message);
    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title, const QString &message);
    static QMessageBox::StandardButton error(QWidget *parent, const QString &title, const QString &message);
    static QMessageBox::StandardButton question(QWidget *parent, const QString &title, const QString &message);

private:
    void setupUI(const QString &title, const QString &message, IconType icon);
    QPushButton *createButton(const QString &text, const QColor &color);
};

class CameraCapture : public QObject
{
    Q_OBJECT

public:
    explicit CameraCapture(QObject *parent = 0);
    ~CameraCapture();
    bool startCapture();
    void stopCapture();
    bool isCapturing() const { return m_capturing; }
    void setDisplayLabel(QLabel *label);

signals:
    void frameReady(const QImage &image);

private slots:
    void onCameraStateChanged(QCamera::State state);
    void onImageCaptured(int id, const QImage &image);
    void onCaptureTimeout();

private:
    QCamera *m_camera;
    QCameraImageCapture *m_imageCapture;
    bool m_capturing;
    QLabel *m_displayLabel;
    QTimer *m_captureTimer;
    QImage m_lastFrame;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void connectToBot();
    void disconnectFromBot();
    void startCameraCapture();
    void stopCameraCapture();
    void onVideoFrameReady(const QImage &image);
    void onCameraFrameReady(const QImage &image);
    void onConnectionStatusChanged(bool connected);
    void onConnectFinished(bool success);
    void onWaitingDialogClosed();
    void onDisconnectTimeout();
    void onSliderValueChanged(int value);
    void onResetClicked();
    void onVoiceControlClicked();
    void onVoiceResultReady(const QString &text);
    void onVoicePartialResult(const QString &text);
    void onVoiceError(const QString &message);
    void onVoiceRecognitionStarted();
    void onVoiceRecognitionStopped();
    void processVoiceCommand(const QString &command);

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
    CameraCapture *cameraCapture;
    QLabel *videoDisplayLabel;

    GlowingButton *btnConnect;
    GlowingButton *btnStartCapture;
    GlowingButton *btnStopCapture;
    GlowingButton *btnReset;
    GlowingButton *btnVoiceControl;

    QLabel *voiceResultLabel;

    QSlider *jointSliders[6];
    QLabel *jointValueLabels[6];
    QLabel *jointNameLabels[6];

    QLabel *labelStatus;
    QLabel *versionLabel;
    StatusIndicator *statusLed;

    ElectronLowLevel *robot;
    VoskRecognizer *voskRecognizer;
    bool isUsbConnected;
    bool isCameraCapturing;
    bool isConnecting;
    bool isVoiceActive;

    WaitingDialog *waitingDialog;
};

#endif
