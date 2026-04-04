/**
 * @file ffmpegvideoplayer.h
 * @brief FFmpeg 视频播放器类声明
 * 
 * 基于 FFmpeg 库的视频播放器：
 * - 支持多种视频格式（MP4、AVI、MKV、MOV 等）
 * - 视频解码和图像转换
 * - 播放控制（播放、暂停、停止、跳转）
 * - 循环播放支持
 * - 帧信号输出（用于传输到机器人屏幕）
 */

#ifndef FFMPEGVIDEOPLAYER_H
#define FFMPEGVIDEOPLAYER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
}

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

class FFmpegVideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit FFmpegVideoPlayer(QWidget *parent = 0);
    ~FFmpegVideoPlayer();

    bool loadVideo(const QString &filePath);
    void play();
    void pause();
    void stop();
    void seek(qint64 position);
    qint64 duration() const;
    qint64 position() const;
    void setVolume(int volume);

signals:
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void stateChanged(int state);
    void playbackFinished();
    void frameReady(const QImage &image);

public slots:
    void setDisplayWidget(QLabel *label);
    void setLooping(bool loop);

private slots:
    void decodeFrame();

private:
    enum PlayerState { StoppedState, PlayingState, PausedState };

    bool initDecoder();
    void closeDecoder();
    QImage decodeNextFrame();
    bool convertFrame(AVFrame *frame, QImage &image);

    QLabel *m_displayLabel;
    QTimer *m_timer;

    AVFormatContext *m_formatContext;
    AVCodecContext *m_videoCodecContext;
    AVCodecContext *m_audioCodecContext;
    AVFrame *m_frame;
    AVFrame *m_rgbFrame;
    SwsContext *m_swsContext;

    int m_videoStreamIndex;
    int m_audioStreamIndex;
    AVPacket *m_packet;

    PlayerState m_state;
    qint64 m_duration;
    qint64 m_currentPosition;
    qint64 m_startTime;
    int m_frameRate;

    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_quit;
    bool m_loop;
    QString m_tempFilePath;

    uint8_t *m_buffer;
};

#endif
