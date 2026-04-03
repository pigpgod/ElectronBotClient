#ifndef ELECTRONBOTSDK_ELECTRONLOWLEVEL_H
#define ELECTRONBOTSDK_ELECTRONLOWLEVEL_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <cstdint>

class ElectronLowLevel : public QObject
{
    Q_OBJECT

public:
    explicit ElectronLowLevel(QObject *parent = nullptr);
    ~ElectronLowLevel();

    bool Connect();
    bool Disconnect();
    void SetImageSrc(const QImage &image);
    void SetExtraData(uint8_t* _data, uint32_t _len = 32);
    void SetJointAngles(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6,
                        bool _enable = false);
    void GetJointAngles(float* _jointAngles);
    uint8_t* GetExtraData(uint8_t* _data = nullptr);

    int USB_VID = 0x1001;
    int USB_PID = 0x8023;
    bool isConnected = false;

signals:
    void connectionStatusChanged(bool connected);
    void connectFinished(bool success);

private:
    void runConnect();
    bool runSync();
    bool ReceivePacket(uint8_t* _buffer, uint32_t _packetCount, uint32_t _packetSize);
    bool TransmitPacket(uint8_t* _buffer, uint32_t _packetCount, uint32_t _packetSize);

    QThread *workerThread;
    QMutex dataMutex;
    bool newFrameAvailable = false;
    bool shouldStop = false;
    bool isTransmitting = false;

    uint8_t pingPongWriteIndex = 0;
    uint8_t usbBuffer200[200]{};
    uint8_t frameBufferTx[2][240 * 240 * 3]{};
    uint8_t extraDataBufferTx[2][32]{};
    uint8_t extraDataBufferRx[32]{};
    QImage pendingImage;
    uint32_t timeStamp = 0;
};

#endif
