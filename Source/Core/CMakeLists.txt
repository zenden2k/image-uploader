if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

set(SRC_LIST Network/NetworkClient.cpp
    Upload/DefaultUploadEngine.cpp
    Upload/FileUploadTask.cpp
    Upload/ScriptUploadEngine.cpp
    Upload/UploadEngine.cpp
    Upload/Uploader.cpp
    Upload/UploadTask.cpp
    Upload/UrlShorteningTask.cpp
    Utils/CoreUtils.cpp
    Utils/CryptoUtils.cpp
    Utils/SimpleXml.cpp
    Utils/StringUtils.cpp
    Utils/ConsoleUtils.cpp
    3rdpart/base64.cpp
    3rdpart/pcreplusplus.cpp
    3rdpart/tinystr.cpp
    3rdpart/tinyxml.cpp
    3rdpart/tinyxmlerror.cpp
    3rdpart/tinyxmlparser.cpp
    Settings/CliSettings.cpp
    OutputCodeGenerator.cpp
    UploadEngineList.cpp
    SettingsManager.cpp
    AppParams.cpp
    Scripting/API/Functions.cpp
    Scripting/API/RegularExpression.cpp
    Scripting/API/ScriptAPI.cpp
    Logging.cpp
    Upload/ConsoleUploadErrorHandler.cpp
    Upload/FileQueueUploader.cpp
    Upload/FileQueueUploaderPrivate.cpp
    Upload/FolderList.cpp
    Upload/ServerProfile.cpp
    Upload/ServerSync.cpp
    Upload/UploadEngineManager.cpp
    Upload/UploadManager.cpp
    Upload/UploadSession.cpp
    Upload/AdvancedUploadEngine.cpp
    Upload/MegaNzUploadEngine.cpp
    ServiceLocator.cpp
    Settings/BasicSettings.cpp
    Logging/MyLogSink.cpp
    Logging/ConsoleLogger.cpp
    Scripting/ScriptsManager.cpp
    Network/CurlShare.cpp
    ThreadSync.cpp
    Scripting/Script.cpp
    Scripting/API/UploadTaskWrappers.cpp
    Scripting/API/GumboBingings/GumboDocument.cpp
    TempFileDeleter.cpp
    Utils/DesktopUtils.cpp
    HistoryManager.cpp
    CoreFunctions.cpp
    3rdpart/GumboQuery/GQDocument.cpp
    3rdpart/GumboQuery/Node.cpp
    3rdpart/GumboQuery/Object.cpp
    3rdpart/GumboQuery/GQ_Parser.cpp
    3rdpart/GumboQuery/QueryUtil.cpp
    3rdpart/GumboQuery/Selection.cpp
    3rdpart/GumboQuery/Selector.cpp
    Settings.cpp
    Scripting/API/Process.cpp
    Upload/Filters/UrlShorteningFilter.cpp
    Upload/Filters/UserFilter.cpp
    LocalFileCache.cpp
    Utils/SystemUtils.cpp
    Settings/EncodedPassword.cpp
    Scripting/API/WebBrowserPrivateBase.cpp
    Utils/GlobalMutex.cpp
    Utils/TextUtils.cpp
    Scripting/UploadFilterScript.cpp
    3rdpart/htmlentities.cpp)

if(WIN32)
    list(APPEND SRC_LIST 
        DefaultProxyProvider.cpp 
        3rdpart/CodePages.cpp 
        Scripting/API/HtmlDocument.cpp
        Scripting/API/COMUtils.cpp
        Scripting/API/HtmlDocumentPrivate_win.cpp
        Scripting/API/HtmlElement.cpp
        Scripting/API/HtmlElementPrivate_win.cpp
        Scripting/API/WebBrowser.cpp
        Scripting/API/WebBrowserPrivate_win.cpp
        ../Func/UpdatePackage.cpp
        ../Func/IuCommonFunctions.cpp
        ../3rdpart/Registry.cpp
        ../3rdpart/Unzipper.cpp
        ../3rdpart/MemberFunctionCallback.cpp
        ../Gui/Dialogs/WebViewWindow.cpp
        ../Gui/Controls/WTLBrowserView.cpp
        ../Func/WinUtils.cpp
        3rdpart/CodePages.cpp
        ../Gui/GuiTools.cpp
        Images/Utils.cpp
    )
endif()

add_library(libiucore STATIC ${SRC_LIST} )

if(MSVC)
    target_compile_options(libiucore PRIVATE "/MP")
endif()

set_target_properties(libiucore PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)