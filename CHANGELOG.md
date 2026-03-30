# ElectronBot Client 修改说明

## 概述

本次更新将机器人连接方式从 **TCP Socket** 改为 **USB 直连**，并实现连接后自动播放视频到机器人屏幕。

## 主要变更

### 1. 新增 USB 连接支持

添加了 `ElectronLowLevel` 类（基于 `electron_low_level.h/cpp`），通过 USB 接口直接连接 ElectronBot 机器人。

**新增文件：**
- `electron_low_level.h` - 机器人控制类头文件
- `electron_low_level.cpp` - USB 通信实现
- `3rdparty/USBInterface.h` - USB 接口定义
- `3rdparty/USBInterface.dll` - USB 通信动态库
- `3rdparty/lib/USBInterface.lib` - USB 接口导入库

### 2. 连接方式改变

| 变更项 | 旧版本 | 新版本 |
|--------|--------|--------|
| 连接方式 | TCP Socket (IP:Port) | USB 直连 (VID: 0x1001, PID: 0x8023) |
| 连接按钮 | Connect to Bot | Connect to Bot (切换) |
| 状态显示 | IP地址和端口 | Connected/Disconnected |

### 3. 视频播放功能

- 连接成功后自动加载并循环播放 `happy.mp4` 视频
- 每帧视频通过 USB 同步到机器人 240x240 屏幕
- 断开连接后停止播放

### 4. 异步通信架构

`ElectronLowLevel` 使用 `QThread` 实现异步 USB 通信，避免阻塞 UI 线程：

- `Connect()` - 在后台线程扫描并连接 USB 设备
- `SetImageSrc()` - 非阻塞式传递图像数据（线程安全）
- 后台线程持续监控新帧并同步到机器人

### 5. 图像处理

图像缩放到 240x240 像素（机器人屏幕分辨率），逐像素拷贝确保数据对齐：

```cpp
QImage scaled = pendingImage.scaled(240, 240, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
QImage rgb = scaled.convertToFormat(QImage::Format_RGB888);
// 逐像素拷贝到 frameBufferTx
```

## 项目结构

```
ElectronBotClient/
├── electron_low_level.h/cpp    # USB 机器人控制
├── mainwindow.h/cpp            # 主窗口（视频播放+连接控制）
├── ffmpegvideoplayer.h/cpp     # FFmpeg 视频播放
└── 3rdparty/
    ├── USBInterface.h/dll/lib # USB 通信库
    ├── bin/                    # FFmpeg DLL
    └── lib/                    # 导入库
```

## 使用方法

1. 点击 **"Connect to Bot"** 按钮连接机器人
2. 连接成功后自动播放视频到机器人屏幕
3. 再次点击 **"Disconnect"** 断开连接

## 技术细节

- **USB 通信协议**：Bulk Transfer，4个数据包传输一帧 240x240 RGB 图像
- **像素格式**：RGB888 (每像素 3 字节)
- **帧率**：取决于视频源帧率和 USB 带宽
