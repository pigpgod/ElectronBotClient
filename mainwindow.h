#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTcpSocket>
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
#include <QHostAddress>
#include <QInputDialog>
#include <QLineEdit>
#include <QTime>
#include <QDataStream>

#include "ffmpegvideoplayer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    void tcpConnected();
    void tcpDisconnected();
    void tcpError(QAbstractSocket::SocketError);

private:
    void setupUI();
    void setupConnections();

    FFmpegVideoPlayer *videoPlayer;
    QLabel *videoDisplayLabel;

    QPushButton *btnOpen;
    QPushButton *btnPlay;
    QPushButton *btnPause;
    QPushButton *btnStop;
    QPushButton *btnConnect;
    QPushButton *btnStartCapture;
    QPushButton *btnStopCapture;

    QLabel *labelStatus;

    QTcpSocket *tcpSocket;
    QHostAddress botAddress;
    quint16 botPort;

    QTimer *captureTimer;
    bool isCapturing;
    int captureInterval;
};
#endif
