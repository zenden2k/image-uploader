#-------------------------------------------------
#
# Project created by QtCreator 2012-12-15T09:51:21
#
#-------------------------------------------------

QT       += core gui webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qimageuploader
TEMPLATE = app

DEFINES += TIXML_USE_STL
DEFINES += PCRE_STATIC

SOURCES += main.cpp\
        Gui/mainwindow.cpp \
    ../Core/Network/NetworkClient.cpp \
    ../Core/Utils/StringUtils.cpp \
    ../Core/Utils/SimpleXml.cpp \
    ../Core/Utils/GlobalMutex.cpp \
    ../Core/Utils/CryptoUtils.cpp \
    ../Core/Utils/CoreUtils.cpp \
    ../Core/3rdpart/tinyxmlparser.cpp \
    ../Core/3rdpart/tinyxmlerror.cpp \
    ../Core/3rdpart/tinyxml.cpp \
    ../Core/3rdpart/tinystr.cpp \
    ../Core/3rdpart/pcreplusplus.cpp \
    ../Core/3rdpart/parser.cpp \
    ../Core/3rdpart/CodePages.cpp \
    ../Core/3rdpart/base64.cpp \
    ../Core/Upload/Uploader.cpp \
    ../Core/Upload/UploadEngine.cpp \
    ../Core/Upload/DefaultUploadEngine.cpp \
    ../Core/UploadEngineList.cpp \
    ../Core/Video/AbstractFrameGrabber.cpp \
    ../Core/Video/AbstractVideoFrame.cpp \
    ../Core/Video/DirectshowFrameGrabber.cpp \
    ../Core/Video/VideoGrabber.cpp \
    ../Core/Video/AvcodecFrameGrabber.cpp \
    Gui/FrameGrabberDlg.cpp \
    ../Core/ScreenCapture/ScreenCapture.cpp \
    Gui/RegionSelect.cpp \
    Gui/models/uploadtreemodel.cpp \
    ../Core/Upload/FileQueueUploader.cpp \
    ../Core/Video/AbstractImage.cpp \
    ../Core/Video/QtImage.cpp \
    ../Core/Upload/FileQueueUploaderPrivate.cpp \
    ../Core/Upload/FileUploadTask.cpp \
    ../Core/Upload/FolderList.cpp \
    ../Core/Upload/ScriptUploadEngine.cpp \
    ../Core/Upload/ServerProfile.cpp \
    ../Core/Upload/ServerSync.cpp \
    ../Core/Upload/UploadEngineManager.cpp \
    ../Core/Upload/UploadFilter.cpp \
    ../Core/Upload/UploadManager.cpp \
    ../Core/Upload/UploadSession.cpp \
    ../Core/Upload/UploadTask.cpp \
    ../Core/Upload/UrlShorteningTask.cpp \
    ../Core/Upload/Filters/UrlShorteningFilter.cpp \
    ../Core/Settings/QtGuiSettings.cpp \
    ../Core/ServiceLocator.cpp \
    ../Core/Network/CurlShare.cpp \
    ../Core/Video/DirectShowUtil.cpp \
    ../Core/Scripting/API/COMUtils.cpp \
    ../Core/Scripting/API/Functions.cpp \
    ../Core/Scripting/API/Process.cpp \
    ../Core/Scripting/API/RegularExpression.cpp \
    ../Core/Scripting/API/ScriptAPI.cpp \
    ../Core/Scripting/API/UploadTaskWrappers.cpp \
    ../Core/Scripting/Script.cpp \
    ../Core/Scripting/ScriptsManager.cpp \
    ../Core/Scripting/UploadFilterScript.cpp \
    ../Core/ThreadSync.cpp \
    ../Core/Utils/TextUtils.cpp \
    ../Core/3rdpart/htmlentities.cpp \
    ../3rdpart/Registry.cpp \
    ../Func/WinUtils.cpp \
    ../Core/AppParams.cpp \
    ../Core/TempFileDeleter.cpp \
    ../Core/Upload/Filters/UserFilter.cpp \
    ../Core/HistoryManager.cpp \
    ../Core/3rdpart/dxerr.cpp \
    ../Core/LocalFileCache.cpp \
    ../Core/Settings/BasicSettings.cpp \
    ../Core/Settings/EncodedPassword.cpp \
    ../Core/Settings/CommonGuiSettings.cpp \
    ../Core/SettingsManager.cpp \
    ../Core/Utils/DesktopUtils.cpp

HEADERS  += Gui/mainwindow.h \
    ../Core/Network/NetworkClient.h \
    ../Core/Utils/utils_win.h \
    ../Core/Utils/utils_unix.h \
    ../Core/Utils/StringUtils.h \
    ../Core/Utils/SimpleXml.h \
    ../Core/Utils/GlobalMutex.h \
    ../Core/Utils/CryptoUtils.h \
    ../Core/Utils/CoreUtils.h \
    ../Core/Utils/CoreTypes.h \
    ../Core/3rdpart/utf8.h \
    ../Core/3rdpart/tinyxml.h \
    ../Core/3rdpart/tinystr.h \
    ../Core/3rdpart/pstdint.h \
    ../Core/3rdpart/pcreplusplus.h \
    ../Core/3rdpart/parser.h \
    ../Core/3rdpart/FastDelegateBind.h \
    ../Core/3rdpart/FastDelegate.h \
    ../Core/3rdpart/CP_RSA.h \
    ../Core/3rdpart/codepages.h \
    ../Core/3rdpart/base64.h \
    ../Core/3rdpart/utf8/unchecked.h \
    ../Core/3rdpart/utf8/core.h \
    ../Core/3rdpart/utf8/checked.h \
    ../Core/Upload/Uploader.h \
    ../Core/Upload/UploadEngine.h \
    ../Core/Upload/FileQueueUploader.h \
    ../Core/Upload/DefaultUploadEngine.h \
    ../Core/Upload/CommonTypes.h \
    ../Core/UploadEngineList.h \
    ../Core/Video/AbstractFrameGrabber.h \
    ../Core/Video/AbstractVideoFrame.h \
    ../Core/Video/DirectshowFrameGrabber.h \
    ../Core/Video/VideoGrabber.h \
    ../Core/Video/AvcodecFrameGrabber.h \
    Gui/FrameGrabberDlg.h \
    Gui/models/uploadtreemodel.h \
    ../Core/Upload/UploadObjectQueue.h \
    ../Core/Upload/UploadObjectHandler.h \
    ../Core/Upload/UploadObject.h \
    ../Core/Upload/QueueUploader.h \
    ../Core/Video/AbstractImage.h \
    ../Core/Video/QtImage.h \
    ../Core/Upload/FileQueueUploaderPrivate.h \
    ../Core/Upload/FileUploadTask.h \
    ../Core/Upload/FolderList.h \
    ../Core/Upload/ScriptUploadEngine.h \
    ../Core/Upload/ServerProfile.h \
    ../Core/Upload/ServerSync.h \
    ../Core/Upload/UploadEngineManager.h \
    ../Core/Upload/UploadFilter.h \
    ../Core/Upload/UploadManager.h \
    ../Core/Upload/UploadResult.h \
    ../Core/Upload/UploadSession.h \
    ../Core/Upload/UploadTask.h \
    ../Core/Upload/UrlShorteningTask.h \
    ../Core/Upload/Filters/ImageConverterFilter.h \
    ../Core/Upload/Filters/UrlShorteningFilter.h \
    ../Core/ServiceLocator.h \
    ../Core/Network/CurlShare.h \
    ../Core/Video/DirectShowUtil.h \
    ../Core/Scripting/API/COMUtils.h \
    ../Core/Scripting/API/Functions.h \
    ../Core/Scripting/API/HtmlDocument.h \
    ../Core/Scripting/API/HtmlDocumentPrivate_win.h \
    ../Core/Scripting/API/HtmlElement.h \
    ../Core/Scripting/API/HtmlElementPrivate_win.h \
    ../Core/Scripting/API/Process.h \
    ../Core/Scripting/API/RegularExpression.h \
    ../Core/Scripting/API/ScriptAPI.h \
    ../Core/Scripting/API/UploadTaskWrappers.h \
    ../Core/Scripting/API/WebBrowser.h \
    ../Core/Scripting/API/WebBrowserPrivate_win.h \
    ../Core/Scripting/API/WebBrowserPrivateBase.h \
    ../Core/Scripting/Script.h \
    ../Core/Scripting/ScriptsManager.h \
    ../Core/Scripting/Squirrelnc.h \
    ../Core/Scripting/UploadFilterScript.h \
    ../Core/ThreadSync.h \
    ../Core/Utils/TextUtils.h \
    ../Core/3rdpart/htmlentities.h \
    ../3rdpart/Registry.h \
    ../Func/WinUtils.h \
    ../Core/AppParams.h \
    ../Core/TempFileDeleter.h \
    ../Core/Upload/Filters/UserFilter.h \
    ../Core/HistoryManager.h \
    ../Core/3rdpart/dxerr.h \
    ../Core/LocalFileCache.h \
    ../Core/Settings/BasicSettings.h \
    ../Core/Settings/QtGuiSettings.h \
    ../Core/Settings/EncodedPassword.h \
    ../Core/Settings/CommonGuiSettings.h

FORMS    += Gui/ui/mainwindow.ui \
    Gui/ui/FrameGrabberDlg.ui

INCLUDEPATH += $$_PRO_FILE_PWD_/../
#INCLUDEPATH += $$_PRO_FILE_PWD_/libs/include

INCLUDEPATH += $$_PRO_FILE_PWD_/

win32 {
    DEFINES += UNICODE _UNICODE CURL_STATICLIB PCRE_STATIC _WIN32_IE=0x0603 WINVER=0x0601 _WIN32_WINNT=0x0601
    QMAKE_CFLAGS_RELEASE += /MT
    QMAKE_CXXFLAGS_RELEASE += /MT
    QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
} else {
	 DEFINES +=  USE_OPENSSL _FILE_OFFSET_BITS=64
}
unix:{
eval(QMAKE_TARGET.arch = ""):{
# QMAKE_TARGET.arch isn't set properly on Linux.
# If we get a bitset-specific mkspec, use it
linux-g++-32:QMAKE_TARGET.arch = x86
linux-g++-64:QMAKE_TARGET.arch = x86_64

# If we get a generic one, then determine the
# arch of the machine and assign
linux-g++:{
  ARCH = $$system(uname -m) # i686 or x86_64
  contains(ARCH, x86_64):{
    QMAKE_TARGET.arch = x86_64
  }
  else{
    QMAKE_TARGET.arch = x86
  }
} # linux-g++
} # eval
} # unix

DEFINES += TIXML_USE_STL


INCLUDEPATH += ../

INCLUDEPATH += ../../Contrib/Include/pcre/
INCLUDEPATH += ../../Contrib/Include/squirrel/
INCLUDEPATH += ../../Contrib/Include/sqplus/
INCLUDEPATH += ../../Contrib/Include/WTL/
INCLUDEPATH += ../../Contrib/Include/
unix {
#INCLUDEPATH += /usr/include/jsoncpp/
}
win32 {
INCLUDEPATH += ../../Contrib/Include/Libs/
}
DEFINES += TIXML_USE_STL IU_QT GOOGLE_GLOG_DLL_DECL=

contains(QMAKE_TARGET.arch, x86_64) {
    DEFINES += _SQ64
    ARCH = amd64
} else {
    ARCH = i386
}
#_SQ64

#DEPENDPATH += SQUIRREL2/lib
#LIBS+= -L$$PWD/SQUIRREL2/lib/
win32 {
LIBS+=  -L../../Contrib/Lib/Release/
LIBS+=  -L../../Contrib/Lib/
LIBS+=  -L../../Contrib/Source/boost/stage/lib/
}
unix {
LIBS+=  -L../../Contrib/Lib/Linux/$$ARCH
}
macx {
LIBS+=  -L../../Contrib/Lib/Mac/$$ARCH
}

win32 {
    LIBS+= -llibcurl -llibeay32  -lOleacc -llibglog_static -lminizip -lWs2_32
} else {
    LIBS+=   -lcurl   -lssl -lcrypto -lglog
}
LIBS+=  -lsqplus  -lsquirrel -lsqstdlib  -ljsoncpp -lpcre


win32 {
INCLUDEPATH += $$_PRO_FILE_PWD_/../../Contrib/include/BaseClasses
INCLUDEPATH += $$_PRO_FILE_PWD_/../../Contrib/include/DX
INCLUDEPATH += $$_PRO_FILE_PWD_/../../Contrib/Source/boost/
LIBS += -lstrmbase -lgdi32
}

LIBS += -lavcodec -lswresample -lavformat -lavutil -lswscale


CONFIG += c++11

win32 {
   QMAKE_LFLAGS_CONSOLE = /SUBSYSTEM:CONSOLE,5.01

    DEFINES += _ATL_XP_TARGETING
    QMAKE_CFLAGS += /D _USING_V110_SDK71_
    QMAKE_CXXFLAGS += /D _USING_V110_SDK71_
    LIBS *= -L"%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Lib"
    INCLUDEPATH += "%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Include"
}
win32:OUTDIR = ../../Build/qimageuploader/win32
unix:OUTDIR = ../../Build/qimageuploader/linux/$$ARCH
macx:OUTDIR = ../../Build/qimageuploader/mac/$$ARCH

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

win32 {
    #QMAKE_LFLAGS_RELEASE+=/DELAYLOAD:Qt5WebKitWidgets.dll
}

RESOURCES += \
    qimageuploader.qrc

#include(3rdparty/qtdotnetstyle.pri)
