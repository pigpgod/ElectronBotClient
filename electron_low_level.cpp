#include "electron_low_level.h"
#include "USBInterface.h"

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

bool ElectronLowLevel::Connect()
{
    int devNum = USB_ScanDevice(USB_PID, USB_VID);
    if (devNum <= 0)
    {
        isConnected = false;
        return false;
    }

    if (workerThread != nullptr)
    {
        shouldStop = true;
        if (workerThread->isRunning())
        {
            workerThread->quit();
            workerThread->wait(1000);
        }
        if (workerThread->isRunning())
        {
            workerThread->terminate();
            workerThread->wait();
        }
        delete workerThread;
        workerThread = nullptr;
    }

    shouldStop = false;
    workerThread = QThread::create([this]() {
        runConnect();
    });

    connect(workerThread, &QThread::finished, this, [this]() {
        isConnected = false;
        emit connectionStatusChanged(false);
    });

    workerThread->start();

    for (int i = 0; i < 50; i++) {
        QThread::msleep(100);
        if (isConnected) {
            return true;
        }
    }

    emit connectFinished(false);
    return false;
}

void ElectronLowLevel::runConnect()
{
    int devNum = USB_ScanDevice(USB_PID, USB_VID);

    if (devNum > 0)
    {
        if (USB_OpenDevice(0))
        {
            isConnected = true;
            timeStamp = 0;

            QMetaObject::invokeMethod(this, [this]() {
                emit connectionStatusChanged(true);
            }, Qt::QueuedConnection);

            while (!shouldStop && isConnected)
            {
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
        }
    }
}

bool ElectronLowLevel::Disconnect()
{
    if (!isConnected && workerThread == nullptr) return true;

    shouldStop = true;

    if (workerThread != nullptr)
    {
        if (workerThread->isRunning())
        {
            workerThread->quit();
            workerThread->wait(1000);
        }
        if (workerThread->isRunning())
        {
            workerThread->terminate();
            workerThread->wait();
        }
        delete workerThread;
        workerThread = nullptr;
    }

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

bool ElectronLowLevel::runSync()
{
    isTransmitting = true;

    uint32_t frameBufferOffset = 0;
    uint8_t index = pingPongWriteIndex;
    pingPongWriteIndex = pingPongWriteIndex == 0 ? 1 : 0;

    if (pendingImage.isNull()) {
        isTransmitting = false;
        return true;
    }

    QImage scaled = pendingImage.scaled(240, 240, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage rgb = scaled.convertToFormat(QImage::Format_RGB888);

    const uint8_t* src = rgb.bits();
    int srcStride = rgb.bytesPerLine();

    for (int y = 0; y < 240; y++)
    {
        const uint8_t* srcRow = src + y * srcStride;
        uint8_t* dstRow = frameBufferTx[index] + y * 240 * 3;
        for (int x = 0; x < 240; x++)
        {
            dstRow[x * 3] = srcRow[x * 3];
            dstRow[x * 3 + 1] = srcRow[x * 3 + 1];
            dstRow[x * 3 + 2] = srcRow[x * 3 + 2];
        }
    }

    for (int p = 0; p < 4; p++)
    {
        if (!ReceivePacket(reinterpret_cast<uint8_t*>(extraDataBufferRx), 1, 32))
        {
            isTransmitting = false;
            return false;
        }

        if (!TransmitPacket(reinterpret_cast<uint8_t*>(frameBufferTx[index]) + frameBufferOffset, 84, 512))
        {
            isTransmitting = false;
            return false;
        }
        frameBufferOffset += 43008;

        memcpy(usbBuffer200, reinterpret_cast<uint8_t*>(frameBufferTx[index]) + frameBufferOffset, 192);
        memcpy(usbBuffer200 + 192, reinterpret_cast<uint8_t*>(extraDataBufferTx[index]), 32);

        if (!TransmitPacket(usbBuffer200, 1, 224))
        {
            isTransmitting = false;
            return false;
        }
        frameBufferOffset += 192;
    }

    timeStamp++;
    isTransmitting = false;
    return true;
}

bool ElectronLowLevel::ReceivePacket(uint8_t* _buffer, uint32_t _packetCount, uint32_t _packetSize)
{
    uint32_t packetCount = _packetCount;
    uint32_t ret;
    do
    {
        do
        {
            ret = USB_BulkReceive(0, EP1_IN, reinterpret_cast<char*>(_buffer), _packetSize, 100);
        } while (ret != _packetSize);

        packetCount--;
    } while (packetCount > 0);

    return packetCount == 0;
}

bool ElectronLowLevel::TransmitPacket(uint8_t* _buffer, uint32_t _packetCount, uint32_t _packetSize)
{
    uint32_t packetCount = _packetCount;
    uint32_t dataOffset = 0;
    uint32_t ret;
    do
    {
        do
        {
            ret = USB_BulkTransmit(0, EP1_OUT,
                                   reinterpret_cast<char*>(_buffer) + dataOffset,
                                   _packetSize, 100);
        } while (!ret);

        dataOffset += _packetSize;
        packetCount--;
    } while (packetCount > 0);

    return packetCount == 0;
}

void ElectronLowLevel::SetExtraData(uint8_t* _data, uint32_t _len)
{
    if (_len <= 32)
        memcpy(extraDataBufferTx[pingPongWriteIndex], _data, _len);
}

uint8_t* ElectronLowLevel::GetExtraData(uint8_t* _data)
{
    if (_data != nullptr)
        memcpy(_data, extraDataBufferRx, 32);

    return extraDataBufferRx;
}

void ElectronLowLevel::SetJointAngles(float _j1, float _j2, float _j3, float _j4, float _j5, float _j6,
                                      bool _enable)
{
    float jointAngleSetPoints[6];

    jointAngleSetPoints[0] = _j1;
    jointAngleSetPoints[1] = _j2;
    jointAngleSetPoints[2] = _j3;
    jointAngleSetPoints[3] = _j4;
    jointAngleSetPoints[4] = _j5;
    jointAngleSetPoints[5] = _j6;

    extraDataBufferTx[pingPongWriteIndex][0] = _enable ? 1 : 0;
    for (int j = 0; j < 6; j++)
        for (int i = 0; i < 4; i++)
        {
            auto* b = (unsigned char*) &(jointAngleSetPoints[j]);
            extraDataBufferTx[pingPongWriteIndex][j * 4 + i + 1] = *(b + i);
        }
}

void ElectronLowLevel::GetJointAngles(float* _jointAngles)
{
    for (int j = 0; j < 6; j++)
    {
        _jointAngles[j] = *((float*) (extraDataBufferRx + 4 * j + 1));
    }
}
