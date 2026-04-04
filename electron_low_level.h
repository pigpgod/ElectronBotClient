#ifndef ELECTRONBOTSDK_ELECTRONLOWLEVEL_H
#define ELECTRONBOTSDK_ELECTRONLOWLEVEL_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <cstdint>

class LibUsbWrapper;

class ElectronLowLevel : public QObject
{
    Q_OBJECT

public:
    explicit ElectronLowLevel(QObject *parent = nullptr);
    ~ElectronLowLevel();

    bool Connect();
    bool Disconnect();
    void SetImageSrc(const QImage &image);
    void SetExtraData(uint8_t* data, uint32_t len = kExtraDataSize);
    void SetJointAngles(float j1, float j2, float j3, float j4, float j5, float j6,
                        bool enable = false);
    void GetJointAngles(float* jointAngles);
    uint8_t* GetExtraData(uint8_t* data = nullptr);

    int USB_VID = 0x1001;
    int USB_PID = 0x8023;
    bool isConnected = false;

signals:
    void connectionStatusChanged(bool connected);
    void connectFinished(bool success);

private:
    static constexpr int kImageSize = 240;
    static constexpr int kExtraDataSize = 32;
    static constexpr int kJointCount = 6;
    static constexpr int kFrameBufferSize = kImageSize * kImageSize * 3;
    static constexpr int kUsbTimeoutMs = 100;
    static constexpr int kConnectTimeoutMs = 5000;
    static constexpr int kConnectPollIntervalMs = 100;
    static constexpr int kThreadWaitMs = 1000;
    static constexpr int kDeviceCheckIntervalMs = 50;
    static constexpr int kDeviceCheckLoopCount = kDeviceCheckIntervalMs;
    
    static constexpr int kEp1In = 0x81;
    static constexpr int kEp1Out = 0x01;
    
    static constexpr int kPacketSize512 = 512;
    static constexpr int kPacketSize224 = 224;
    static constexpr int kPacketCount84 = 84;
    static constexpr int kPacketCount1 = 1;
    static constexpr int kFrameChunkSize = 43008;
    static constexpr int kFrameTailSize = 192;
    static constexpr int kTransferPhases = 4;

    void runConnect();
    bool runSync();
    bool processImageFrame();
    bool transmitFrameData(int pingPongIndex);
    bool receivePacket(uint8_t* buffer, uint32_t packetCount, uint32_t packetSize);
    bool transmitPacket(uint8_t* buffer, uint32_t packetCount, uint32_t packetSize);
    void stopWorkerThread();

    QThread *workerThread;
    QMutex dataMutex;
    LibUsbWrapper* usb;
    int deviceCheckCounter = 0;
    bool connectSuccess = false;
    bool newFrameAvailable = false;
    bool shouldStop = false;
    bool isTransmitting = false;

    uint8_t pingPongWriteIndex = 0;
    uint8_t usbBuffer200[200]{};
    uint8_t frameBufferTx[2][kFrameBufferSize]{};
    uint8_t extraDataBufferTx[2][kExtraDataSize]{};
    uint8_t extraDataBufferRx[kExtraDataSize]{};
    QImage pendingImage;
    uint32_t timeStamp = 0;
};

#endif
