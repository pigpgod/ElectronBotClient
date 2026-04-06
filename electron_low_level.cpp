/**
 * @file electron_low_level.cpp
 * @brief ElectronBot 低层 USB 通信类实现
 * 
 * 实现功能：
 * - USB 设备连接/断开管理
 * - 工作线程管理
 * - 图像帧数据处理和传输
 * - 关节角度和额外数据收发
 */

#include "electron_low_level.h"
#include "libusb_wrapper.h"

ElectronLowLevel::ElectronLowLevel(QObject *parent)
    : QObject(parent)
    , workerThread(0)
    , usb(new LibUsbWrapper())
    , deviceCheckCounter(0)
    , connectSuccess(false)
    , newFrameAvailable(false)
    , shouldStop(false)
    , isTransmitting(false)
    , pingPongWriteIndex(0)
    , timeStamp(0)
{
    USB_VID = 0x1001;
    USB_PID = 0x8023;
    isConnected = false;
    usb->init();
}

ElectronLowLevel::~ElectronLowLevel()
{
    Disconnect();
    delete usb;
}

void ElectronLowLevel::stopWorkerThread()
{
    if (workerThread == 0) return;

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
    workerThread = 0;
}

bool ElectronLowLevel::Connect()
{
    stopWorkerThread();

    usb->closeDevice(0);

    if (isConnected)
    {
        isConnected = false;
        emit connectionStatusChanged(false);
    }

    shouldStop = false;
    connectSuccess = false;

    workerThread = QThread::create([this]() {
        runConnect();
    });

    connect(workerThread, SIGNAL(finished()), this, SLOT(onThreadFinished()));

    workerThread->start();

    int maxAttempts = kConnectTimeoutMs / kConnectPollIntervalMs;
    for (int i = 0; i < maxAttempts; i++)
    {
        QThread::msleep(kConnectPollIntervalMs);
        if (connectSuccess)
        {
            return true;
        }
        if (!workerThread->isRunning())
        {
            break;
        }
    }

    stopWorkerThread();
    return false;
}

void ElectronLowLevel::onThreadFinished()
{
    if (!connectSuccess)
    {
        isConnected = false;
    }
}

void ElectronLowLevel::onConnected()
{
    emit connectionStatusChanged(true);
}

void ElectronLowLevel::onDisconnected()
{
    emit connectionStatusChanged(false);
}

void ElectronLowLevel::runConnect()
{
    int devNum = usb->scanDevice(USB_VID, USB_PID);

    if (devNum <= 0)
    {
        return;
    }

    if (!usb->openDevice(0, USB_VID, USB_PID))
    {
        return;
    }

    usb->claimInterface(0, 0);

    isConnected = true;
    connectSuccess = true;
    timeStamp = 0;
    deviceCheckCounter = 0;

    QMetaObject::invokeMethod(this, "onConnected", Qt::QueuedConnection);

    while (!shouldStop && isConnected)
    {
        deviceCheckCounter++;
        if (deviceCheckCounter >= kDeviceCheckLoopCount)
        {
            deviceCheckCounter = 0;
            if (!usb->isDeviceConnected(0))
            {
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

    usb->releaseInterface(0, 0);
    usb->closeDevice(0);

    if (!shouldStop)
    {
        QMetaObject::invokeMethod(this, "onDisconnected", Qt::QueuedConnection);
    }
}

bool ElectronLowLevel::Disconnect()
{
    stopWorkerThread();

    if (isConnected)
    {
        usb->closeDevice(0);
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
        ret = usb->bulkRead(0, kEp1In, buffer, packetSize, kUsbTimeoutMs);
        if (ret <= 0)
        {
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
        ret = usb->bulkWrite(0, kEp1Out, buffer + dataOffset, packetSize, kUsbTimeoutMs);
        if (ret <= 0)
        {
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
    if (data != 0)
    {
        memcpy(data, extraDataBufferRx, kExtraDataSize);
    }

    return extraDataBufferRx;
}

void ElectronLowLevel::SetJointAngles(float j1, float j2, float j3, float j4, float j5, float j6,
                                      bool enable)
{
    float jointAngleSetPoints[kJointCount] = {j1, j2, j3, j4, j5, j6};

    for (int bufIndex = 0; bufIndex < 2; bufIndex++)
    {
        extraDataBufferTx[bufIndex][0] = enable ? 1 : 0;

        for (int j = 0; j < kJointCount; j++)
        {
            unsigned char* bytes = reinterpret_cast<unsigned char*>(&jointAngleSetPoints[j]);
            for (int i = 0; i < 4; i++)
            {
                extraDataBufferTx[bufIndex][j * 4 + i + 1] = bytes[i];
            }
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
