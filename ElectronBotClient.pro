QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = ElectronBotClient
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ffmpegvideoplayer.cpp \
    electron_low_level.cpp \
    libusb_wrapper.cpp \
    appstyle.cpp \
    voskrecognizer.cpp

HEADERS += \
    mainwindow.h \
    ffmpegvideoplayer.h \
    electron_low_level.h \
    libusb_wrapper.h \
    appstyle.h \
    voskrecognizer.h

RESOURCES += \
    resources.qrc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/3rdparty

FFMPEG_PATH = $$PWD/3rdparty
LIBUSBWIN32_PATH = $$PWD/3rdparty/libusb-win32/libusb-win32-bin-1.4.0.2

INCLUDEPATH += $${FFMPEG_PATH}/include
INCLUDEPATH += $${LIBUSBWIN32_PATH}/include

VOSK_PATH = $$PWD/3rdparty/vosk
VOSK_MODEL_PATH = $$PWD/3rdparty/vosk-models/vosk-model-cn-0.22

win32: LIBS += -L$${FFMPEG_PATH}/lib \
    -lavcodec -lavformat -lavutil -lavfilter -lswscale -lswresample \
    -lws2_32 \
    -L$${LIBUSBWIN32_PATH}/bin/amd64 \
    -lusb

VOSK_BRIDGE_SRC = $$PWD/vosk_bridge.c
VOSK_BRIDGE_EXE = $$PWD/vosk_bridge.exe

win32-g++ {
    vosk_bridge_compiler.commands = gcc -o $$shell_path($${VOSK_BRIDGE_EXE}) $$shell_path($${VOSK_BRIDGE_SRC}) -static-libgcc -fno-exceptions -s
    vosk_bridge_compiler.input = VOSK_BRIDGE_SRC
    vosk_bridge_compiler.output = VOSK_BRIDGE_EXE
    vosk_bridge_compiler.CONFIG += target_predeps no_link
    vosk_bridge_compiler.name = Compiling vosk_bridge.exe
    QMAKE_EXTRA_COMPILERS += vosk_bridge_compiler
}

win32: QMAKE_POST_LINK += \
    copy /Y \"$${FFMPEG_PATH}\\bin\\*.dll\" \"$$OUT_PWD\\debug\\\" & \
    copy /Y \"$${LIBUSBWIN32_PATH}\\bin\\amd64\\libusb0.dll\" \"$$OUT_PWD\\debug\\\" & \
    if not exist \"$$OUT_PWD\\debug\\vosk\" mkdir \"$$OUT_PWD\\debug\\vosk\" & \
    copy /Y \"$${VOSK_PATH}\\bin\\*.dll\" \"$$OUT_PWD\\debug\\vosk\\\" & \
    copy /Y \"$$PWD\\vosk_bridge.exe\" \"$$OUT_PWD\\debug\\vosk\\\" & \
    if not exist \"$$OUT_PWD\\debug\\vosk-models\\vosk-model-cn-0.22\" mkdir \"$$OUT_PWD\\debug\\vosk-models\\vosk-model-cn-0.22\" & \
    xcopy /Y /E \"$${VOSK_MODEL_PATH}\\*\" \"$$OUT_PWD\\debug\\vosk-models\\vosk-model-cn-0.22\\\"
