TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../Core/Network/NetworkManager.cpp \
    ../Core/Upload/DefaultUploadEngine.cpp \
    ../Core/Upload/FileQueueUploader.cpp \
    ../Core/Upload/FileUploadTask.cpp \
    ../Core/Upload/ScriptUploadEngine.cpp \
    ../Core/Upload/UploadEngine.cpp \
    ../Core/Upload/Uploader.cpp \
    ../Core/Upload/UploadTask.cpp \
    ../Core/Upload/UrlShorteningTask.cpp \
    ../Core/Utils/CoreUtils.cpp \
    ../Core/Utils/CryptoUtils.cpp \
    ../Core/Utils/SimpleXml.cpp \
    ../Core/Utils/StringUtils.cpp \
    ../Core/3rdpart/base64.cpp \
    ../Core/3rdpart/pcreplusplus.cpp \
    ../Core/3rdpart/tinystr.cpp \
    ../Core/3rdpart/tinyxml.cpp \
    ../Core/3rdpart/tinyxmlerror.cpp \
    ../Core/3rdpart/tinyxmlparser.cpp \
    ../Func/Settings.cpp \
    ../Core/OutputCodeGenerator.cpp \
    ../Core/UploadEngineList.cpp \
    ../Core/SettingsManager.cpp \
    ../Core/3rdpart/CP_RSA.cpp

HEADERS += \
    ../Core/Upload/CommonTypes.h \
    ../Core/Upload/DefaultUploadEngine.h \
    ../Core/Upload/FileQueueUploader.h \
    ../Core/Upload/FileUploadTask.h \
    ../Core/Upload/ScriptUploadEngine.h \
    ../Core/Upload/UploadEngine.h \
    ../Core/Upload/Uploader.h \
    ../Core/Upload/UploadTask.h \
    ../Core/Upload/UrlShorteningTask.h \
    ../Core/Utils/CoreTypes.h \
    ../Core/Utils/CoreUtils.h \
    ../Core/Utils/CryptoUtils.h \
    ../Core/Utils/SimpleXml.h \
    ../Core/Utils/StringUtils.h \
    ../Core/Utils/utils_unix.h \
    ../Core/3rdpart/base64.h \
    ../Core/3rdpart/FastDelegate.h \
    ../Core/3rdpart/FastDelegateBind.h \
    ../Core/3rdpart/parser.h \
    ../Core/3rdpart/pcreplusplus.h \
    ../Core/3rdpart/pstdint.h \
    ../Core/3rdpart/tinystr.h \
    ../Core/3rdpart/tinyxml.h \
    ../Core/3rdpart/utf8.h

INCLUDEPATH += ../
INCLUDEPATH += SQUIRREL2/include
INCLUDEPATH += SQUIRREL2/sqplus/

DEFINES += TIXML_USE_STL IU_CLI

#DEPENDPATH += SQUIRREL2/lib
LIBS+= -lcurl -lpcre  -lssl -lcrypto -lZThread -L$$PWD/SQUIRREL2/lib/ -lsqplus -lsqstdlib -lsquirrel

CONFIG += c++11


win32:OUTDIR = ../../Build/
unix:OUTDIR = ../../Build//linux
macx:OUTDIR = ../../Build/mac

debug:DESTDIR = $$OUTDIR/debug/executable
debug:OBJECTS_DIR = $$OUTDIR/debug/temp
debug:MOC_DIR = $$OUTDIR/debug/temp
debug:RCC_DIR = $$OUTDIR/debug/temp
debug:UI_DIR = $$OUTDIR/debug/temp

release:DESTDIR = $$OUTDIR/release/executable
release:OBJECTS_DIR = $$OUTDIR/release/temp
release:MOC_DIR = $$OUTDIR/release/temp
release:RCC_DIR = $$OUTDIR/release/temp
release:UI_DIR = $$OUTDIR/release/temp

TARGET = imgupload
