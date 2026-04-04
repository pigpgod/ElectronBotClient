#include "electron_low_level.h"
#include "USBInterface.h"
#include <QDebug>

ElectronLowLevel::ElectronLowLevel(QObject *parent)
    : QObject(parent)
    , workerThread(nullptr)
    , shouldStop(false)
    , isTransmitting(false)
{
}

ElectronLowLevel::~ElectronLowLevel()
{
    Disconnect();
}

void ElectronLowLevel::stopWorkerThread()
{
    if (workerThread == nullptr) return;

    shouldStop = true;
    
    if (workerThread->isRunning())
    {
        workerThread->quit();
        workerThread->wait(kThreadWaitMs);
    }
    
    if (workerThread->isRunning())
    {
        workerThread->terminate();
        workerThread->wait();
    }
    
    delete workerThread;
    workerThread = nullptr;
}

bool ElectronLowLevel::Connect()
{
    if (isConnected)
    {
        Disconnect();
    }

    stopWorkerThread();

    int devNum = USB_ScanDevice(USB_PID, USB_VID);
    if (devNum <= 0)
    {
        isConnected = false;
        return false;
    }

    shouldStop = false;
    workerThread = QThread::create([this]() {
        runConnect();
    });

    connect(workerThread, &QThread::finished, this, [this]() {
        isConnected = false;
    });

    workerThread->start();

    int maxAttempts = kConnectTimeoutMs / kConnectPollIntervalMs;
    for (int i = 0; i < maxAttempts; i++)
    {
        QThread::msleep(kConnectPollIntervalMs);
        if (isConnected)
        {
            return true;
        }
    }

    stopWorkerThread();
    return false;
}

void ElectronLowLevel::runConnect()
{
    int devNum = USB_ScanDevice(USB_PID, USB_VID);

    if (devNum > 0 && USB_OpenDevice(0))
    {
        isConnected = true;
        timeStamp = 0;
        deviceCheckCounter = 0;

        QMetaObject::invokeMethod(this, [this]() {
            emit connectionStatusChanged(true);
        }, Qt::QueuedConnection);

        while (!shouldStop && isConnected)
        {
            deviceCheckCounter++;
            if (deviceCheckCounter >= kDeviceCheckLoopCount)
            {
                deviceCheckCounter = 0;
                bool checkResult = USB_CheckDevice(0);
                qDebug() << "USB_CheckDevice result:" << checkResult;
                if (!checkResult)
                {
                    qDebug() << "Device disconnected detected!";
                    isConnected = false;
                    break;
                }
            }

            QMutexLocker locker(&dataMutex);
            if (newFrameAvailable)
            {
                if (!isTransmitting)
                {
                    newFrameAvailable = false;
                    locker.unlock();
                    if (!runSync())
                    {
                        isConnected = false;
                        break;
                    }
                }
                else
                {
                    newFrameAvailable = false;
                    locker.unlock();
                }
            }
            QThread::msleep(1);
        }

        USB_CloseDevice(0);
        qDebug() << "USB device closed";

        if (!shouldStop)
        {
            QMetaObject::invokeMethod(this, [this]() {
                emit connectionStatusChanged(false);
            }, Qt::QueuedConnection);
        }
    }
}

bool ElectronLowLevel::Disconnect()
{
    stopWorkerThread();

    if (isConnected)
    {
        USB_CloseDevice(0);
        isConnected = false;
    }

    emit connectionStatusChanged(false);
    return true;
}

void ElectronLowLevel::SetImageSrc(const QImage &image)
{
    if (!isConnected) return;

    QMutexLocker locker(&dataMutex);
    pendingImage = image.copy();
    newFrameAvailable = true;
}

bool ElectronLowLevel::processImageFrame()
{
    if (pendingImage.isNull())
    {
        return true;
    }

    QImage scaled = pendingImage.scaled(kImageSize, kImageSize, 
                                        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage rgb = scaled.convertToFormat(QImage::Format_RGB888);

    const uint8_t* src = rgb.bits();
    int srcStride = rgb.bytesPerLine();
    uint8_t* dst = frameBufferTx[pingPongWriteIndex];

    for (int y = 0; y < kImageSize; y++)
    {
        const uint8_t* srcRow = src + y * srcStride;
        uint8_t* dstRow = dst + y * kImageSize * 3;
        
        for (int x = 0; x < kImageSize; x++)
        {
            dstRow[x * 3] = srcRow[x * 3];
            dstRow[x * 3 + 1] = srcRow[x * 3 + 1];
            dstRow[x * 3 + 2] = srcRow[x * 3 + 2];
        }
    }

    return true;
}

bool ElectronLowLevel::transmitFrameData(int pingPongIndex)
{
    uint32_t frameBufferOffset = 0;

    for (int p = 0; p < kTransferPhases; p++)
    {
        if (!receivePacket(reinterpret_cast<uint8_t*>(extraDataBufferRx), 
                          kPacketCount1, kExtraDataSize))
        {
            return false;
        }

        if (!transmitPacket(reinterpret_cast<uint8_t*>(frameBufferTx[pingPongIndex]) + frameBufferOffset, 
                           kPacketCount84, kPacketSize512))
        {
            return false;
        }
        frameBufferOffset += kFrameChunkSize;

        memcpy(usbBuffer200, 
               reinterpret_cast<uint8_t*>(frameBufferTx[pingPongIndex]) + frameBufferOffset, 
               kFrameTailSize);
        memcpy(usbBuffer200 + kFrameTailSize, 
               reinterpret_cast<uint8_t*>(extraDataBufferTx[pingPongIndex]), 
               kExtraDataSize);

        if (!transmitPacket(usbBuffer200, kPacketCount1, kPacketSize224))
        {
            return false;
        }
        frameBufferOffset += kFrameTailSize;
    }

    return true;
}

bool ElectronLowLevel::runSync()
{
    isTransmitting = true;

    int index = pingPongWriteIndex;
    pingPongWriteIndex = pingPongWriteIndex == 0 ? 1 : 0;

    if (!processImageFrame())
    {
        isTransmitting = false;
        return false;
    }

    if (!transmitFrameData(index))
    {
        isTransmitting = false;
        return false;
    }

    timeStamp++;
    isTransmitting = false;
    return true;
}

bool ElectronLowLevel::receivePacket(uint8_t* buffer, uint32_t packetCount, uint32_t packetSize)
{
    uint32_t remaining = packetCount;
    int ret;
    
    do
    {
        ret = USB_BulkReceive(0, kEp1In, reinterpret_cast<char*>(buffer), 
                              packetSize, kUsbTimeoutMs);
        if (ret <= 0)
        {
            qDebug() << "receivePacket failed, ret:" << ret;
            return false;
        }
        if (ret == (int)packetSize)
        {
            remaining--;
        }
    } while (remaining > 0);

    return true;
}

bool ElectronLowLevel::transmitPacket(uint8_t* buffer, uint32_t packetCount, uint32_t packetSize)
{
    uint32_t remaining = packetCount;
    uint32_t dataOffset = 0;
    int ret;
    
    do
    {
        ret = USB_BulkTransmit(0, kEp1Out,
                               reinterpret_cast<char*>(buffer) + dataOffset,
                               packetSize, kUsbTimeoutMs);
        if (ret <= 0)
        {
            qDebug() << "transmitPacket failed, ret:" << ret;
            return false;
        }
        dataOffset += packetSize;
        remaining--;
    } while (remaining > 0);

    return true;
}

void ElectronLowLevel::SetExtraData(uint8_t* data, uint32_t len)
{
    if (len <= kExtraDataSize)
    {
        memcpy(extraDataBufferTx[pingPongWriteIndex], data, len);
    }
}

uint8_t* ElectronLowLevel::GetExtraData(uint8_t* data)
{
    if (data != nullptr)
    {
        memcpy(data, extraDataBufferRx, kExtraDataSize);
    }

    return extraDataBufferRx;
}

void ElectronLowLevel::SetJointAngles(float j1, float j2, float j3, float j4, float j5, float j6,
                                      bool enable)
{
    float jointAngleSetPoints[kJointCount] = {j1, j2, j3, j4, j5, j6};

    extraDataBufferTx[pingPongWriteIndex][0] = enable ? 1 : 0;
    
    for (int j = 0; j < kJointCount; j++)
    {
        auto* bytes = reinterpret_cast<unsigned char*>(&jointAngleSetPoints[j]);
        for (int i = 0; i < 4; i++)
        {
            extraDataBufferTx[pingPongWriteIndex][j * 4 + i + 1] = bytes[i];
        }
    }
}

void ElectronLowLevel::GetJointAngles(float* jointAngles)
{
    for (int j = 0; j < kJointCount; j++)
    {
        jointAngles[j] = *reinterpret_cast<float*>(extraDataBufferRx + 4 * j + 1);
    }
}
