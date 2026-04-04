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
    appstyle.cpp

HEADERS += \
    mainwindow.h \
    ffmpegvideoplayer.h \
    electron_low_level.h \
    appstyle.h

RESOURCES += \
    resources.qrc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/3rdparty

FFMPEG_PATH = $$PWD/3rdparty

INCLUDEPATH += $${FFMPEG_PATH}/include

win32: LIBS += -L$${FFMPEG_PATH}/lib \
    -lavcodec -lavformat -lavutil -lavfilter -lswscale -lswresample \
    -lws2_32 \
    -lUSBInterface

win32: QMAKE_POST_LINK += copy /Y \"$${FFMPEG_PATH}\\bin\\*.dll\" \"$$OUT_PWD\\\" & copy /Y \"$${FFMPEG_PATH}\\USBInterface.dll\" \"$$OUT_PWD\\\"
