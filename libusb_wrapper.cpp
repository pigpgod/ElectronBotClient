/**
 * @file libusb_wrapper.cpp
 * @brief libusb-win32 封装类实现
 * 
 * 实现 USB 设备的底层操作：
 * - 设备初始化和释放
 * - 设备扫描和打开
 * - 批量数据读写（含 512 字节对齐的零长度包处理）
 * - 设备连接状态检测
 * - 接口声明和释放
 */

#include "libusb_wrapper.h"
#include <lusb0_usb.h>

LibUsbWrapper::LibUsbWrapper()
    : initialized(false)
{
    for (int i = 0; i < kMaxDevices; i++)
    {
        deviceHandles[i] = nullptr;
        deviceVids[i] = 0;
        devicePids[i] = 0;
    }
}

LibUsbWrapper::~LibUsbWrapper()
{
    deinit();
}

bool LibUsbWrapper::init()
{
    if (initialized) return true;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    initialized = true;
    return true;
}

void LibUsbWrapper::deinit()
{
    if (!initialized) return;

    for (int i = 0; i < kMaxDevices; i++)
    {
        if (deviceHandles[i] != nullptr)
        {
            usb_close(static_cast<usb_dev_handle*>(deviceHandles[i]));
            deviceHandles[i] = nullptr;
        }
    }

    initialized = false;
}

int LibUsbWrapper::scanDevice(int vid, int pid)
{
    if (!initialized) return 0;

    usb_find_busses();
    usb_find_devices();

    int found = 0;

    for (struct usb_bus* bus = usb_get_busses(); bus != nullptr; bus = bus->next)
    {
        for (struct usb_device* dev = bus->devices; dev != nullptr; dev = dev->next)
        {
            if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid)
            {
                found++;
            }
        }
    }

    return found;
}

bool LibUsbWrapper::openDevice(int index, int vid, int pid)
{
    if (!initialized || index < 0 || index >= kMaxDevices) return false;
    if (deviceHandles[index] != nullptr) return false;

    usb_find_busses();
    usb_find_devices();

    int foundIndex = 0;

    for (struct usb_bus* bus = usb_get_busses(); bus != nullptr; bus = bus->next)
    {
        for (struct usb_device* dev = bus->devices; dev != nullptr; dev = dev->next)
        {
            if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid)
            {
                if (foundIndex == index)
                {
                    usb_dev_handle* handle = usb_open(dev);
                    if (handle != nullptr)
                    {
                        deviceHandles[index] = handle;
                        deviceVids[index] = vid;
                        devicePids[index] = pid;
                        return true;
                    }
                    return false;
                }
                foundIndex++;
            }
        }
    }

    return false;
}

void LibUsbWrapper::closeDevice(int index)
{
    if (index < 0 || index >= kMaxDevices) return;
    if (deviceHandles[index] == nullptr) return;

    usb_close(static_cast<usb_dev_handle*>(deviceHandles[index]));
    deviceHandles[index] = nullptr;
    deviceVids[index] = 0;
    devicePids[index] = 0;
}

bool LibUsbWrapper::isDeviceConnected(int index)
{
    if (!initialized || index < 0 || index >= kMaxDevices) return false;
    if (deviceHandles[index] == nullptr) return false;

    usb_find_busses();
    usb_find_devices();

    int vid = deviceVids[index];
    int pid = devicePids[index];

    for (struct usb_bus* bus = usb_get_busses(); bus != nullptr; bus = bus->next)
    {
        for (struct usb_device* dev = bus->devices; dev != nullptr; dev = dev->next)
        {
            if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid)
            {
                return true;
            }
        }
    }

    return false;
}

int LibUsbWrapper::bulkRead(int index, int endpoint, uint8_t* data, int length, int timeoutMs)
{
    if (!initialized || index < 0 || index >= kMaxDevices) return -1;
    if (deviceHandles[index] == nullptr) return -1;

    int ret = usb_bulk_read(static_cast<usb_dev_handle*>(deviceHandles[index]),
                            endpoint, reinterpret_cast<char*>(data), length, timeoutMs);
    return ret;
}

int LibUsbWrapper::bulkWrite(int index, int endpoint, const uint8_t* data, int length, int timeoutMs)
{
    if (!initialized || index < 0 || index >= kMaxDevices) return -1;
    if (deviceHandles[index] == nullptr) return -1;

    int ret = usb_bulk_write(static_cast<usb_dev_handle*>(deviceHandles[index]),
                             endpoint, reinterpret_cast<char*>(const_cast<uint8_t*>(data)), length, timeoutMs);
    
    if ((length % 512) == 0)
    {
        usb_bulk_write(static_cast<usb_dev_handle*>(deviceHandles[index]),
                       endpoint, reinterpret_cast<char*>(const_cast<uint8_t*>(data)), 0, timeoutMs);
    }

    return ret;
}

bool LibUsbWrapper::claimInterface(int index, int interfaceNum)
{
    if (!initialized || index < 0 || index >= kMaxDevices) return false;
    if (deviceHandles[index] == nullptr) return false;

    int ret = usb_claim_interface(static_cast<usb_dev_handle*>(deviceHandles[index]), interfaceNum);
    return ret == 0;
}

bool LibUsbWrapper::releaseInterface(int index, int interfaceNum)
{
    if (!initialized || index < 0 || index >= kMaxDevices) return false;
    if (deviceHandles[index] == nullptr) return false;

    int ret = usb_release_interface(static_cast<usb_dev_handle*>(deviceHandles[index]), interfaceNum);
    return ret == 0;
}
