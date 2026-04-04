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
    appstyle.cpp

HEADERS += \
    mainwindow.h \
    ffmpegvideoplayer.h \
    electron_low_level.h \
    libusb_wrapper.h \
    appstyle.h

RESOURCES += \
    resources.qrc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/3rdparty

FFMPEG_PATH = $$PWD/3rdparty
LIBUSBWIN32_PATH = $$PWD/3rdparty/libusb-win32/libusb-win32-bin-1.4.0.2

INCLUDEPATH += $${FFMPEG_PATH}/include
INCLUDEPATH += $${LIBUSBWIN32_PATH}/include

win32: LIBS += -L$${FFMPEG_PATH}/lib \
    -lavcodec -lavformat -lavutil -lavfilter -lswscale -lswresample \
    -lws2_32 \
    -L$${LIBUSBWIN32_PATH}/bin/amd64 \
    -lusb

win32: QMAKE_POST_LINK += copy /Y \"$${FFMPEG_PATH}\\bin\\*.dll\" \"$$OUT_PWD\\\" & copy /Y \"$${LIBUSBWIN32_PATH}\\bin\\amd64\\libusb0.dll\" \"$$OUT_PWD\\\"
