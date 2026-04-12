#include "voskrecognizer.h"
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QCoreApplication>
#include <QFile>

VoskRecognizer::VoskRecognizer(QObject *parent)
    : QObject(parent)
    , m_bridgeProcess(0)
    , m_audioInput(0)
    , m_audioDevice(0)
    , m_running(false)
    , m_bridgeReady(false)
{
}

VoskRecognizer::~VoskRecognizer()
{
    stop();
}

void VoskRecognizer::setModelPath(const QString &path)
{
    m_modelPath = path;
}

bool VoskRecognizer::isRunning() const
{
    return m_running;
}

void VoskRecognizer::start()
{
    if (m_running) return;

    QString appDir = QCoreApplication::applicationDirPath();
    QString bridgeExe = appDir + "/vosk/vosk_bridge.exe";

    if (!QFile::exists(bridgeExe)) {
        emit errorOccurred(QString("找不到语音桥接程序: %1").arg(bridgeExe));
        return;
    }

    if (!QFile::exists(m_modelPath)) {
        emit errorOccurred(QString("找不到语音模型: %1").arg(m_modelPath));
        return;
    }

    m_bridgeProcess = new QProcess(this);
    m_bridgeProcess->setWorkingDirectory(appDir + "/vosk");
    m_bridgeProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_bridgeProcess, SIGNAL(readyRead()), this, SLOT(onBridgeReadyRead()));
    connect(m_bridgeProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onBridgeError(QProcess::ProcessError)));
    connect(m_bridgeProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBridgeFinished(int,QProcess::ExitStatus)));

    m_bridgeReady = false;
    m_buffer.clear();
    m_bridgeProcess->start(bridgeExe, QStringList() << m_modelPath);

    if (!m_bridgeProcess->waitForStarted(5000)) {
        emit errorOccurred(QString("无法启动语音桥接程序: %1").arg(m_bridgeProcess->errorString()));
        m_bridgeProcess->deleteLater();
        m_bridgeProcess = 0;
        return;
    }

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultInputDevice();
    
    if (deviceInfo.isNull()) {
        m_bridgeProcess->kill();
        m_bridgeProcess->waitForFinished(3000);
        m_bridgeProcess->deleteLater();
        m_bridgeProcess = 0;
        emit errorOccurred("未找到麦克风设备");
        return;
    }

    if (!deviceInfo.isFormatSupported(format)) {
        format = deviceInfo.nearestFormat(format);
    }

    m_audioInput = new QAudioInput(deviceInfo, format, this);
    m_audioDevice = m_audioInput->start();

    if (!m_audioDevice) {
        delete m_audioInput;
        m_audioInput = 0;
        m_bridgeProcess->kill();
        m_bridgeProcess->waitForFinished(3000);
        m_bridgeProcess->deleteLater();
        m_bridgeProcess = 0;
        emit errorOccurred("无法启动麦克风");
        return;
    }

    connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(onAudioDataReady()));

    m_running = true;
    emit recognitionStarted();
}

void VoskRecognizer::stop()
{
    if (!m_running) return;

    m_running = false;
    m_bridgeReady = false;

    if (m_audioInput) {
        m_audioInput->stop();
        delete m_audioInput;
        m_audioInput = 0;
        m_audioDevice = 0;
    }

    if (m_bridgeProcess) {
        m_bridgeProcess->closeWriteChannel();
        m_bridgeProcess->waitForFinished(3000);
        if (m_bridgeProcess->state() != QProcess::NotRunning) {
            m_bridgeProcess->kill();
            m_bridgeProcess->waitForFinished(1000);
        }
        m_bridgeProcess->deleteLater();
        m_bridgeProcess = 0;
    }

    m_buffer.clear();
    emit recognitionStopped();
}

void VoskRecognizer::onAudioDataReady()
{
    if (!m_running || !m_audioDevice || !m_bridgeProcess) return;

    QByteArray data = m_audioDevice->readAll();
    if (data.isEmpty()) return;

    m_bridgeProcess->write(data);
}

void VoskRecognizer::onBridgeReadyRead()
{
    if (!m_bridgeProcess) return;

    m_buffer += m_bridgeProcess->readAll();
    tryParseJson();
}

void VoskRecognizer::tryParseJson()
{
    while (true) {
        int start = m_buffer.indexOf('{');
        if (start < 0) {
            m_buffer.clear();
            return;
        }

        if (start > 0) {
            m_buffer = m_buffer.mid(start);
        }

        int depth = 0;
        int end = -1;
        for (int i = 0; i < m_buffer.size(); ++i) {
            char c = m_buffer.at(i);
            if (c == '{') depth++;
            else if (c == '}') {
                depth--;
                if (depth == 0) {
                    end = i + 1;
                    break;
                }
            }
        }

        if (end < 0) return;

        QByteArray jsonBytes = m_buffer.left(end);
        m_buffer = m_buffer.mid(end);

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &error);
        if (doc.isNull()) continue;

        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();

        if (type == "ready") {
            m_bridgeReady = true;
        } else if (type == "error") {
            emit errorOccurred(obj.value("message").toString());
        } else if (obj.contains("text")) {
            QString text = obj.value("text").toString().trimmed();
            if (!text.isEmpty()) {
                emit resultReady(text);
            }
        } else if (obj.contains("partial")) {
            QString text = obj.value("partial").toString().trimmed();
            if (!text.isEmpty()) {
                emit partialResultReady(text);
            }
        }
    }
}

void VoskRecognizer::onBridgeError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    if (m_running) {
        emit errorOccurred(QString("语音桥接程序发生错误: %1").arg(
            m_bridgeProcess ? m_bridgeProcess->errorString() : "未知错误"));
        stop();
    }
}

void VoskRecognizer::onBridgeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    
    if (m_running) {
        m_running = false;
        m_bridgeReady = false;

        if (m_audioInput) {
            m_audioInput->stop();
            delete m_audioInput;
            m_audioInput = 0;
            m_audioDevice = 0;
        }

        if (m_bridgeProcess) {
            m_bridgeProcess->deleteLater();
            m_bridgeProcess = 0;
        }

        m_buffer.clear();
        emit recognitionStopped();
    }
}
