#include "mainwindow.h"
#include <QFileInfo>
#include <QPainterPath>
#include <QtMath>

CustomTitleBar::CustomTitleBar(QWidget *parent)
    : QWidget(parent)
    , parentWindow(parent)
{
    setFixedHeight(56);
    setStyleSheet("background: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(24, 0, 16, 0);
    layout->setSpacing(12);

    QLabel *iconLabel = new QLabel("EB", this);
    iconLabel->setStyleSheet(R"(
        color: #E63946;
        font-size: 22px;
        font-family: 'Segoe UI', Arial;
        font-weight: 900;
    )");

    titleLabel = new QLabel("ELECTRONBOT", this);
    titleLabel->setStyleSheet(R"(
        color: #ffffff;
        font-family: 'Segoe UI', Arial;
        font-size: 18px;
        font-weight: 700;
    )");

    QLabel *subtitleLabel = new QLabel("v3.0", this);
    subtitleLabel->setStyleSheet(R"(
        color: #E63946;
        font-family: 'Segoe UI', Arial;
        font-size: 11px;
        font-weight: 600;
    )");

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(1);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    layout->addWidget(iconLabel);
    layout->addLayout(titleLayout);
    layout->addStretch();

    QString btnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            color: rgba(255,255,255,0.6);
            font-size: 20px;
            font-family: 'Segoe UI', Arial;
            padding: 10px 18px;
            border-radius: 6px;
        }
        QPushButton:hover {
            color: #fff;
            background: rgba(230,57,70,0.15);
        }
    )";

    btnMinimize = new QPushButton("-", this);
    btnMinimize->setStyleSheet(btnStyle);
    btnMinimize->setFixedSize(50, 40);

    btnMaximize = new QPushButton("+", this);
    btnMaximize->setStyleSheet(btnStyle);
    btnMaximize->setFixedSize(50, 40);

    btnClose = new QPushButton("x");
    QString closeStyle = btnStyle + R"(
        QPushButton:hover {
            color: #fff;
            background: #E63946;
        }
    )";
    btnClose->setStyleSheet(closeStyle);
    btnClose->setFixedSize(50, 40);

    layout->addWidget(btnMinimize);
    layout->addWidget(btnMaximize);
    layout->addWidget(btnClose);

    connect(btnMinimize, &QPushButton::clicked, this, &CustomTitleBar::onMinimizeClicked);
    connect(btnMaximize, &QPushButton::clicked, this, &CustomTitleBar::onMaximizeClicked);
    connect(btnClose, &QPushButton::clicked, this, &CustomTitleBar::onCloseClicked);
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
        btnMaximize->setText("+");
    } else {
        parentWindow->showMaximized();
        btnMaximize->setText("=");
    }
}

void CustomTitleBar::onCloseClicked()
{
    parentWindow->close();
}

GlowingButton::GlowingButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_glowColor(230, 57, 70)
    , isHovered(false)
    , animValue(0)
{
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(48);
    setStyleSheet("background: transparent; border: none;");

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, [this]() {
        animValue = (animValue + 1) % 360;
        update();
    });
}

void GlowingButton::enterEvent(QEvent *)
{
    isHovered = true;
    animationTimer->start(16);
}

void GlowingButton::leaveEvent(QEvent *)
{
    isHovered = false;
    animationTimer->stop();
    update();
}

void GlowingButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();
    int radius = 8;

    QColor glowColor = m_glowColor;

    if (isHovered) {
        int pulse = int(25.0 * qSin(qDegreesToRadians(double(animValue * 3))));
        glowColor = QColor(
            qMin(255, m_glowColor.red() + pulse),
            qMin(255, m_glowColor.green()),
            qMin(255, m_glowColor.blue())
        );

        QPen glowPen(glowColor, 2);
        painter.setPen(glowPen);
        painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), radius, radius);

        QLinearGradient gradient(0, 0, 0, rect.height());
        gradient.setColorAt(0, QColor(glowColor.red(), glowColor.green(), glowColor.blue(), 35));
        gradient.setColorAt(0.5, QColor(glowColor.red(), glowColor.green(), glowColor.blue(), 15));
        gradient.setColorAt(1, QColor(glowColor.red(), glowColor.green(), glowColor.blue(), 35));
        painter.setBrush(gradient);
        painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), radius, radius);
    }

    QPen borderPen(QColor(glowColor.red(), glowColor.green(), glowColor.blue(), isHovered ? 220 : 120), isHovered ? 2 : 1);
    painter.setPen(borderPen);
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), radius, radius);

    QFont font = this->font();
    font.setFamily("Segoe UI");
    font.setBold(true);
    font.setPointSize(11);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 3);
    painter.setFont(font);
    painter.setPen(isHovered ? glowColor : QColor(glowColor.red(), glowColor.green(), glowColor.blue(), 180));
    painter.drawText(rect, Qt::AlignCenter, text().toUpper());
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
    setFixedSize(28, 28);

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, [this]() {
        showOn = !showOn;
        animValue = (animValue + 10) % 360;
        update();
    });
}

void StatusIndicator::setStatus(bool connected)
{
    m_isConnected = connected;
    m_ledColor = connected ? QColor(76, 175, 80) : QColor(230, 57, 70);
    if (connected) {
        blinkTimer->start(80);
    } else {
        blinkTimer->stop();
        showOn = true;
        animValue = 0;
    }
    update();
}

void StatusIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();
    QPointF center(rect.width() / 2.0, rect.height() / 2.0);

    QColor outerColor = m_ledColor;
    if (!showOn) {
        outerColor = QColor(m_ledColor.red(), m_ledColor.green(), m_ledColor.blue(), 50);
    }

    QRadialGradient gradient(center, rect.width() / 2.0);
    gradient.setColorAt(0, QColor(255, 255, 255, 200));
    gradient.setColorAt(0.3, outerColor);
    gradient.setColorAt(1, QColor(outerColor.red(), outerColor.green(), outerColor.blue(), 80));

    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect.adjusted(3, 3, -3, -3));

    QPen ringPen(outerColor, 2);
    painter.setPen(ringPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect.adjusted(4, 4, -4, -4));
}

void StatusIndicator::setLedColor(const QColor &color)
{
    m_ledColor = color;
    update();
}

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
    , m_color(230, 57, 70)
    , m_rotation(0)
{
    setFixedSize(64, 64);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, [this]() {
        m_rotation = (m_rotation + 30) % 360;
        update();
    });
    m_timer->start(80);
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

    QRect rect = this->rect();
    QPointF center(rect.width() / 2.0, rect.height() / 2.0);
    double radius = rect.width() / 2.0 - 8;

    painter.setPen(Qt::NoPen);
    for (int i = 0; i < 10; i++) {
        double angle = qDegreesToRadians(double(m_rotation + i * 36));
        double alpha = 255.0 * (i + 1) / 10.0;
        QColor dotColor = QColor(m_color.red(), m_color.green(), m_color.blue(), alpha);
        painter.setBrush(QBrush(dotColor));
        double dotX = center.x() + radius * qCos(angle) - 5;
        double dotY = center.y() + radius * qSin(angle) - 5;
        painter.drawEllipse(QPointF(dotX + 5, dotY + 5), 5, 5);
    }
}

WaitingDialog::WaitingDialog(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent)
    , animValue(0)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setFixedSize(420, 240);
    setStyleSheet("background: transparent;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *container = new QWidget(this);
    container->setObjectName("container");
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(30, 32, 30, 28);
    containerLayout->setSpacing(18);
    containerLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        color: #E63946;
        font-family: 'Segoe UI', Arial;
        font-size: 18px;
        font-weight: 700;
    )");

    loadingIndicator = new LoadingIndicator(this);

    messageLabel = new QLabel(message, this);
    messageLabel->setObjectName("messageLabel");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet(R"(
        color: #ffffff;
        font-family: 'Segoe UI', Arial;
        font-size: 14px;
    )");

    statusLabel = new QLabel("Please wait...", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(R"(
        color: rgba(255,255,255,0.5);
        font-family: 'Segoe UI', Arial;
        font-size: 12px;
    )");

    containerLayout->addWidget(titleLabel);
    containerLayout->addWidget(loadingIndicator, 0, Qt::AlignCenter);
    containerLayout->addWidget(messageLabel);
    containerLayout->addWidget(statusLabel, 0, Qt::AlignCenter);

    mainLayout->addWidget(container);

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, [this, container]() {
        animValue = (animValue + 2) % 360;

        int pulseR = 230 + (animValue % 25);
        int pulseG = 57 + (animValue % 15);
        QString color = QString("rgb(%1,%2,%3)").arg(pulseR).arg(pulseG).arg(70);
        container->setStyleSheet(QString(R"(
            #container {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #16161a,
                    stop:1 #111114);
                border: 2px solid %1;
                border-radius: 14px;
            }
            #titleLabel {
                color: #E63946;
                font-size: 18px;
            }
            #messageLabel {
                color: #ffffff;
                font-size: 14px;
            }
            #statusLabel {
                color: rgba(255,255,255,0.5);
                font-size: 12px;
            }
        )").arg(color));
    });
    animationTimer->start(30);

    scanLineTimer = new QTimer(this);
    connect(scanLineTimer, &QTimer::timeout, [this]() {
        update();
    });
    scanLineTimer->start(50);

    setStyleSheet(R"(
        QDialog {
            background: transparent;
        }
    )");
}

void WaitingDialog::setMessage(const QString &message)
{
    messageLabel->setText(message);
}

void WaitingDialog::setSuccess(const QString &message)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(R"(
        color: #4CAF50;
        font-family: 'Segoe UI', Arial;
        font-size: 13px;
        font-weight: 600;
    )");
    animationTimer->stop();
    scanLineTimer->stop();
    loadingIndicator->setColor(QColor(76, 175, 80));
}

void WaitingDialog::setFailed(const QString &message)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(R"(
        color: #E63946;
        font-family: 'Segoe UI', Arial;
        font-size: 13px;
        font-weight: 600;
    )");
    animationTimer->stop();
    scanLineTimer->stop();
    loadingIndicator->setColor(QColor(230, 57, 70));
}

void WaitingDialog::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int scanY = (animValue * height()) / 360;
    QLinearGradient scanGradient(0, scanY - 25, 0, scanY + 25);
    scanGradient.setColorAt(0, QColor(230, 57, 70, 0));
    scanGradient.setColorAt(0.5, QColor(230, 57, 70, 45));
    scanGradient.setColorAt(1, QColor(230, 57, 70, 0));

    painter.fillRect(rect(), QColor(0, 0, 0, 0));
    painter.fillRect(0, scanY - 25, width(), 50, scanGradient);
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
    setFixedSize(450, 260);
    setupUI(title, message, icon);
}

void CustomMessageBox::setupUI(const QString &title, const QString &message, IconType icon)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *container = new QWidget(this);
    container->setObjectName("msgContainer");
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(28, 26, 28, 24);
    containerLayout->setSpacing(18);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(this);
    QString iconText, iconColor;
    switch (icon) {
        case Information: iconText = "i"; iconColor = "#E63946"; break;
        case Warning: iconText = "!"; iconColor = "#FF9800"; break;
        case Error: iconText = "X"; iconColor = "#E63946"; break;
        case Success: iconText = "OK"; iconColor = "#4CAF50"; break;
    }
    iconLabel->setText(iconText);
    iconLabel->setStyleSheet(QString(R"(
        color: %1;
        font-size: 28px;
        font-family: 'Segoe UI', Arial;
        font-weight: 900;
    )").arg(iconColor));

    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(R"(
        color: #ffffff;
        font-family: 'Segoe UI', Arial;
        font-size: 17px;
        font-weight: 700;
    )");

    titleLayout->addWidget(iconLabel);
    titleLayout->addSpacing(18);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    QLabel *msgLabel = new QLabel(message, this);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet(R"(
        color: rgba(255,255,255,0.85);
        font-family: 'Segoe UI', Arial;
        font-size: 14px;
    )");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *okBtn = createButton("OK", QColor(230, 57, 70));
    btnLayout->addWidget(okBtn);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

    containerLayout->addLayout(titleLayout);
    containerLayout->addWidget(msgLabel);
    containerLayout->addStretch();
    containerLayout->addLayout(btnLayout);

    mainLayout->addWidget(container);

    setStyleSheet(R"(
        QDialog {
            background: transparent;
        }
        #msgContainer {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #16161a,
                stop:1 #111114);
            border: 2px solid rgba(230, 57, 70, 0.5);
            border-radius: 14px;
        }
    )");
}

QPushButton *CustomMessageBox::createButton(const QString &text, const QColor &color)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setFixedSize(110, 44);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(QString(R"(
        QPushButton {
            background: rgba(%1, %2, %3, 0.15);
            border: 2px solid rgba(%1, %2, %3, 0.7);
            border-radius: 8px;
            color: rgb(%1, %2, %3);
            font-family: 'Segoe UI', Arial;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton:hover {
            background: rgba(%1, %2, %3, 0.3);
            border-color: rgb(%1, %2, %3);
        }
    )").arg(color.red()).arg(color.green()).arg(color.blue()));
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , videoPlayer(nullptr)
    , videoDisplayLabel(nullptr)
    , isCapturing(false)
    , captureInterval(50)
    , robot(nullptr)
    , isUsbConnected(false)
    , waitingDialog(nullptr)
    , isConnecting(false)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    videoPlayer = new FFmpegVideoPlayer(this);
    setupUI();
    setupConnections();

    videoPlayer->setDisplayWidget(videoDisplayLabel);
    videoPlayer->setLooping(true);

    if (videoPlayer->loadVideo(":/res/happy.mp4")) {
        videoPlayer->play();
    }

    resize(1320, 860);
}

MainWindow::~MainWindow()
{
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
    if (captureTimer) {
        captureTimer->stop();
        delete captureTimer;
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
    setCentralWidget(mainContainer);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    titleBar = new CustomTitleBar(this);
    mainLayout->addWidget(titleBar);

    centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    mainLayout->addWidget(centralWidget);

    QVBoxLayout *centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(28, 22, 28, 22);
    centerLayout->setSpacing(18);

    QWidget *videoContainer = new QWidget(this);
    videoContainer->setObjectName("videoContainer");
    QVBoxLayout *videoLayout = new QVBoxLayout(videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(10);

    QWidget *videoHeader = new QWidget(this);
    videoHeader->setObjectName("videoHeader");
    QHBoxLayout *videoHeaderLayout = new QHBoxLayout(videoHeader);
    videoHeaderLayout->setContentsMargins(16, 12, 16, 12);

    QLabel *videoIcon = new QLabel("[VIDEO]", this);
    videoIcon->setStyleSheet(R"(
        color: #E63946;
        font-size: 14px;
        font-family: 'Segoe UI', Arial;
        font-weight: 800;
    )");

    QLabel *videoTitle = new QLabel("LIVE PREVIEW", this);
    videoTitle->setStyleSheet(R"(
        color: #ffffff;
        font-family: 'Segoe UI', Arial;
        font-size: 14px;
        font-weight: 700;
    )");

    QLabel *videoStatus = new QLabel("ONLINE", this);
    videoStatus->setStyleSheet(R"(
        color: #4CAF50;
        font-family: 'Segoe UI', Arial;
        font-size: 12px;
        font-weight: 700;
        padding: 4px 12px;
        background: rgba(76,175,80,0.15);
        border-radius: 4px;
    )");

    videoHeaderLayout->addWidget(videoIcon);
    videoHeaderLayout->addSpacing(12);
    videoHeaderLayout->addWidget(videoTitle);
    videoHeaderLayout->addStretch();
    videoHeaderLayout->addWidget(videoStatus);

    videoDisplayLabel = new QLabel(this);
    videoDisplayLabel->setMinimumSize(960, 560);
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    videoDisplayLabel->setStyleSheet(R"(
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
            stop:0 #0d0d0d,
            stop:0.5 #101010,
            stop:1 #141414);
        border: 2px solid rgba(230, 57, 70, 0.35);
        border-radius: 10px;
    )");

    QWidget *videoFooter = new QWidget(this);
    videoFooter->setObjectName("videoFooter");
    QHBoxLayout *videoFooterLayout = new QHBoxLayout(videoFooter);
    videoFooterLayout->setContentsMargins(16, 8, 16, 8);

    QLabel *resolution = new QLabel("1920 x 1080", this);
    resolution->setStyleSheet(R"(
        color: rgba(255,255,255,0.45);
        font-family: 'Segoe UI', Arial;
        font-size: 11px;
        font-weight: 500;
    )");

    QLabel *fps = new QLabel("30 FPS", this);
    fps->setStyleSheet(R"(
        color: rgba(255,255,255,0.45);
        font-family: 'Segoe UI', Arial;
        font-size: 11px;
        font-weight: 500;
    )");

    videoFooterLayout->addWidget(resolution);
    videoFooterLayout->addSpacing(20);
    videoFooterLayout->addWidget(fps);
    videoFooterLayout->addStretch();

    videoLayout->addWidget(videoHeader);
    videoLayout->addWidget(videoDisplayLabel);
    videoLayout->addWidget(videoFooter);

    QWidget *controlContainer = new QWidget(this);
    controlContainer->setObjectName("controlContainer");
    QVBoxLayout *controlContainerLayout = new QVBoxLayout(controlContainer);
    controlContainerLayout->setContentsMargins(0, 0, 0, 0);
    controlContainerLayout->setSpacing(12);

    QWidget *controlHeader = new QWidget(this);
    controlHeader->setObjectName("controlHeader");
    QHBoxLayout *controlHeaderLayout = new QHBoxLayout(controlHeader);
    controlHeaderLayout->setContentsMargins(16, 12, 16, 12);

    QLabel *controlIcon = new QLabel("[CTRL]", this);
    controlIcon->setStyleSheet(R"(
        color: #E63946;
        font-size: 14px;
        font-family: 'Segoe UI', Arial;
        font-weight: 800;
    )");

    QLabel *controlTitle = new QLabel("CONTROL PANEL", this);
    controlTitle->setStyleSheet(R"(
        color: #ffffff;
        font-family: 'Segoe UI', Arial;
        font-size: 14px;
        font-weight: 700;
    )");

    controlHeaderLayout->addWidget(controlIcon);
    controlHeaderLayout->addSpacing(12);
    controlHeaderLayout->addWidget(controlTitle);
    controlHeaderLayout->addStretch();

    QWidget *controlPanel = new QWidget(this);
    controlPanel->setObjectName("controlPanel");
    QGridLayout *controlGrid = new QGridLayout(controlPanel);
    controlGrid->setContentsMargins(18, 18, 18, 18);
    controlGrid->setSpacing(14);

    btnOpen = new GlowingButton("OPEN FILE", this);
    btnOpen->setGlowColor(QColor(100, 180, 255));
    btnOpen->setFixedWidth(150);

    btnPlay = new GlowingButton("PLAY", this);
    btnPlay->setGlowColor(QColor(76, 175, 80));

    btnPause = new GlowingButton("PAUSE", this);
    btnPause->setGlowColor(QColor(255, 183, 77));

    btnStop = new GlowingButton("STOP", this);
    btnStop->setGlowColor(QColor(239, 83, 80));

    btnConnect = new GlowingButton("CONNECT", this);
    btnConnect->setGlowColor(QColor(230, 57, 70));
    btnConnect->setFixedWidth(170);

    btnStartCapture = new GlowingButton("CAPTURE", this);
    btnStartCapture->setGlowColor(QColor(171, 71, 188));

    btnStopCapture = new GlowingButton("STOP CAP", this);
    btnStopCapture->setGlowColor(QColor(239, 108, 0));

    controlGrid->addWidget(btnOpen, 0, 0);
    controlGrid->addWidget(btnPlay, 0, 1);
    controlGrid->addWidget(btnPause, 0, 2);
    controlGrid->addWidget(btnStop, 0, 3);
    controlGrid->addWidget(btnConnect, 0, 4);
    controlGrid->addWidget(btnStartCapture, 0, 5);
    controlGrid->addWidget(btnStopCapture, 0, 6);

    QWidget *statusPanel = new QWidget(this);
    statusPanel->setObjectName("statusPanel");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusPanel);
    statusLayout->setContentsMargins(18, 14, 18, 14);
    statusLayout->setSpacing(18);

    statusLed = new StatusIndicator(this);

    QLabel *statusLabel = new QLabel("STATUS:", this);
    statusLabel->setStyleSheet(R"(
        color: rgba(255,255,255,0.55);
        font-family: 'Segoe UI', Arial;
        font-size: 13px;
        font-weight: 600;
    )");

    labelStatus = new QLabel("SYSTEM READY", this);
    labelStatus->setStyleSheet(R"(
        color: #E63946;
        font-family: 'Segoe UI', Arial;
        font-size: 14px;
        font-weight: 700;
    )");

    QLabel *versionLabel = new QLabel("v3.0.0", this);
    versionLabel->setStyleSheet(R"(
        color: rgba(255,255,255,0.25);
        font-family: 'Segoe UI', Arial;
        font-size: 11px;
    )");

    statusLayout->addWidget(statusLed);
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(labelStatus);
    statusLayout->addStretch();
    statusLayout->addWidget(versionLabel);

    controlContainerLayout->addWidget(controlHeader);
    controlContainerLayout->addWidget(controlPanel);

    centerLayout->addWidget(videoContainer);
    centerLayout->addSpacing(18);
    centerLayout->addWidget(controlContainer);
    centerLayout->addSpacing(12);
    centerLayout->addWidget(statusPanel);

    applyStyleSheet();

    btnStopCapture->setEnabled(false);
    btnPlay->setEnabled(false);
    btnPause->setEnabled(false);
    btnStop->setEnabled(false);

    robot = new ElectronLowLevel(this);
    captureTimer = new QTimer(this);
}

void MainWindow::setupConnections()
{
    connect(btnOpen, &GlowingButton::clicked, this, &MainWindow::openFile);
    connect(btnPlay, &GlowingButton::clicked, this, &MainWindow::playVideo);
    connect(btnPause, &GlowingButton::clicked, this, &MainWindow::pauseVideo);
    connect(btnStop, &GlowingButton::clicked, this, &MainWindow::stopVideo);
    connect(btnConnect, &GlowingButton::clicked, this, &MainWindow::connectToBot);
    connect(btnStartCapture, &GlowingButton::clicked, this, &MainWindow::startScreenCapture);
    connect(btnStopCapture, &GlowingButton::clicked, this, &MainWindow::stopScreenCapture);

    connect(captureTimer, &QTimer::timeout, this, &MainWindow::sendScreenData);
    connect(videoPlayer, &FFmpegVideoPlayer::frameReady, this, &MainWindow::onFrameReady);
    connect(robot, &ElectronLowLevel::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);
    connect(robot, &ElectronLowLevel::connectFinished, this, &MainWindow::onConnectFinished);
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(R"(
        QMainWindow {
            background: transparent;
        }
        #mainContainer {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #121214,
                stop:0.3 #151518,
                stop:0.7 #131315,
                stop:1 #0e0e10);
            border: 1px solid rgba(230, 57, 70, 0.25);
            border-radius: 0px;
        }
        #videoContainer {
            background: transparent;
        }
        #videoHeader {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(230, 57, 70, 0.18),
                stop:1 rgba(230, 57, 70, 0.06));
            border: 1px solid rgba(230, 57, 70, 0.35);
            border-radius: 10px 10px 0 0;
        }
        #videoFooter {
            background: rgba(0, 0, 0, 0.35);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 0 0 10px 10px;
        }
        #controlContainer {
            background: transparent;
        }
        #controlHeader {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(230, 57, 70, 0.18),
                stop:1 rgba(230, 57, 70, 0.06));
            border: 1px solid rgba(230, 57, 70, 0.35);
            border-radius: 10px 10px 0 0;
        }
        #controlPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(22, 22, 26, 0.95),
                stop:1 rgba(16, 16, 19, 0.95));
            border: 1px solid rgba(230, 57, 70, 0.2);
            border-top: none;
            border-radius: 0 0 10px 10px;
        }
        #statusPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(28, 28, 33, 0.95),
                stop:1 rgba(19, 19, 23, 0.95));
            border: 1px solid rgba(230, 57, 70, 0.25);
            border-radius: 10px;
        }
        QLabel {
            color: #e8e8e8;
        }
    )");
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

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Video File", "", "Video Files (*.mp4 *.avi *.mkv *.mov);;All Files (*)");
    if (!fileName.isEmpty()) {
        if (videoPlayer->loadVideo(fileName)) {
            videoPlayer->setLooping(false);
            videoPlayer->play();
            btnPlay->setEnabled(false);
            btnPause->setEnabled(true);
            btnStop->setEnabled(true);
            labelStatus->setText("FILE LOADED: " + QFileInfo(fileName).fileName());
        }
    }
}

void MainWindow::playVideo()
{
    videoPlayer->play();
    btnPlay->setEnabled(false);
    btnPause->setEnabled(true);
    btnStop->setEnabled(true);
}

void MainWindow::pauseVideo()
{
    videoPlayer->pause();
    btnPlay->setEnabled(true);
    btnPause->setEnabled(false);
}

void MainWindow::stopVideo()
{
    videoPlayer->stop();
    btnPlay->setEnabled(true);
    btnPause->setEnabled(false);
    btnStop->setEnabled(false);
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
        showErrorDialog("CONNECTION FAILED",
            "Unable to connect to ElectronBot.\n"
            "Please check if the device is connected via USB.");
        return;
    }

    isConnecting = true;
    btnConnect->setEnabled(false);

    showWaitingDialog("CONNECTING", "Scanning for ElectronBot...");
}

void MainWindow::onWaitingDialogClosed()
{
    isConnecting = false;
    btnConnect->setEnabled(true);
}

void MainWindow::disconnectFromBot()
{
    showWaitingDialog("DISCONNECTING", "Closing USB connection...");
    btnConnect->setEnabled(false);

    QTimer::singleShot(100, this, [this]() {
        robot->Disconnect();
        hideWaitingDialog();
    });
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    isUsbConnected = connected;

    if (connected) {
        isConnecting = false;

        hideWaitingDialog();

        statusLed->setStatus(true);
        labelStatus->setStyleSheet(R"(
            color: #4CAF50;
            font-family: 'Segoe UI', Arial;
            font-size: 14px;
            font-weight: 700;
        )");
        labelStatus->setText("CONNECTED TO ELECTRONBOT");
        btnConnect->setText("DISCONNECT");
        btnConnect->setEnabled(true);
        btnStartCapture->setEnabled(true);

        if (videoPlayer->loadVideo(":/res/happy.mp4")) {
            videoPlayer->setLooping(true);
            videoPlayer->play();
        }
    } else {
        if (isConnecting) {
            return;
        }

        stopScreenCapture();
        statusLed->setStatus(false);
        labelStatus->setStyleSheet(R"(
            color: #E63946;
            font-family: 'Segoe UI', Arial;
            font-size: 14px;
            font-weight: 700;
        )");
        labelStatus->setText("SYSTEM READY");
        btnConnect->setText("CONNECT");
        btnConnect->setEnabled(true);
    }
}

void MainWindow::onConnectFinished(bool success)
{
    if (!success && !isUsbConnected) {
        hideWaitingDialog();
        showErrorDialog("CONNECTION FAILED",
            "Unable to connect to ElectronBot.\n"
            "Please check if the device is connected via USB.");
        btnConnect->setEnabled(true);
        isConnecting = false;
    }
}

void MainWindow::startScreenCapture()
{
    if (!isUsbConnected) {
        showErrorDialog("NOT CONNECTED", "Please connect to ElectronBot first.");
        return;
    }
    isCapturing = true;
    btnStartCapture->setEnabled(false);
    btnStopCapture->setEnabled(true);
    videoPlayer->pause();
    captureTimer->start(captureInterval);
    labelStatus->setStyleSheet(R"(
        color: #AB47BC;
        font-family: 'Segoe UI', Arial;
        font-size: 14px;
        font-weight: 700;
    )");
    labelStatus->setText("SCREEN CAPTURE ACTIVE");
}

void MainWindow::stopScreenCapture()
{
    if (!isCapturing) return;

    isCapturing = false;
    captureTimer->stop();
    btnStartCapture->setEnabled(true);
    btnStopCapture->setEnabled(false);
    videoPlayer->play();

    if (isUsbConnected) {
        labelStatus->setStyleSheet(R"(
            color: #4CAF50;
            font-family: 'Segoe UI', Arial;
            font-size: 14px;
            font-weight: 700;
        )");
        labelStatus->setText("CONNECTED TO ELECTRONBOT");
    }
}

void MainWindow::sendScreenData()
{
    if (!isCapturing || !isUsbConnected) {
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QPixmap pixmap = screen->grabWindow(QDesktopWidget().winId());
    QImage image = pixmap.toImage();

    robot->SetImageSrc(image);
}

void MainWindow::onFrameReady(const QImage &image)
{
    if (!isUsbConnected) {
        return;
    }

    robot->SetImageSrc(image);
}
