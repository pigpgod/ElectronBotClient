#ifndef LIBUSB_WRAPPER_H
#define LIBUSB_WRAPPER_H

#include <cstdint>

class LibUsbWrapper
{
public:
    LibUsbWrapper();
    ~LibUsbWrapper();

    bool init();
    void deinit();

    int scanDevice(int vid, int pid);
    bool openDevice(int index, int vid, int pid);
    void closeDevice(int index);
    bool isDeviceConnected(int index);

    int bulkRead(int index, int endpoint, uint8_t* data, int length, int timeoutMs);
    int bulkWrite(int index, int endpoint, const uint8_t* data, int length, int timeoutMs);

    bool claimInterface(int index, int interfaceNum);
    bool releaseInterface(int index, int interfaceNum);

private:
    static constexpr int kMaxDevices = 4;
    
    bool initialized;
    void* deviceHandles[kMaxDevices];
    int deviceVids[kMaxDevices];
    int devicePids[kMaxDevices];
};

#endif
