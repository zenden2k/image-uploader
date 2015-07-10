TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
#CONFIG += object_parallel_to_source

win32 {
    DEFINES += UNICODE _UNICODE CURL_STATICLIB PCRE_STATIC _WIN32_IE=0x0603 WINVER=0x0601 _WIN32_WINNT=0x0601 _CRT_SECURE_NO_WARNINGS
    QMAKE_CFLAGS_RELEASE += /MT
    QMAKE_CXXFLAGS_RELEASE += /MT
    QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
    QMAKE_CXXFLAGS_WARN_ON -= -w34100 # http://stackoverflow.com/questions/20402722/why-disable-specific-warning-not-working-in-visual-studio
    QMAKE_CXXFLAGS  += /wd4100 /wd4996 #disable msvc specific warnings
} else {
    DEFINES += _FILE_OFFSET_BITS=64
}
DEFINES += __STDC_WANT_LIB_EXT1__=1
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

!win32 {
DEFINES += USE_OPENSSL
}

#VPATH= d:/Develop/imageuploader-1.3.2-vs2013/image-uploader/Source/

SOURCES += main.cpp \
    ../Core/Network/NetworkClient.cpp \
    ../Core/Upload/DefaultUploadEngine.cpp \
#    ../Core/Upload/FileQueueUploader.cpp \
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
    ../Core/Utils/ConsoleUtils.cpp \
    ../Core/3rdpart/base64.cpp \
    ../Core/3rdpart/pcreplusplus.cpp \
    ../Core/3rdpart/tinystr.cpp \
    ../Core/3rdpart/tinyxml.cpp \
    ../Core/3rdpart/tinyxmlerror.cpp \
    ../Core/3rdpart/tinyxmlparser.cpp \
    ../Core/Settings/CliSettings.cpp \
    ../Core/OutputCodeGenerator.cpp \
    ../Core/UploadEngineList.cpp \
    ../Core/SettingsManager.cpp \
    ../Core/AppParams.cpp \
    ../Core/Scripting/API/Functions.cpp \
    ../Core/Scripting/API/RegularExpression.cpp \
    ../Core/Scripting/API/ScriptAPI.cpp \
    ../Core/Logging.cpp \
    ../Core/Upload/ConsoleUploadErrorHandler.cpp \
    ../Core/Upload/FileQueueUploader.cpp \
    ../Core/Upload/FileQueueUploaderPrivate.cpp \
    ../Core/Upload/FolderList.cpp \
    ../Core/Upload/ServerProfile.cpp \
    ../Core/Upload/ServerSync.cpp \
    ../Core/Upload/UploadEngineManager.cpp \
    ../Core/Upload/UploadManager.cpp \
    ../Core/Upload/UploadSession.cpp \
    ../Core/ServiceLocator.cpp \
    ../Core/Settings/BasicSettings.cpp \
    ../Core/Logging/MyLogSink.cpp \
    ../Core/Logging/ConsoleLogger.cpp \
    ../Core/Scripting/ScriptsManager.cpp \
    ../Core/Network/CurlShare.cpp \
    ConsoleScriptDialogProvider.cpp \
    ../Core/ThreadSync.cpp \
    ../Core/Scripting/Script.cpp \
    ../Core/Scripting/API/UploadTaskWrappers.cpp \
    ../Core/Scripting/API/GumboBingings/GumboDocument.cpp \
    ../Core/TempFileDeleter.cpp \
    ../Core/Utils/DesktopUtils.cpp \
    ../Core/HistoryManager.cpp \
    ../Core/CoreFunctions.cpp \
    ../Core/3rdpart/GumboQuery/GQDocument.cpp \
    ../Core/3rdpart/GumboQuery/Node.cpp \
    ../Core/3rdpart/GumboQuery/Object.cpp \
    ../Core/3rdpart/GumboQuery/GQ_Parser.cpp \
    ../Core/3rdpart/GumboQuery/QueryUtil.cpp \
    ../Core/3rdpart/GumboQuery/Selection.cpp \
    ../Core/3rdpart/GumboQuery/Selector.cpp \
    ../Core/Settings.cpp \
    ../Core/Scripting/API/Process.cpp \
    ../Core/Upload/Filters/UrlShorteningFilter.cpp \
    ../Core/Upload/Filters/UserFilter.cpp \
    ../Core/LocalFileCache.cpp \
    ../Core/Utils/SystemUtils.cpp \
    ../Core/Settings/EncodedPassword.cpp \
    ../Core/Scripting/API/WebBrowserPrivateBase.cpp \
    ../Core/Utils/GlobalMutex.cpp \
    ../Core/Utils/TextUtils.cpp \
    ../Core/Scripting/UploadFilterScript.cpp \
    ../Core/3rdpart/htmlentities.cpp \

win32 {
SOURCES += ../Core/Scripting/API/HtmlDocument.cpp \
          ../Core/Scripting/API/COMUtils.cpp \
    ../Core/Scripting/API/HtmlDocumentPrivate_win.cpp \
    ../Core/Scripting/API/HtmlElement.cpp \
    ../Core/Scripting/API/HtmlElementPrivate_win.cpp \
    ../Core/Scripting/API/WebBrowser.cpp \
    ../Core/Scripting/API/WebBrowserPrivate_win.cpp \
    ../Func/UpdatePackage.cpp \
    ../Func/IuCommonFunctions.cpp \
    ../3rdpart/Registry.cpp \
    ../3rdpart/Unzipper.cpp \
    ../3rdpart/MemberFunctionCallback.cpp \
    ../Gui/Dialogs/WebViewWindow.cpp \
    ../Gui/Controls/WTLBrowserView.cpp \
    ../Func/WinUtils.cpp \
    ../Core/3rdpart/CodePages.cpp \
    ../Gui/GuiTools.cpp
	
}

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
    ../Core/3rdpart/utf8.h \
    ../Core/AppParams.h \
    ../Core/Scripting/API/Functions.h \
    ../Core/Scripting/API/RegularExpression.h \
    ../Core/Scripting/API/Scripting/API.h \
    ../Core/Logging.h \
    ../Core/Upload/ConsoleUploadErrorHandler.h \
    ../Core/Upload/FileQueueUploaderPrivate.h \
    ../Core/Upload/FolderList.h \
    ../Core/Upload/ServerProfile.h \
    ../Core/Upload/ServerSync.h \
    ../Core/Upload/UploadEngineManager.h \
    ../Core/Upload/UploadErrorHandler.h \
    ../Core/Upload/UploadFilter.h \
    ../Core/Upload/UploadManager.h \
    ../Core/Upload/UploadResult.h \
    ../Core/Upload/UploadSession.h \
    ../Core/ServiceLocator.h \
    ../Core/Settings/BasicSettings.h \
    ../Core/Settings/CliSettings.h \
    ../Core/Logging/MyLogSink.h \
    ../Core/Logging/ConsoleLogger.h \
    ../Core/Scripting/ScriptsManager.h \
    ../Core/Scripting/Squirrelnc.h \
    ../Core/Network/CurlShare.h \
    ConsoleScriptDialogProvider.h \
    ../Core/ThreadSync.h \
    ../Core/Scripting/API/GumboBingings/GumboDocument.h \
    ../Core/TempFileDeleter.h \
    ../Core/Utils/DesktopUtils.h \
    ../Core/HistoryManager.h \
    ../Core/CoreFunctions.h \
    ../Core/3rdpart/GumboQuery/Document.h \
    ../Core/3rdpart/GumboQuery/Node.h \
    ../Core/3rdpart/GumboQuery/Object.h \
    ../Core/3rdpart/GumboQuery/Parser.h \
    ../Core/3rdpart/GumboQuery/QueryUtil.h \
    ../Core/3rdpart/GumboQuery/Selection.h \
    ../Core/3rdpart/GumboQuery/Selector.h \
    ../Core/Settings.h \
    ../Core/Upload/Filters/UrlShorteningFilter.h \
    ../Core/Upload/Filters/UserFilter.h \
    ../Core/LocalFileCache.h \
    ../Core/Utils/TextUtils.h \
    ConsoleUtils.h

win32 {
HEADERS += \
   ../Core/Scripting/API/COMUtils.h \
    ../Core/Scripting/API/HtmlDocument.h \
    ../Core/Scripting/API/HtmlDocumentPrivate_win.h \
    ../Core/Scripting/API/HtmlElement.h \
    ../Core/Scripting/API/HtmlElementPrivate_win.h \
    ../Core/Scripting/API/WebBrowser.h \
    ../Core/Scripting/API/WebBrowserPrivate_win.h \
    ../Func/UpdatePackage.h \
    ../Func/IuCommonFunctions.h \
    ../3rdpart/Registry.h \
    ../3rdpart/Unzipper.h \
    ../Gui/Dialogs/WebViewWindow.h \
    ../Gui/Controls/WTLBrowserView.h \
    ../Func/WinUtils.h \
    ../Core/3rdpart/codepages.h \
    ../Gui/GuiTools.h
}

INCLUDEPATH += ../

INCLUDEPATH += ../../Contrib/Include/pcre/
INCLUDEPATH += ../../Contrib/Include/squirrel/
INCLUDEPATH += ../../Contrib/Include/sqplus/
INCLUDEPATH += ../../Contrib/Include/WTL/
INCLUDEPATH += ../../Contrib/Include/
INCLUDEPATH += ../../Contrib/Include/gumbo
unix {
#INCLUDEPATH += /usr/include/jsoncpp/
}
win32 {
INCLUDEPATH += ../../Contrib/Include/Libs/
LIBS+=  -L../../Contrib/Source/boost/stage/lib/
INCLUDEPATH += ../../Contrib/Source/boost/
}
DEFINES += TIXML_USE_STL IU_CLI GOOGLE_GLOG_DLL_DECL=

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
#release:LIBS+=  -L../../Contrib/Lib/Release/
debug:LIBS+=  -L../../Contrib/Lib/Debug/
}
unix {
LIBS+=  -L../../Contrib/Lib/Linux/$$ARCH
}
macx {
LIBS+=  -L../../Contrib/Lib/Mac/$$ARCH
}

win32 {
    LIBS+= -llibcurl -llibeay32  -lOleacc -llibglog_static -lminizip
} else {
    LIBS+=   -lcurl   -lssl -lcrypto -lglog  -lpthread -lboost_filesystem -lboost_system -lncurses
}
LIBS+=  -lsquirrel -lsqstdlib  -ljsoncpp -lpcre -lgumbo

#-lZThread

CONFIG += c++11

win32 {
   QMAKE_LFLAGS_CONSOLE = /SUBSYSTEM:CONSOLE,5.01

    DEFINES += _ATL_XP_TARGETING
    QMAKE_CFLAGS += /D _USING_V110_SDK71_
    QMAKE_CXXFLAGS += /D _USING_V110_SDK71_
    LIBS *= -L"%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Lib"
    INCLUDEPATH += "%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Include"
}
win32:OUTDIR = ../../Build/CLI/win32
unix:OUTDIR = ../../Build/CLI/linux/$$ARCH
macx:OUTDIR = ../../Build/CLI/mac/$$ARCH

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
