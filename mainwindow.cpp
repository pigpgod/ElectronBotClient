/**
 * @file mainwindow.cpp
 * @brief 主窗口类实现
 * 
 * 实现应用程序的主界面功能：
 * - 自定义标题栏的拖拽和窗口控制
 * - 科技风格按钮样式
 * - 状态指示灯的显示和闪烁
 * - 等待对话框的动画效果
 * - 视频播放和摄像头捕获切换
 * - USB 设备连接管理
 */

#include "mainwindow.h"
#include <QPainterPath>
#include <QtMath>
#include <QCoreApplication>

CustomTitleBar::CustomTitleBar(QWidget *parent)
    : QWidget(parent)
    , parentWindow(parent)
{
    setFixedHeight(40);
    setStyleSheet("background: #111820; border-bottom: 1px solid #2A3A4A;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(8);

    QLabel *iconLabel = new QLabel("⚡", this);
    iconLabel->setStyleSheet("color: #00D4FF; font-size: 20px;");

    titleLabel = new QLabel("ELECTRONBOT CONTROLLER", this);
    titleLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textPrimaryColor, 14, 600));

    QLabel *versionLabel = new QLabel("v3.0", this);
    versionLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 12, 400));

    layout->addWidget(iconLabel);
    layout->addSpacing(4);
    layout->addWidget(titleLabel);
    layout->addSpacing(6);
    layout->addWidget(versionLabel);
    layout->addStretch();

    QString btnStyle = QString(
        "QPushButton {"
        "    background: transparent;"
        "    border: none;"
        "    color: %1;"
        "    font-size: 14px;"
        "    padding: 6px 10px;"
        "}"
        "QPushButton:hover {"
        "    background: #1A2332;"
        "}"
    ).arg(AppStyle::textSecondaryColor);

    btnMinimize = new QPushButton("─", this);
    btnMinimize->setStyleSheet(btnStyle);
    btnMinimize->setFixedSize(36, 28);

    btnMaximize = new QPushButton("□", this);
    btnMaximize->setStyleSheet(btnStyle);
    btnMaximize->setFixedSize(36, 28);

    btnClose = new QPushButton("✕");
    QString closeStyle = btnStyle + QString(
        "QPushButton:hover {"
        "    background: #FF3366;"
        "    color: white;"
        "}"
    );
    btnClose->setStyleSheet(closeStyle);
    btnClose->setFixedSize(36, 28);

    layout->addWidget(btnMinimize);
    layout->addWidget(btnMaximize);
    layout->addWidget(btnClose);

    connect(btnMinimize, SIGNAL(clicked()), this, SLOT(onMinimizeClicked()));
    connect(btnMaximize, SIGNAL(clicked()), this, SLOT(onMaximizeClicked()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
}

void CustomTitleBar::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - parentWindow->frameGeometry().topLeft();
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        parentWindow->move(event->globalPos() - dragPosition);
    }
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent *)
{
}

void CustomTitleBar::onMinimizeClicked()
{
    parentWindow->showMinimized();
}

void CustomTitleBar::onMaximizeClicked()
{
    if (parentWindow->isMaximized()) {
        parentWindow->showNormal();
        btnMaximize->setText("□");
    } else {
        parentWindow->showMaximized();
        btnMaximize->setText("❐");
    }
}

void CustomTitleBar::onCloseClicked()
{
    parentWindow->close();
}

GlowingButton::GlowingButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_glowColor(0, 212, 255)
    , isHovered(false)
    , animValue(0)
{
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(42);
    setMinimumWidth(130);
    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(onAnimationTimeout()));
}

void GlowingButton::onAnimationTimeout()
{
    animValue = (animValue + 3) % 360;
    update();
}

void GlowingButton::enterEvent(QEvent *)
{
    isHovered = true;
    animationTimer->start(20);
}

void GlowingButton::leaveEvent(QEvent *)
{
    isHovered = false;
    animationTimer->stop();
    update();
}

void GlowingButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();
    int radius = 4;

    if (isHovered) {
        QRadialGradient glow(QPointF(rect.center()), rect.width() * 0.8);
        glow.setColorAt(0, QColor(m_glowColor.red(), m_glowColor.green(), m_glowColor.blue(), 60));
        glow.setColorAt(1, QColor(m_glowColor.red(), m_glowColor.green(), m_glowColor.blue(), 0));
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect.adjusted(-4, -4, 4, 4), radius + 2, radius + 2);
    }

    QPen borderPen(m_glowColor, 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(0, 0, -1, -1), radius, radius);

    QFont font = this->font();
    font.setFamily(AppStyle::fontFamily);
    font.setBold(true);
    font.setPointSize(11);
    painter.setFont(font);
    painter.setPen(m_glowColor);
    painter.drawText(rect, Qt::AlignCenter, text());
}

void GlowingButton::setGlowColor(const QColor &color)
{
    m_glowColor = color;
    update();
}

StatusIndicator::StatusIndicator(QWidget *parent)
    : QWidget(parent)
    , m_ledColor(Qt::red)
    , m_isConnected(false)
    , showOn(true)
    , animValue(0)
{
    setFixedSize(10, 10);

    blinkTimer = new QTimer(this);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(onBlinkTimeout()));
}

void StatusIndicator::setStatus(bool connected)
{
    m_isConnected = connected;
    m_ledColor = connected ? QColor(0, 255, 136) : QColor(255, 51, 102);
    if (connected) {
        blinkTimer->start(500);
    } else {
        blinkTimer->stop();
        showOn = true;
    }
    update();
}

void StatusIndicator::onBlinkTimeout()
{
    showOn = !showOn;
    update();
}

void StatusIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color = m_ledColor;
    if (!showOn && m_isConnected) {
        color = color.darker(200);
    }

    QRadialGradient gradient(rect().center(), 5);
    gradient.setColorAt(0, color.lighter(150));
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(1, color.darker(150));

    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect());

    painter.setPen(QPen(color, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect().adjusted(-1, -1, 0, 0));
}

void StatusIndicator::setLedColor(const QColor &color)
{
    m_ledColor = color;
    update();
}

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
    , m_color(0, 212, 255)
    , m_rotation(0)
{
    setFixedSize(40, 40);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRotationTimeout()));
    m_timer->start(40);
}

void LoadingIndicator::onRotationTimeout()
{
    m_rotation = (m_rotation + 30) % 360;
    update();
}

void LoadingIndicator::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void LoadingIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPointF center(width() / 2.0, height() / 2.0);
    double radius = width() / 2.0 - 4;

    painter.setPen(QPen(m_color, 3, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    
    int spanAngle = 90 * 16;
    int startAngle = (m_rotation - 90) * 16;
    painter.drawArc(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2),
                    startAngle, spanAngle);
    
    painter.setPen(QPen(QColor(m_color.red(), m_color.green(), m_color.blue(), 100), 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2),
                    startAngle + spanAngle, 360 * 16 - spanAngle);
}

WaitingDialog::WaitingDialog(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent)
    , animValue(0)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setFixedSize(320, 160);
    setStyleSheet("background: #111820; border: 1px solid #2A3A4A;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);
    mainLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 14, 600));

    loadingIndicator = new LoadingIndicator(this);

    messageLabel = new QLabel(message, this);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textSecondaryColor, 13, 400));

    statusLabel = new QLabel("请稍候...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 12, 400));

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(loadingIndicator, 0, Qt::AlignCenter);
    mainLayout->addWidget(messageLabel);
    mainLayout->addWidget(statusLabel, 0, Qt::AlignCenter);

    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(onAnimationTimeout()));
    animationTimer->start(30);
}

void WaitingDialog::onAnimationTimeout()
{
    animValue = (animValue + 2) % 360;
}

void WaitingDialog::onScanLineTimeout()
{
    update();
}

void WaitingDialog::setMessage(const QString &message)
{
    messageLabel->setText(message);
}

void WaitingDialog::setSuccess(const QString &message)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::successColor, 10, 500));
    animationTimer->stop();
    loadingIndicator->setColor(QColor(0, 255, 136));
}

void WaitingDialog::setFailed(const QString &message)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(AppStyle::labelStyle("#FF3366", 10, 500));
    animationTimer->stop();
    loadingIndicator->setColor(QColor(255, 51, 102));
}

void WaitingDialog::paintEvent(QPaintEvent *)
{
}

void WaitingDialog::setProgressText(const QString &text)
{
    statusLabel->setText(text);
}

CustomMessageBox::CustomMessageBox(const QString &title, const QString &message, IconType icon, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setFixedSize(340, 150);
    setupUI(title, message, icon);
}

void CustomMessageBox::setupUI(const QString &title, const QString &message, IconType icon)
{
    setStyleSheet("background: #111820; border: 1px solid #2A3A4A;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(this);
    QString iconText, iconColor;
    switch (icon) {
        case Information: iconText = "ℹ"; iconColor = AppStyle::primaryColor; break;
        case Warning: iconText = "⚠"; iconColor = AppStyle::warningColor; break;
        case Error: iconText = "✕"; iconColor = "#FF3366"; break;
        case Success: iconText = "✓"; iconColor = AppStyle::successColor; break;
    }
    iconLabel->setText(iconText);
    iconLabel->setStyleSheet(AppStyle::labelStyle(iconColor, 18, 700));

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textPrimaryColor, 14, 600));

    titleLayout->addWidget(iconLabel);
    titleLayout->addSpacing(6);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    QLabel *msgLabel = new QLabel(message, this);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textSecondaryColor, 13, 400));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *okBtn = createButton("确定", QColor(0, 212, 255));
    btnLayout->addWidget(okBtn);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));

    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(msgLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}

QPushButton *CustomMessageBox::createButton(const QString &text, const QColor &color)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setFixedSize(70, 28);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(QString(
        "QPushButton {"
        "    background: transparent;"
        "    border: 1px solid %1;"
        "    border-radius: 3px;"
        "    color: %1;"
        "    font-family: %2;"
        "    font-size: 11px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background: %1;"
        "    color: #0A0E14;"
        "}"
    ).arg(color.name()).arg(AppStyle::fontFamily));
    return btn;
}

QMessageBox::StandardButton CustomMessageBox::information(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(title, message, Information, parent);
    box.exec();
    return QMessageBox::Ok;
}

QMessageBox::StandardButton CustomMessageBox::warning(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(title, message, Warning, parent);
    box.exec();
    return QMessageBox::Ok;
}

QMessageBox::StandardButton CustomMessageBox::error(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(title, message, Error, parent);
    box.exec();
    return QMessageBox::Ok;
}

QMessageBox::StandardButton CustomMessageBox::question(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(title, message, Success, parent);
    box.exec();
    return QMessageBox::Ok;
}

CameraCapture::CameraCapture(QObject *parent)
    : QObject(parent)
    , m_camera(0)
    , m_imageCapture(0)
    , m_capturing(false)
    , m_displayLabel(0)
    , m_captureTimer(0)
{
    m_captureTimer = new QTimer(this);
    connect(m_captureTimer, SIGNAL(timeout()), this, SLOT(onCaptureTimeout()));
}

CameraCapture::~CameraCapture()
{
    stopCapture();
}

void CameraCapture::setDisplayLabel(QLabel *label)
{
    m_displayLabel = label;
}

bool CameraCapture::startCapture()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        return false;
    }

    m_camera = new QCamera(cameras.first(), this);
    connect(m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(onCameraStateChanged(QCamera::State)));

    m_imageCapture = new QCameraImageCapture(m_camera, this);
    m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    connect(m_imageCapture, SIGNAL(imageCaptured(int, QImage)), this, SLOT(onImageCaptured(int, QImage)));

    m_camera->setCaptureMode(QCamera::CaptureStillImage);
    m_camera->start();

    m_capturing = true;
    m_captureTimer->start(33);

    return true;
}

void CameraCapture::stopCapture()
{
    m_capturing = false;
    m_captureTimer->stop();

    if (m_camera) {
        m_camera->stop();
        delete m_camera;
        m_camera = 0;
    }

    if (m_imageCapture) {
        delete m_imageCapture;
        m_imageCapture = 0;
    }
}

void CameraCapture::onCameraStateChanged(QCamera::State state)
{
    Q_UNUSED(state);
}

void CameraCapture::onImageCaptured(int id, const QImage &image)
{
    Q_UNUSED(id);

    m_lastFrame = image;

    if (m_displayLabel) {
        QPixmap pixmap = QPixmap::fromImage(image);
        m_displayLabel->setPixmap(pixmap.scaled(m_displayLabel->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
    }

    emit frameReady(image);
}

void CameraCapture::onCaptureTimeout()
{
    if (m_camera && m_imageCapture && m_capturing) {
        if (m_camera->state() == QCamera::ActiveState) {
            if (m_imageCapture->isReadyForCapture()) {
                m_imageCapture->capture();
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , videoPlayer(0)
    , cameraCapture(0)
    , videoDisplayLabel(0)
    , robot(0)
    , voskRecognizer(0)
    , isUsbConnected(false)
    , isCameraCapturing(false)
    , isConnecting(false)
    , isVoiceActive(false)
    , waitingDialog(0)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    videoPlayer = new FFmpegVideoPlayer(this);
    cameraCapture = new CameraCapture(this);

    voskRecognizer = new VoskRecognizer(this);
    QString modelPath = QCoreApplication::applicationDirPath() + "/vosk-models/vosk-model-small-cn-0.3";
    voskRecognizer->setModelPath(modelPath);

    setupUI();
    setupConnections();

    videoPlayer->setDisplayWidget(videoDisplayLabel);
    videoPlayer->setLooping(true);

    if (videoPlayer->loadVideo(":/res/happy.mp4")) {
        videoPlayer->play();
    }

    resize(1100, 750);
}

MainWindow::~MainWindow()
{
    if (voskRecognizer && voskRecognizer->isRunning()) {
        voskRecognizer->stop();
    }
    if (waitingDialog) {
        waitingDialog->close();
        delete waitingDialog;
    }
    if (robot) {
        if (isUsbConnected) {
            robot->Disconnect();
        }
        delete robot;
    }
    if (cameraCapture) {
        cameraCapture->stopCapture();
        delete cameraCapture;
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::setupUI()
{
    QWidget *mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer");
    mainContainer->setStyleSheet("background: #0A0E14;");
    setCentralWidget(mainContainer);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    titleBar = new CustomTitleBar(this);
    mainLayout->addWidget(titleBar);

    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    mainLayout->addWidget(centralWidget);

    QHBoxLayout *centerHLayout = new QHBoxLayout(centralWidget);
    centerHLayout->setContentsMargins(8, 8, 8, 8);
    centerHLayout->setSpacing(8);

    QWidget *leftPanel = new QWidget(this);
    leftPanel->setStyleSheet("background: #111820; border: 1px solid #2A3A4A;");
    leftPanel->setFixedWidth(240);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(12, 12, 12, 12);
    leftLayout->setSpacing(8);

    QLabel *controlHeader = new QLabel("◆ 控制面板", this);
    controlHeader->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 13, 600));
    leftLayout->addWidget(controlHeader);

    QFrame *line1 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("background: #2A3A4A;");
    line1->setFixedHeight(1);
    leftLayout->addWidget(line1);

    btnConnect = new GlowingButton("连接设备", this);
    btnConnect->setGlowColor(QColor(0, 212, 255));

    btnStartCapture = new GlowingButton("摄像头捕获", this);
    btnStartCapture->setGlowColor(QColor(0, 255, 136));

    btnStopCapture = new GlowingButton("停止捕获", this);
    btnStopCapture->setGlowColor(QColor(255, 184, 0));

    leftLayout->addWidget(btnConnect);
    leftLayout->addWidget(btnStartCapture);
    leftLayout->addWidget(btnStopCapture);

    btnVoiceControl = new GlowingButton("语音控制", this);
    btnVoiceControl->setGlowColor(QColor(178, 102, 255));
    leftLayout->addWidget(btnVoiceControl);

    voiceResultLabel = new QLabel("语音: 未启动", this);
    voiceResultLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 11, 400));
    voiceResultLabel->setWordWrap(true);
    voiceResultLabel->setMaximumHeight(40);
    leftLayout->addWidget(voiceResultLabel);

    QLabel *motorHeader = new QLabel("◆ 电机控制", this);
    motorHeader->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 13, 600));
    leftLayout->addWidget(motorHeader);

    QFrame *line3 = new QFrame(this);
    line3->setFrameShape(QFrame::HLine);
    line3->setStyleSheet("background: #2A3A4A;");
    line3->setFixedHeight(1);
    leftLayout->addWidget(line3);

    QString jointNames[6] = {"J1 头部", "J2 左肩部", "J3 左手", "J4 右肩部", "J5 右手", "J6 底座"};
    QString sliderStyle = QString(
        "QSlider::groove:horizontal {"
        "    border: 1px solid #2A3A4A;"
        "    height: 6px;"
        "    background: #1A2332;"
        "    border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: %1;"
        "    border: 1px solid %1;"
        "    width: 14px;"
        "    margin: -5px 0;"
        "    border-radius: 7px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: %1;"
        "    border-radius: 3px;"
        "}"
    ).arg(AppStyle::primaryColor);

    for (int i = 0; i < 6; ++i) {
        QWidget *sliderWidget = new QWidget(this);
        QVBoxLayout *sliderLayout = new QVBoxLayout(sliderWidget);
        sliderLayout->setContentsMargins(0, 4, 0, 4);
        sliderLayout->setSpacing(2);

        QHBoxLayout *labelLayout = new QHBoxLayout();
        jointNameLabels[i] = new QLabel(jointNames[i], this);
        jointNameLabels[i]->setStyleSheet(AppStyle::labelStyle(AppStyle::textSecondaryColor, 11, 500));
        
        jointValueLabels[i] = new QLabel("0°", this);
        jointValueLabels[i]->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 11, 600));
        jointValueLabels[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        labelLayout->addWidget(jointNameLabels[i]);
        labelLayout->addWidget(jointValueLabels[i]);

        jointSliders[i] = new QSlider(Qt::Horizontal, this);
        jointSliders[i]->setRange(-180, 180);
        jointSliders[i]->setValue(0);
        jointSliders[i]->setStyleSheet(sliderStyle);
        jointSliders[i]->setFixedHeight(20);

        sliderLayout->addLayout(labelLayout);
        sliderLayout->addWidget(jointSliders[i]);

        leftLayout->addWidget(sliderWidget);
    }

    btnReset = new GlowingButton("复位电机", this);
    btnReset->setGlowColor(QColor(255, 184, 0));
    leftLayout->addWidget(btnReset);

    leftLayout->addStretch();

    QLabel *statusHeader = new QLabel("◆ 状态信息", this);
    statusHeader->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 13, 600));
    leftLayout->addWidget(statusHeader);

    QFrame *line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("background: #2A3A4A;");
    line2->setFixedHeight(1);
    leftLayout->addWidget(line2);

    QWidget *statusInfo = new QWidget(this);
    QVBoxLayout *statusInfoLayout = new QVBoxLayout(statusInfo);
    statusInfoLayout->setContentsMargins(0, 8, 0, 0);
    statusInfoLayout->setSpacing(6);

    QHBoxLayout *connStatus = new QHBoxLayout();
    statusLed = new StatusIndicator(this);
    QLabel *connLabel = new QLabel("连接状态:", this);
    connLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textSecondaryColor, 12, 400));
    labelStatus = new QLabel("未连接", this);
    labelStatus->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 12, 500));
    connStatus->addWidget(statusLed);
    connStatus->addWidget(connLabel);
    connStatus->addWidget(labelStatus);
    connStatus->addStretch();

    QHBoxLayout *versionLayout = new QHBoxLayout();
    QLabel *verLabel = new QLabel("版本:", this);
    verLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 12, 400));
    versionLabel = new QLabel("v3.0", this);
    versionLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 12, 400));
    versionLayout->addWidget(verLabel);
    versionLayout->addWidget(versionLabel);
    versionLayout->addStretch();

    statusInfoLayout->addLayout(connStatus);
    statusInfoLayout->addLayout(versionLayout);
    leftLayout->addWidget(statusInfo);

    QWidget *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background: #111820; border: 1px solid #2A3A4A;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *videoHeader = new QWidget(this);
    videoHeader->setFixedHeight(32);
    videoHeader->setStyleSheet("background: #0D1318; border-bottom: 1px solid #2A3A4A;");
    QHBoxLayout *videoHeaderLayout = new QHBoxLayout(videoHeader);
    videoHeaderLayout->setContentsMargins(12, 0, 12, 0);

    QLabel *videoTitle = new QLabel("◆ 视频预览", this);
    videoTitle->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 13, 600));

    QLabel *videoStatus = new QLabel("● LIVE", this);
    videoStatus->setStyleSheet(AppStyle::statusBadgeStyle(AppStyle::successColor, AppStyle::successColorAlpha(0.15), 12));

    videoHeaderLayout->addWidget(videoTitle);
    videoHeaderLayout->addStretch();
    videoHeaderLayout->addWidget(videoStatus);

    videoDisplayLabel = new QLabel(this);
    videoDisplayLabel->setMinimumSize(640, 480);
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    videoDisplayLabel->setStyleSheet("background: #050810; border: none;");

    rightLayout->addWidget(videoHeader);
    rightLayout->addWidget(videoDisplayLabel, 1);

    centerHLayout->addWidget(leftPanel);
    centerHLayout->addWidget(rightPanel, 1);

    btnStopCapture->setEnabled(false);
    btnStartCapture->setEnabled(false);

    robot = new ElectronLowLevel(this);
    cameraCapture->setDisplayLabel(videoDisplayLabel);
}

void MainWindow::setupConnections()
{
    connect(btnConnect, SIGNAL(clicked()), this, SLOT(connectToBot()));
    connect(btnStartCapture, SIGNAL(clicked()), this, SLOT(startCameraCapture()));
    connect(btnStopCapture, SIGNAL(clicked()), this, SLOT(stopCameraCapture()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(onResetClicked()));
    connect(btnVoiceControl, SIGNAL(clicked()), this, SLOT(onVoiceControlClicked()));

    for (int i = 0; i < 6; ++i) {
        connect(jointSliders[i], SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    }

    connect(videoPlayer, SIGNAL(frameReady(QImage)), this, SLOT(onVideoFrameReady(QImage)));
    connect(cameraCapture, SIGNAL(frameReady(QImage)), this, SLOT(onCameraFrameReady(QImage)));
    connect(robot, SIGNAL(connectionStatusChanged(bool)), this, SLOT(onConnectionStatusChanged(bool)));
    connect(robot, SIGNAL(connectFinished(bool)), this, SLOT(onConnectFinished(bool)));

    connect(voskRecognizer, SIGNAL(resultReady(QString)), this, SLOT(onVoiceResultReady(QString)));
    connect(voskRecognizer, SIGNAL(partialResultReady(QString)), this, SLOT(onVoicePartialResult(QString)));
    connect(voskRecognizer, SIGNAL(errorOccurred(QString)), this, SLOT(onVoiceError(QString)));
    connect(voskRecognizer, SIGNAL(recognitionStarted()), this, SLOT(onVoiceRecognitionStarted()));
    connect(voskRecognizer, SIGNAL(recognitionStopped()), this, SLOT(onVoiceRecognitionStopped()));
}

void MainWindow::showWaitingDialog(const QString &title, const QString &message)
{
    if (!waitingDialog) {
        waitingDialog = new WaitingDialog(title, message, this);
    }
    waitingDialog->setMessage(message);
    waitingDialog->show();
    waitingDialog->activateWindow();
}

void MainWindow::hideWaitingDialog()
{
    if (waitingDialog) {
        waitingDialog->hide();
    }
}

void MainWindow::showErrorDialog(const QString &title, const QString &message)
{
    CustomMessageBox::error(this, title, message);
}

void MainWindow::showSuccessDialog(const QString &title, const QString &message)
{
    CustomMessageBox::information(this, title, message);
}

void MainWindow::connectToBot()
{
    if (isUsbConnected) {
        disconnectFromBot();
        return;
    }

    if (isConnecting) {
        return;
    }

    if (!robot->Connect()) {
        showErrorDialog("连接失败", "无法连接到 ElectronBot。\n请检查设备是否已通过 USB 连接。");
        return;
    }

    isConnecting = true;
    btnConnect->setEnabled(false);

    showWaitingDialog("正在连接", "正在扫描 ElectronBot...");
}

void MainWindow::onWaitingDialogClosed()
{
    isConnecting = false;
    btnConnect->setEnabled(true);
}

void MainWindow::disconnectFromBot()
{
    showWaitingDialog("正在断开", "正在关闭 USB 连接...");
    btnConnect->setEnabled(false);

    QTimer::singleShot(100, this, SLOT(onDisconnectTimeout()));
}

void MainWindow::onDisconnectTimeout()
{
    robot->Disconnect();
    hideWaitingDialog();
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    isUsbConnected = connected;

    if (connected) {
        isConnecting = false;

        hideWaitingDialog();

        statusLed->setStatus(true);
        labelStatus->setStyleSheet(AppStyle::labelStyle(AppStyle::successColor, 12, 500));
        labelStatus->setText("已连接");
        btnConnect->setText("断开连接");
        btnConnect->setGlowColor(QColor(255, 51, 102));
        btnConnect->setEnabled(true);
        btnStartCapture->setEnabled(true);
    } else {
        if (isConnecting) {
            return;
        }

        stopCameraCapture();
        statusLed->setStatus(false);
        labelStatus->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 12, 500));
        labelStatus->setText("未连接");
        btnConnect->setText("连接设备");
        btnConnect->setGlowColor(QColor(0, 212, 255));
        btnConnect->setEnabled(true);
        btnStartCapture->setEnabled(false);
    }
}

void MainWindow::onConnectFinished(bool success)
{
    if (!success && !isUsbConnected) {
        hideWaitingDialog();
        showErrorDialog("连接失败", "无法连接到 ElectronBot。\n请检查设备是否已通过 USB 连接。");
        btnConnect->setEnabled(true);
        isConnecting = false;
    }
}

void MainWindow::startCameraCapture()
{
    if (!isUsbConnected) {
        showErrorDialog("未连接", "请先连接 ElectronBot。");
        return;
    }

    videoPlayer->pause();

    if (!cameraCapture->startCapture()) {
        videoPlayer->play();
        showErrorDialog("摄像头错误", "无法访问摄像头。\n请检查摄像头是否已连接。");
        return;
    }

    isCameraCapturing = true;
    btnStartCapture->setEnabled(false);
    btnStopCapture->setEnabled(true);
    labelStatus->setStyleSheet(AppStyle::labelStyle("#00FF88", 12, 500));
    labelStatus->setText("捕获中");
}

void MainWindow::stopCameraCapture()
{
    if (!isCameraCapturing) return;

    cameraCapture->stopCapture();
    isCameraCapturing = false;
    btnStartCapture->setEnabled(true);
    btnStopCapture->setEnabled(false);

    videoPlayer->play();

    if (isUsbConnected) {
        labelStatus->setStyleSheet(AppStyle::labelStyle(AppStyle::successColor, 12, 500));
        labelStatus->setText("已连接");
    }
}

void MainWindow::onVideoFrameReady(const QImage &image)
{
    if (!isUsbConnected || isCameraCapturing) {
        return;
    }

    robot->SetImageSrc(image);
}

void MainWindow::onCameraFrameReady(const QImage &image)
{
    if (!isUsbConnected || !isCameraCapturing) {
        return;
    }

    robot->SetImageSrc(image);
}

void MainWindow::onSliderValueChanged(int value)
{
    Q_UNUSED(value);

    QSlider *senderSlider = qobject_cast<QSlider*>(sender());
    if (!senderSlider) return;

    for (int i = 0; i < 6; ++i) {
        if (jointSliders[i] == senderSlider) {
            int angle = jointSliders[i]->value();
            jointValueLabels[i]->setText(QString("%1°").arg(angle));
            break;
        }
    }

    if (isUsbConnected) {
        float angles[6];
        for (int i = 0; i < 6; ++i) {
            angles[i] = static_cast<float>(jointSliders[i]->value());
        }
        robot->SetJointAngles(angles[0], angles[1], angles[2], angles[3], angles[4], angles[5], true);
    }
}

void MainWindow::onResetClicked()
{
    for (int i = 0; i < 6; ++i) {
        jointSliders[i]->blockSignals(true);
        jointSliders[i]->setValue(0);
        jointValueLabels[i]->setText("0°");
        jointSliders[i]->blockSignals(false);
    }

    if (isUsbConnected) {
        robot->SetJointAngles(0, 0, 0, 0, 0, 0, true);
    }
}

void MainWindow::onVoiceControlClicked()
{
    if (isVoiceActive) {
        voskRecognizer->stop();
    } else {
        voskRecognizer->start();
    }
}

void MainWindow::onVoiceResultReady(const QString &text)
{
    voiceResultLabel->setText(QString("语音: %1").arg(text));
    voiceResultLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::successColor, 11, 500));
    processVoiceCommand(text);
}

void MainWindow::onVoicePartialResult(const QString &text)
{
    voiceResultLabel->setText(QString("识别中: %1").arg(text));
    voiceResultLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::primaryColor, 11, 400));
}

void MainWindow::onVoiceError(const QString &message)
{
    voiceResultLabel->setText(QString("语音错误: %1").arg(message));
    voiceResultLabel->setStyleSheet(AppStyle::labelStyle("#FF3366", 11, 500));
    isVoiceActive = false;
    btnVoiceControl->setText("语音控制");
    btnVoiceControl->setGlowColor(QColor(178, 102, 255));
}

void MainWindow::onVoiceRecognitionStarted()
{
    isVoiceActive = true;
    btnVoiceControl->setText("停止语音");
    btnVoiceControl->setGlowColor(QColor(255, 51, 102));
    voiceResultLabel->setText("语音: 正在监听...");
    voiceResultLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::successColor, 11, 500));
}

void MainWindow::onVoiceRecognitionStopped()
{
    isVoiceActive = false;
    btnVoiceControl->setText("语音控制");
    btnVoiceControl->setGlowColor(QColor(178, 102, 255));
    voiceResultLabel->setText("语音: 已停止");
    voiceResultLabel->setStyleSheet(AppStyle::labelStyle(AppStyle::textMutedColor, 11, 400));
}

void MainWindow::processVoiceCommand(const QString &command)
{
    QString cmd = command.toLower().trimmed();

    if (cmd.contains("连接")) {
        if (!isUsbConnected) connectToBot();
    } else if (cmd.contains("断开")) {
        if (isUsbConnected) disconnectFromBot();
    } else if (cmd.contains("摄像头") || cmd.contains("开启")) {
        if (isUsbConnected && !isCameraCapturing) startCameraCapture();
    } else if (cmd.contains("停止") || cmd.contains("关闭")) {
        if (isCameraCapturing) stopCameraCapture();
    } else if (cmd.contains("复位") || cmd.contains("归零")) {
        onResetClicked();
    } else if (cmd.contains("一号") || cmd.contains("头部")) {
        if (cmd.contains("左")) jointSliders[0]->setValue(jointSliders[0]->value() - 15);
        else if (cmd.contains("右")) jointSliders[0]->setValue(jointSliders[0]->value() + 15);
    } else if (cmd.contains("二号") || cmd.contains("左肩")) {
        if (cmd.contains("上")) jointSliders[1]->setValue(jointSliders[1]->value() + 15);
        else if (cmd.contains("下")) jointSliders[1]->setValue(jointSliders[1]->value() - 15);
    } else if (cmd.contains("三号") || cmd.contains("左手")) {
        if (cmd.contains("上")) jointSliders[2]->setValue(jointSliders[2]->value() + 15);
        else if (cmd.contains("下")) jointSliders[2]->setValue(jointSliders[2]->value() - 15);
    } else if (cmd.contains("四号") || cmd.contains("右肩")) {
        if (cmd.contains("上")) jointSliders[3]->setValue(jointSliders[3]->value() + 15);
        else if (cmd.contains("下")) jointSliders[3]->setValue(jointSliders[3]->value() - 15);
    } else if (cmd.contains("五号") || cmd.contains("右手")) {
        if (cmd.contains("上")) jointSliders[4]->setValue(jointSliders[4]->value() + 15);
        else if (cmd.contains("下")) jointSliders[4]->setValue(jointSliders[4]->value() - 15);
    } else if (cmd.contains("六号") || cmd.contains("底座")) {
        if (cmd.contains("左")) jointSliders[5]->setValue(jointSliders[5]->value() - 15);
        else if (cmd.contains("右")) jointSliders[5]->setValue(jointSliders[5]->value() + 15);
    }
}
