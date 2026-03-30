#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , videoPlayer(nullptr)
    , videoDisplayLabel(nullptr)
    , tcpSocket(nullptr)
    , isCapturing(false)
    , captureInterval(50)
    , botPort(6666)
{
    videoPlayer = new FFmpegVideoPlayer(this);
    setupUI();
    setupConnections();

    videoPlayer->setDisplayWidget(videoDisplayLabel);
    videoPlayer->setLooping(true);

    if (videoPlayer->loadVideo(":/res/happy.mp4")) {
        videoPlayer->play();
    }

    resize(1024, 768);
    setWindowTitle("ElectronBot Client - Screen Transmission & MP4 Player");
}

MainWindow::~MainWindow()
{
    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
        delete tcpSocket;
    }
    if (captureTimer) {
        captureTimer->stop();
        delete captureTimer;
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    videoDisplayLabel = new QLabel(this);
    videoDisplayLabel->setMinimumSize(640, 480);
    videoDisplayLabel->setStyleSheet("background-color: black;");
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(videoDisplayLabel);

    QHBoxLayout *controlLayout = new QHBoxLayout();

    btnOpen = new QPushButton("Open", this);
    btnPlay = new QPushButton("Play", this);
    btnPause = new QPushButton("Pause", this);
    btnStop = new QPushButton("Stop", this);
    btnConnect = new QPushButton("Connect to Bot", this);
    btnStartCapture = new QPushButton("Start Capture", this);
    btnStopCapture = new QPushButton("Stop Capture", this);

    controlLayout->addWidget(btnOpen);
    controlLayout->addWidget(btnPlay);
    controlLayout->addWidget(btnPause);
    controlLayout->addWidget(btnStop);
    controlLayout->addWidget(btnConnect);
    controlLayout->addWidget(btnStartCapture);
    controlLayout->addWidget(btnStopCapture);

    mainLayout->addLayout(controlLayout);

    QHBoxLayout *statusLayout = new QHBoxLayout();
    labelStatus = new QLabel("Disconnected", this);
    statusLayout->addWidget(labelStatus);
    statusLayout->addStretch();

    mainLayout->addLayout(statusLayout);

    setCentralWidget(centralWidget);

    btnStopCapture->setEnabled(false);
    btnPlay->setEnabled(false);
    btnPause->setEnabled(false);
    btnStop->setEnabled(false);

    tcpSocket = new QTcpSocket(this);
    captureTimer = new QTimer(this);
}

void MainWindow::setupConnections()
{
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(btnPlay, SIGNAL(clicked()), this, SLOT(playVideo()));
    connect(btnPause, SIGNAL(clicked()), this, SLOT(pauseVideo()));
    connect(btnStop, SIGNAL(clicked()), this, SLOT(stopVideo()));
    connect(btnConnect, SIGNAL(clicked()), this, SLOT(connectToBot()));
    connect(btnStartCapture, SIGNAL(clicked()), this, SLOT(startScreenCapture()));
    connect(btnStopCapture, SIGNAL(clicked()), this, SLOT(stopScreenCapture()));

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(tcpConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));

    connect(captureTimer, SIGNAL(timeout()), this, SLOT(sendScreenData()));
}

void MainWindow::tcpConnected()
{
    labelStatus->setText("Connected to Bot");
    btnConnect->setText("Disconnect");
}

void MainWindow::tcpDisconnected()
{
    labelStatus->setText("Disconnected");
    btnConnect->setText("Connect to Bot");
}

void MainWindow::tcpError(QAbstractSocket::SocketError)
{
    QMessageBox::warning(this, "Connection Error", tcpSocket->errorString());
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
            labelStatus->setText("File loaded: " + fileName);
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
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
        disconnectFromBot();
    } else {
        bool ok;
        QString ip = QInputDialog::getText(this, "Connect to ElectronBot",
                                           "Enter Bot IP Address:",
                                           QLineEdit::Normal, "192.168.1.100", &ok);
        if (ok && !ip.isEmpty()) {
            botAddress.setAddress(ip);
            tcpSocket->connectToHost(botAddress, botPort);
        }
    }
}

void MainWindow::disconnectFromBot()
{
    stopScreenCapture();
    tcpSocket->disconnectFromHost();
}

void MainWindow::startScreenCapture()
{
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "Not Connected", "Please connect to ElectronBot first.");
        return;
    }
    isCapturing = true;
    btnStartCapture->setEnabled(false);
    btnStopCapture->setEnabled(true);
    captureTimer->start(captureInterval);
    labelStatus->setText("Screen capture started");
}

void MainWindow::stopScreenCapture()
{
    isCapturing = false;
    captureTimer->stop();
    btnStartCapture->setEnabled(true);
    btnStopCapture->setEnabled(false);
    labelStatus->setText("Screen capture stopped");
}

void MainWindow::sendScreenData()
{
    if (!isCapturing || tcpSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QPixmap pixmap = screen->grabWindow(QDesktopWidget().winId());
    pixmap = pixmap.scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPG", 80);

    QDataStream stream(tcpSocket);
    stream << byteArray;

    labelStatus->setText("Sending: " + QString::number(byteArray.size()) + " bytes");
}
