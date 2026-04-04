/**
 * @file ffmpegvideoplayer.cpp
 * @brief FFmpeg 视频播放器类实现
 * 
 * 实现视频播放功能：
 * - 视频文件加载和解析
 * - FFmpeg 解码器初始化
 * - 帧解码和图像格式转换
 * - 定时器驱动的帧解码
 * - Qt 资源文件支持（临时文件方式）
 */

#include "ffmpegvideoplayer.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

extern "C" {
#include <libavutil/opt.h>
}

FFmpegVideoPlayer::FFmpegVideoPlayer(QWidget *parent)
    : QWidget(parent)
    , m_displayLabel(0)
    , m_timer(0)
    , m_formatContext(0)
    , m_videoCodecContext(0)
    , m_audioCodecContext(0)
    , m_frame(0)
    , m_rgbFrame(0)
    , m_swsContext(0)
    , m_videoStreamIndex(-1)
    , m_audioStreamIndex(-1)
    , m_packet(0)
    , m_state(StoppedState)
    , m_duration(0)
    , m_currentPosition(0)
    , m_startTime(0)
    , m_frameRate(25)
    , m_quit(false)
    , m_loop(false)
    , m_buffer(0)
    , m_tempFilePath()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(decodeFrame()));
}

FFmpegVideoPlayer::~FFmpegVideoPlayer()
{
    stop();
    closeDecoder();
    if (!m_tempFilePath.isEmpty()) {
        QFile::remove(m_tempFilePath);
    }
}

bool FFmpegVideoPlayer::loadVideo(const QString &filePath)
{
    stop();
    closeDecoder();

    if (!m_tempFilePath.isEmpty()) {
        QFile::remove(m_tempFilePath);
        m_tempFilePath.clear();
    }

    QString actualPath = filePath;
    if (filePath.startsWith(":")) {
        QFile resourceFile(filePath);
        if (!resourceFile.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open resource file:" << filePath;
            return false;
        }

        QByteArray resourceData = resourceFile.readAll();
        resourceFile.close();

        QFileInfo fileInfo(filePath);
        QString tempDir = QCoreApplication::applicationDirPath() + "/temp";
        QDir().mkpath(tempDir);
        m_tempFilePath = tempDir + "/" + fileInfo.fileName();
        QFile tempFile(m_tempFilePath);
        if (!tempFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot create temp file:" << m_tempFilePath;
            return false;
        }
        tempFile.write(resourceData);
        tempFile.close();

        actualPath = m_tempFilePath;
    }

    QMutexLocker locker(&m_mutex);

    m_formatContext = avformat_alloc_context();
    if (!m_formatContext) {
        qDebug() << "Failed to allocate format context";
        return false;
    }

    QByteArray pathBytes = actualPath.toUtf8();
    if (avformat_open_input(&m_formatContext, pathBytes.constData(), 0, 0) != 0) {
        qDebug() << "Failed to open input file:" << actualPath;
        return false;
    }

    if (avformat_find_stream_info(m_formatContext, 0) < 0) {
        qDebug() << "Failed to find stream info";
        return false;
    }

    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;

    for (unsigned int i = 0; i < m_formatContext->nb_streams; i++) {
        AVStream *stream = m_formatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
        }
    }

    if (m_videoStreamIndex == -1) {
        qDebug() << "No video stream found";
        return false;
    }

    AVStream *videoStream = m_formatContext->streams[m_videoStreamIndex];
    const AVCodec *videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!videoCodec) {
        qDebug() << "Video codec not found";
        return false;
    }

    m_videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!m_videoCodecContext) {
        qDebug() << "Failed to allocate video codec context";
        return false;
    }

    if (avcodec_parameters_to_context(m_videoCodecContext, videoStream->codecpar) < 0) {
        qDebug() << "Failed to copy codec parameters";
        return false;
    }

    if (avcodec_open2(m_videoCodecContext, videoCodec, 0) < 0) {
        qDebug() << "Failed to open video codec";
        return false;
    }

    if (m_audioStreamIndex != -1) {
        AVStream *audioStream = m_formatContext->streams[m_audioStreamIndex];
        const AVCodec *audioCodec = avcodec_find_decoder(audioStream->codecpar->codec_id);
        if (audioCodec) {
            m_audioCodecContext = avcodec_alloc_context3(audioCodec);
            if (m_audioCodecContext) {
                if (avcodec_parameters_to_context(m_audioCodecContext, audioStream->codecpar) >= 0) {
                    avcodec_open2(m_audioCodecContext, audioCodec, 0);
                }
            }
        }
    }

    m_frame = av_frame_alloc();
    m_rgbFrame = av_frame_alloc();
    m_packet = av_packet_alloc();

    if (!m_frame || !m_rgbFrame || !m_packet) {
        qDebug() << "Failed to allocate frame or packet";
        return false;
    }

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                             m_videoCodecContext->width,
                                             m_videoCodecContext->height,
                                             1);
    m_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize,
                         m_buffer, AV_PIX_FMT_RGB24,
                         m_videoCodecContext->width,
                         m_videoCodecContext->height, 1);

    m_swsContext = sws_getContext(m_videoCodecContext->width,
                                  m_videoCodecContext->height,
                                  m_videoCodecContext->pix_fmt,
                                  m_videoCodecContext->width,
                                  m_videoCodecContext->height,
                                  AV_PIX_FMT_RGB24,
                                  SWS_BILINEAR,
                                  0, 0, 0);

    m_duration = m_formatContext->duration / 1000;
    emit durationChanged(m_duration);

    AVStream *stream = m_formatContext->streams[m_videoStreamIndex];
    if (stream->avg_frame_rate.num > 0 && stream->avg_frame_rate.den > 0) {
        m_frameRate = stream->avg_frame_rate.num / stream->avg_frame_rate.den;
        if (m_frameRate <= 0 || m_frameRate > 120) {
            m_frameRate = 25;
        }
    } else {
        m_frameRate = 25;
    }

    int interval = 1000 / m_frameRate;
    m_timer->setInterval(interval);

    return true;
}

void FFmpegVideoPlayer::play()
{
    if (m_state == PlayingState) return;

    m_state = PlayingState;
    m_quit = false;

    if (m_currentPosition > 0) {
        m_startTime = av_gettime_relative() - (m_currentPosition * 1000);
    } else {
        m_startTime = av_gettime_relative();
    }

    if (!m_timer->isActive()) {
        m_timer->start();
    }
    emit stateChanged(m_state);
}

void FFmpegVideoPlayer::pause()
{
    if (m_state != PlayingState) return;

    m_state = PausedState;
    m_timer->stop();
    emit stateChanged(m_state);
}

void FFmpegVideoPlayer::stop()
{
    m_timer->stop();

    QMutexLocker locker(&m_mutex);
    m_state = StoppedState;
    m_currentPosition = 0;
    m_quit = true;

    if (m_formatContext) {
        av_seek_frame(m_formatContext, m_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
    }

    emit stateChanged(m_state);
}

void FFmpegVideoPlayer::seek(qint64 position)
{
    if (!m_formatContext) return;

    AVStream *stream = m_formatContext->streams[m_videoStreamIndex];
    qint64 timestamp = position * 1000;
    av_seek_frame(m_formatContext, m_videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);

    m_currentPosition = position;
    m_startTime = av_gettime_relative() - (position * 1000);

    avcodec_flush_buffers(m_videoCodecContext);
    emit positionChanged(m_currentPosition);
}

qint64 FFmpegVideoPlayer::duration() const
{
    return m_duration;
}

qint64 FFmpegVideoPlayer::position() const
{
    return m_currentPosition;
}

void FFmpegVideoPlayer::setVolume(int volume)
{
    Q_UNUSED(volume);
}

void FFmpegVideoPlayer::setDisplayWidget(QLabel *label)
{
    m_displayLabel = label;
}

void FFmpegVideoPlayer::decodeFrame()
{
    if (m_state != PlayingState || !m_displayLabel) return;

    qint64 currentTime = (av_gettime_relative() - m_startTime) / 1000;
    emit positionChanged(currentTime);

    while (true) {
        int ret = avcodec_receive_frame(m_videoCodecContext, m_frame);
        if (ret == 0) {
            QImage image;
            if (convertFrame(m_frame, image)) {
                QPixmap pixmap = QPixmap::fromImage(image);
                m_displayLabel->setPixmap(pixmap.scaled(m_displayLabel->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation));
                emit frameReady(image);
            }
            break;
        } else if (ret == AVERROR(EAGAIN)) {
            ret = av_read_frame(m_formatContext, m_packet);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    if (m_loop) {
                        av_seek_frame(m_formatContext, m_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
                        avcodec_flush_buffers(m_videoCodecContext);
                        m_startTime = av_gettime_relative();
                    } else {
                        emit playbackFinished();
                        m_state = StoppedState;
                    }
                }
                return;
            }

            if (m_packet->stream_index == m_videoStreamIndex) {
                avcodec_send_packet(m_videoCodecContext, m_packet);
            }
            av_packet_unref(m_packet);
        } else {
            break;
        }
    }
}

void FFmpegVideoPlayer::setLooping(bool loop)
{
    m_loop = loop;
}

bool FFmpegVideoPlayer::convertFrame(AVFrame *frame, QImage &image)
{
    if (!m_swsContext) return false;

    sws_scale(m_swsContext,
              frame->data, frame->linesize, 0,
              m_videoCodecContext->height,
              m_rgbFrame->data, m_rgbFrame->linesize);

    image = QImage(m_buffer,
                   m_videoCodecContext->width,
                   m_videoCodecContext->height,
                   m_rgbFrame->linesize[0],
                   QImage::Format_RGB888);

    return !image.isNull();
}

void FFmpegVideoPlayer::closeDecoder()
{
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = 0;
    }

    if (m_buffer) {
        av_free(m_buffer);
        m_buffer = 0;
    }

    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = 0;
    }

    if (m_rgbFrame) {
        av_frame_free(&m_rgbFrame);
        m_rgbFrame = 0;
    }

    if (m_videoCodecContext) {
        avcodec_close(m_videoCodecContext);
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = 0;
    }

    if (m_audioCodecContext) {
        avcodec_close(m_audioCodecContext);
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = 0;
    }

    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
        avformat_free_context(m_formatContext);
        m_formatContext = 0;
    }

    if (m_packet) {
        av_packet_free(&m_packet);
        m_packet = 0;
    }

    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
}
