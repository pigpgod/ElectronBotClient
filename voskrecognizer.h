#ifndef VOSKRECOGNIZER_H
#define VOSKRECOGNIZER_H

#include <QObject>
#include <QAudioInput>
#include <QIODevice>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

class VoskRecognizer : public QObject
{
    Q_OBJECT

public:
    explicit VoskRecognizer(QObject *parent = 0);
    ~VoskRecognizer();

    void setModelPath(const QString &path);
    void setInputDevice(const QAudioDeviceInfo &deviceInfo);
    void setVolumeGain(qreal gain);
    QList<QAudioDeviceInfo> availableInputDevices() const;
    bool isRunning() const { return m_running; }
    bool isInitialized() const { return m_initialized; }
    bool isInitializing() const { return m_initializing; }

    void initialize();
    void start();
    void stop();
    void pauseAudio();
    void resumeAudio();

signals:
    void resultReady(const QString &text);
    void partialResultReady(const QString &text);
    void errorOccurred(const QString &message);
    void recognitionStarted();
    void recognitionStopped();
    void initializationComplete();

private slots:
    void onAudioDataReady();
    void onBridgeReadyRead();
    void onBridgeError(QProcess::ProcessError error);
    void onBridgeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void tryParseJson();
    bool startBridgeProcess();
    bool startAudioInput();

private:
    QProcess *m_bridgeProcess;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QAudioDeviceInfo m_inputDevice;
    QString m_modelPath;
    QByteArray m_buffer;
    qreal m_volumeGain;
    bool m_initialized;
    bool m_initializing;
    bool m_running;
    bool m_bridgeReady;
};

#endif
