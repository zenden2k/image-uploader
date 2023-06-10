
set(SRC_LIST Network/NetworkClient.cpp
	Network/NetworkClientFactory.cpp
    Upload/DefaultUploadEngine.cpp
    Upload/FileUploadTask.cpp
    Upload/ScriptUploadEngine.cpp
    Upload/UploadEngine.cpp
    Upload/Uploader.cpp
    Upload/UploadTask.cpp
    Upload/AuthTask.cpp
    Upload/UrlShorteningTask.cpp
    Upload/TestConnectionTask.cpp
    Upload/FolderTask.cpp
    Utils/CoreUtils.cpp
    Utils/CryptoUtils.cpp
    Utils/SimpleXml.cpp
    Utils/StringUtils.cpp
    Utils/ConsoleUtils.cpp
    3rdpart/pcreplusplus.cpp
    3rdpart/tinystr.cpp
    3rdpart/tinyxml.cpp
    3rdpart/tinyxmlerror.cpp
    3rdpart/tinyxmlparser.cpp
    OutputCodeGenerator.cpp
    UploadEngineList.cpp
    SettingsManager.cpp
    AppParams.cpp
    Scripting/API/Functions.cpp
    Scripting/API/RegularExpression.cpp
    Scripting/API/ScriptAPI.cpp
    Scripting/API/WebServer.cpp
    Logging.cpp
	DownloadTask.cpp
    Upload/ConsoleUploadErrorHandler.cpp
    Upload/FileQueueUploader.cpp
    Upload/FileQueueUploaderPrivate.cpp
    Upload/FolderList.cpp
    Upload/ServerProfile.cpp
    Upload/ServerProfileGroup.cpp
    Upload/ServerSync.cpp
    Upload/UploadEngineManager.cpp
    Upload/UploadManager.cpp
    Upload/UploadSession.cpp
    Upload/AdvancedUploadEngine.cpp
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
    3rdpart/htmlentities.cpp
    Video/VideoUtils.cpp
    Images/AbstractImage.cpp
)

set(HEADER_LIST Network/NetworkClient.h
	Network/NetworkClientFactory.h
    Upload/DefaultUploadEngine.h
    Upload/FileUploadTask.h
    Upload/ScriptUploadEngine.h
    Upload/UploadEngine.h
    Upload/Uploader.h
    Upload/UploadTask.h
    Upload/AuthTask.h
    Upload/TestConnectionTask.h
    Upload/UrlShorteningTask.h
    Upload/FolderTask.h
    Utils/CoreUtils.h
    Utils/CryptoUtils.h
    Utils/SimpleXml.h
    Utils/StringUtils.h
    Utils/ConsoleUtils.h
    Utils/IOException.h
    3rdpart/pcreplusplus.h
    3rdpart/tinystr.h
    3rdpart/tinyxml.h
    3rdpart/utf8.h
    3rdpart/UriParser.h
    OutputCodeGenerator.h
    UploadEngineList.h
    SettingsManager.h
    AppParams.h
    Scripting/API/Functions.h
    Scripting/API/RegularExpression.h
    Scripting/API/ScriptAPI.h
    Scripting/API/WebServer.h
    Logging.h
    Upload/ConsoleUploadErrorHandler.h
    Upload/FileQueueUploader.h
    Upload/FileQueueUploaderPrivate.h
    Upload/FolderList.h
    Upload/ServerProfile.h
    Upload/ServerProfileGroup.h
    Upload/ServerSync.h
    Upload/UploadEngineManager.h
    Upload/UploadManager.h
    Upload/UploadSession.h
    Upload/AdvancedUploadEngine.h
    ServiceLocator.h
    Settings/BasicSettings.h
    Logging/MyLogSink.h
    Logging/ConsoleLogger.h
    Scripting/ScriptsManager.h
    Network/CurlShare.h
    ThreadSync.h
    Scripting/Script.h
    Scripting/API/UploadTaskWrappers.h
    Scripting/API/GumboBingings/GumboDocument.h
    TempFileDeleter.h
    Utils/DesktopUtils.h
    HistoryManager.h
    CoreFunctions.h
    3rdpart/GumboQuery/Document.h
    3rdpart/GumboQuery/Node.h
    3rdpart/GumboQuery/Object.h
    3rdpart/GumboQuery/Parser.h
    3rdpart/GumboQuery/QueryUtil.h
    3rdpart/GumboQuery/Selection.h
    3rdpart/GumboQuery/Selector.h
    Scripting/API/Process.h
    Upload/Filters/UrlShorteningFilter.h
    Upload/Filters/UserFilter.h
    LocalFileCache.h
    Utils/SystemUtils.h
    Settings/EncodedPassword.h
    Scripting/API/WebBrowserPrivateBase.h
    Utils/GlobalMutex.h
    Utils/TextUtils.h
    Scripting/UploadFilterScript.h
    3rdpart/htmlentities.h
    Video/VideoUtils.h
    i18n/Translator.h
)
    
if(WIN32)
    list(APPEND SRC_LIST 
        Utils/Utils_win.cpp
        Utils/CryptoUtils_win.cpp
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
        ../Gui/GuiTools.cpp
        Images/Utils.cpp
        Images/GdiplusImageReader.cpp
        Images/WebpImageReader.cpp
        Images/ImageLoader.cpp
        ../3rdpart/QColorQuantizer.cpp
        Images/GdiPlusImage.cpp
        ../Func/GdiPlusInitializer.cpp
    )
    
    if (IU_ENABLE_WEBVIEW2)
        list(APPEND SRC_LIST  ../Gui/Controls/Webview2BrowserView.cpp)
    endif()

    list(APPEND HEADER_LIST 
        DefaultProxyProvider.h 
        3rdpart/CodePages.h 
        Scripting/API/HtmlDocument.h
        Scripting/API/COMUtils.h
        Scripting/API/HtmlDocumentPrivate_win.h
        Scripting/API/HtmlElement.h
        Scripting/API/HtmlElementPrivate_win.h
        Scripting/API/WebBrowser.h
        Scripting/API/WebBrowserPrivate_win.h
        ../Func/UpdatePackage.h
        ../Func/IuCommonFunctions.h
        ../3rdpart/Registry.h
        ../3rdpart/Unzipper.h
        ../3rdpart/MemberFunctionCallback.h
        ../Gui/Dialogs/WebViewWindow.h
        ../Gui/Controls/WTLBrowserView.h
        ../Func/WinUtils.h
        ../Gui/GuiTools.h
        Images/Utils.h
        Images/GdiplusImageReader.h
        Images/WebpImageReader.h
        Images/AbstractImageReader.h
        Images/ImageLoader.h
        ../3rdpart/QColorQuantizer.h
    )
else()
    list(APPEND SRC_LIST  
        Utils/Utils_unix.cpp 
        Utils/CryptoUtils_unix.cpp)
endif()

if(IU_ENABLE_MEGANZ)
    list(APPEND SRC_LIST Upload/MegaNzUploadEngine.cpp)
    list(APPEND HEADER_LIST Upload/MegaNzUploadEngine.h)
endif() 

source_group(TREE "${CMAKE_SOURCE_DIR}" PREFIX "Sources" FILES ${SRC_LIST} ${HEADER_LIST})

add_library(iucore STATIC ${SRC_LIST} ${HEADER_LIST})

target_link_libraries(iucore PUBLIC
    CURL::libcurl
    Boost::Boost 
    uriparser::uriparser 
    ZLIB::ZLIB 
    jsoncpp::jsoncpp 
    glog::glog 
    PCRE::PCRE
    squirrel::squirrel 
    squirrel::sqstdlib 
    gumbo-parser::gumbo-parser 
    base64::base64
    SQLite::SQLite3
    ${COMMON_LIBS_LIST}
)

if (IU_ENABLE_MEGANZ)
    target_link_libraries(iucore PUBLIC megaio::megaio
    #cryptopp::cryptopp-static 
    #c-ares::c-ares
    )
endif()

if (IU_USE_OPENSSL)
    target_link_libraries(iucore PUBLIC OpenSSL::OpenSSL)
endif()

if (IU_ENABLE_SFTP)
    #target_link_libraries(iucore PUBLIC Libssh2::Libssh2)
	#list(APPEND COMMON_LIBS_LIST )
endif()

if (WIN32)
    target_link_libraries(iucore PUBLIC minizip::minizip WebP::webpdemux WebP::webpdecoder gdiplus winmm urlmon)
endif()

if (IU_ENABLE_WEBVIEW2)
#    SUPPORTS ONLY DYNAMIC LIBRARIES :(
#    find_program(NUGET_EXE NAMES nuget)
#    if(NOT NUGET_EXE)
#        message("NUGET.EXE not found.")
#        message(FATAL_ERROR "Please install this executable, and run CMake again.")
#    endif()

#    exec_program(${NUGET_EXE} ARGS install "Microsoft.Web.WebView2" -Version 1.0.1774.30 -ExcludeVersion -OutputDirectory ${CMAKE_BINARY_DIR}/packages RETURN_VALUE varrr)
#    message("Var")
#    message(${varrr})
#    exec_program(${NUGET_EXE} ARGS install "Microsoft.Windows.ImplementationLibrary" -Version 1.0.230411.1 -ExcludeVersion -OutputDirectory ${CMAKE_BINARY_DIR}/packages) 
#    target_link_libraries(iucore PUBLIC ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2/build/native/Microsoft.Web.WebView2.targets)
#    target_link_libraries(iucore PUBLIC ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary/build/native/Microsoft.Windows.ImplementationLibrary.targets)
    target_link_libraries(iucore PUBLIC WebView2LoaderStatic.lib version.lib)
endif()

if(MSVC)
    target_compile_options(iucore PRIVATE "/MP")
endif()

set_target_properties(iucore PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
