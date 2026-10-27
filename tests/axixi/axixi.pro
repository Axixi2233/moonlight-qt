TEMPLATE = app
TARGET = axixi-regressions
QT += core gui qml testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
DEFINES += SDL_MAIN_HANDLED
INCLUDEPATH += ../../app
SOURCES += tst_axixi.cpp \
    ../../app/settings/streamingpreferences.cpp \
    ../../app/cli/commandlineparser.cpp \
    ../../app/streaming/video/overlaymanager.cpp \
    ../../app/path.cpp \
    ../../app/wm.cpp
HEADERS += ../../app/settings/streamingpreferences.h
RESOURCES += fonts.qrc
win32 {
    INCLUDEPATH += $$PWD/../../libs/windows/include/x64/SDL2
    QMAKE_CXXFLAGS += /utf-8
    LIBS += -L$$PWD/../../libs/windows/lib/x64 -lSDL2 -lSDL2_ttf
}
unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += sdl2 SDL2_ttf
}
