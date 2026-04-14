# ElectronBot Client

ElectronBot 桌面控制客户端，基于 Qt 5.14 开发，提供机器人控制、摄像头捕获和语音识别功能。

## 功能特性

- **机器人控制**：通过 USB 连接控制 ElectronBot 机器人
- **电机控制**：6 轴电机实时控制
- **摄像头捕获**：实时视频预览和图像捕获
- **语音识别**：基于 Vosk 的中文语音识别
- **视频播放**：FFmpeg 驱动的视频播放

## 系统要求

- Windows 10/11 (64-bit)
- Qt 5.14.2 或更高版本
- MinGW 64-bit 编译器
- USB 端口用于连接 ElectronBot 设备

## 编译步骤

### 1. 安装依赖

确保已安装以下软件：
- [Qt 5.14.2](https://download.qt.io/archive/qt/5.14/5.14.2/) 或更高版本
- [Qt Creator](https://www.qt.io/download-qt-installer)
- MinGW 64-bit 编译器

### 2. 克隆项目

```bash
git clone https://github.com/pigpgod/ElectronBotClient.git
cd ElectronBotClient
```

### 3. 下载语音模型

本项目使用 Vosk 语音识别引擎，需要下载中文语音模型：

1. 访问 [Vosk Models](https://alphacephei.com/vosk/models) 下载中文模型

2. 推荐下载以下模型之一：
   - **vosk-model-cn-0.22** (大型中文模型，准确率高，~1.8GB)
   - **vosk-model-small-cn-0.3** (小型中文模型，体积小，~45MB)

3. 将下载的模型文件夹重命名为 `vosk-model-cn`

4. 将模型文件夹放置到以下位置：
   ```
   ElectronBotClient/
   └── 3rdparty/
       └── vosk-models/
           └── vosk-model-cn/    ← 将模型放在这里
               ├── am/
               ├── conf/
               ├── graph/
               ├── ivector/
               ├── rescore/
               ├── rnnlm/
               └── README
   ```

### 4. 使用 Qt Creator 编译

1. 打开 `ElectronBotClient.pro` 文件
2. 选择 Qt 5.14.2 MinGW 64-bit 套件
3. 点击"配置项目"等待 CMake 完成后
4. 点击"运行 qmake"
5. 点击"构建"开始编译

### 5. 运行程序

编译完成后，可执行文件位于：
```
build-ElectronBotClient-Desktop_Qt_5_14_2_MinGW_64_bit-Debug/
└── debug/
    └── ElectronBotClient.exe
```

## 项目结构

```
ElectronBotClient/
├── 3rdparty/                 # 第三方库
│   ├── bin/                  # FFmpeg DLL 文件
│   ├── include/              # FFmpeg 头文件
│   ├── lib/                  # FFmpeg 库文件
│   ├── libusb-win32/         # LibUSB 驱动
│   └── vosk-models/          # 语音模型（需自行下载）
│       └── vosk-model-cn/     # ← 下载并重命名模型到这里
├── vosk/                     # Vosk 语音识别库
│   ├── bin/
│   ├── include/
│   └── lib/
├── res/                      # 资源文件
├── vosk_bridge.c            # Vosk 桥接程序源码
├── vosk_bridge.exe           # Vosk 桥接程序
├── main.cpp                  # 程序入口
├── mainwindow.cpp/h          # 主窗口实现
├── appstyle.cpp/h            # 界面样式
├── ffmpegvideoplayer.cpp/h   # FFmpeg 视频播放器
├── electron_low_level.cpp/h  # 底层通信
└── voskrecognizer.cpp/h      # 语音识别
```

## 使用说明

### 连接机器人

1. 通过 USB 线连接 ElectronBot 设备到电脑
2. 点击"连接"按钮
3. 等待连接成功后，状态指示灯变为绿色

### 电机控制

- 使用左侧面板的滑块控制 6 个关节的角度
- 角度范围：-180° 到 +180°
- 实时预览电机角度值

### 摄像头功能

- 点击"开启摄像头"启动摄像头预览
- 注意：开启摄像头时会自动关闭语音识别
- 关闭摄像头后会自动恢复语音识别

### 语音识别

- 语音识别在连接后自动启动
- 支持中文语音命令
- 可通过下拉框选择音频输入设备
- 可调节音量增益滑块

## 常见问题

### Q1: 编译报错"找不到 vosk-model-cn"

**解决方案**：请按照上面的"下载语音模型"步骤下载并放置模型文件

### Q2: 摄像头无法打开

**解决方案**：
1. 检查摄像头是否被其他程序占用
2. 关闭其他使用摄像头的应用（如微信、QQ等）
3. 尝试重新插拔 USB 摄像头

### Q3: 语音识别无响应

**解决方案**：
1. 检查麦克风是否正常工作
2. 确认语音模型已正确放置
3. 检查麦克风权限设置

### Q4: USB 连接失败

**解决方案**：
1. 检查 USB 线是否正常
2. 尝试使用电脑后面的 USB 端口
3. 确认 ElectronBot 设备已上电

## 技术栈

- **GUI 框架**: Qt 5.14
- **视频播放**: FFmpeg (libavcodec, libavformat, libavutil)
- **语音识别**: Vosk
- **USB 通信**: LibUSB-Win32
- **编译器**: MinGW-w64 GCC

## 开发相关

### 添加新的语音模型

如果想使用其他语言或更大/更小的模型：

1. 从 [Vosk Models](https://alphacephei.com/vosk/models) 下载新模型
2. 将模型文件夹重命名为 `vosk-model-cn`
3. 替换 `3rdparty/vosk-models/vosk-model-cn` 文件夹
4. 重新编译项目

### 修改模型路径

如果想使用不同名称的模型，修改以下文件中的路径：

- `mainwindow.cpp` 第 666 行
- `ElectronBotClient.pro` 第 42 行

## 许可证

本项目基于 MIT 许可证开源。

## 致谢

- [Vosk](https://alphacephei.com/vosk) - 语音识别引擎
- [FFmpeg](https://ffmpeg.org/) - 多媒体框架
- [Qt](https://www.qt.io/) - 跨平台 GUI 框架
