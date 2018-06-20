#-------------------------------------------------
#
# Project created by QtCreator 2010-12-16T21:57:47
#
#-------------------------------------------------

QT       += widgets webkitwidgets core gui webkit network
#DEFINES += QT_STATIC
TARGET = QtIUHelper
TEMPLATE = app
CONFIG += static

SOURCES += main.cpp\
        mainwindow.cpp \
    qhelpernam.cpp \
    loginparamsform.cpp

HEADERS  += mainwindow.h \
    qhelpernam.h \
    loginparamsform.h

FORMS    += mainwindow.ui \
    loginparamsform.ui

win32 {
   QMAKE_LFLAGS_CONSOLE = /SUBSYSTEM:CONSOLE,5.01
   QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01

    DEFINES += _ATL_XP_TARGETING
    QMAKE_CFLAGS += /D _USING_V110_SDK71_
    QMAKE_CXXFLAGS += /D _USING_V110_SDK71_
    LIBS *= -L"%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Lib"
    INCLUDEPATH += "%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Include"
}

