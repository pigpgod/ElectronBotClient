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
    bool isRunning() const;

public slots:
    void start();
    void stop();

signals:
    void resultReady(const QString &text);
    void partialResultReady(const QString &text);
    void errorOccurred(const QString &message);
    void recognitionStarted();
    void recognitionStopped();

private slots:
    void onAudioDataReady();
    void onBridgeReadyRead();
    void onBridgeError(QProcess::ProcessError error);
    void onBridgeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void tryParseJson();

private:
    QProcess *m_bridgeProcess;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QString m_modelPath;
    QByteArray m_buffer;
    bool m_running;
    bool m_bridgeReady;
};

#endif
