TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

win32 {
    DEFINES += UNICODE _UNICODE CURL_STATICLIB PCRE_STATIC _WIN32_IE=0x0603 WINVER=0x0601 _WIN32_WINNT=0x0601
    QMAKE_CFLAGS_RELEASE += /MT
    QMAKE_CXXFLAGS_RELEASE += /MT
    QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
} else {
	DEFINES += _FILE_OFFSET_BITS=64
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

DEFINES += TIXML_USE_STL USE_OPENSSL


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
    ../Core/3rdpart/CP_RSA.cpp \
    ../Core/AppParams.cpp \
    ../Core/ScriptAPI/Functions.cpp \
    ../Core/ScriptAPI/RegularExpression.cpp \
    ../Core/ScriptAPI/ScriptAPI.cpp \
    ../Core/Logging.cpp

win32 {
SOURCES += ../Core/ScriptAPI/HtmlDocument.cpp \
	  ../Core/ScriptAPI/COMUtils.cpp \
    ../Core/ScriptAPI/HtmlDocumentPrivate_win.cpp \
    ../Core/ScriptAPI/HtmlElement.cpp \
    ../Core/ScriptAPI/HtmlElementPrivate_win.cpp \
    ../Core/ScriptAPI/WebBrowser.cpp \
    ../Core/ScriptAPI/WebBrowserPrivate_win.cpp \
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
    ../Core/ScriptAPI/Functions.h \
    ../Core/ScriptAPI/RegularExpression.h \
    ../Core/ScriptAPI/ScriptAPI.h \
    ../Core/Logging.h

win32 {
HEADERS += \
   ../Core/ScriptAPI/COMUtils.h \
    ../Core/ScriptAPI/HtmlDocument.h \
    ../Core/ScriptAPI/HtmlDocumentPrivate_win.h \
    ../Core/ScriptAPI/HtmlElement.h \
    ../Core/ScriptAPI/HtmlElementPrivate_win.h \
    ../Core/ScriptAPI/WebBrowser.h \
    ../Core/ScriptAPI/WebBrowserPrivate_win.h \
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
unix {
#INCLUDEPATH += /usr/include/jsoncpp/
}
win32 {
INCLUDEPATH += ../../Contrib/Include/Libs/
}
DEFINES += TIXML_USE_STL IU_CLI GOOGLE_GLOG_DLL_DECL=

contains(QMAKE_TARGET.arch, x86_64) {
    DEFINES += _SQ64
    ARCH = amd64
	message("64 bit!!!!")
} else {
    ARCH = i386
}
#_SQ64

#DEPENDPATH += SQUIRREL2/lib
#LIBS+= -L$$PWD/SQUIRREL2/lib/
win32 {
LIBS+=  -L../../Contrib/Lib/Release/
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
    LIBS+=   -lcurl   -lssl -lcrypto -lglog 
}
LIBS+=  -lsqplus  -lsquirrel -lsqstdlib  -ljsoncpp -lpcre

#-lZThread
win32 {
CONFIG += c++11
}
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
