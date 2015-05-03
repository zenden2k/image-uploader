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

