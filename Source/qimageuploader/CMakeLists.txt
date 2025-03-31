cmake_minimum_required(VERSION 3.9.0)

project(qimageuploader)

#if (WIN32)
#   set (CMAKE_PREFIX_PATH "d:\\Qt\\5.15.0\\msvc2019\\")
#endif()*/

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
 
#set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOUIC_SEARCH_PATHS "Gui/ui/")

add_definitions(-DIU_QT)

if (WIN32)
    set(RESOURCE_LIST ../res/qimageuploader.exe.manifest qimageuploader.rc)
    SET(OPTIONS WIN32)
endif()

set(SOURCE_FILES 
    main.cpp
    Gui/MainWindow.cpp
    Gui/FrameGrabberDlg.cpp
    Gui/ResultsWindow.cpp
    Gui/LogWindow.cpp
    Gui/LoginDialog.cpp
    Gui/AboutDialog.cpp
    ../Core/ScreenCapture/ScreenCapture.cpp
    Gui/RegionSelect.cpp
    Gui/models/uploadtreemodel.cpp 
    Gui/controls/MyListWidget.cpp
    QtUploadErrorHandler.cpp
    QtDefaultLogger.cpp
    QtScriptDialogProvider.cpp
    QtScriptFunctionsImpl.cpp
    Gui/controls/ServerSelectorWidget.cpp
    Gui/controls/ProgressRingWidget.cpp
    Gui/controls/UploadTreeView.cpp
    Gui/controls/UploadTreeView.h
    ../Core/Settings/QtGuiSettings.cpp
    ../Core/Settings/CommonGuiSettings.cpp
    ../Core/AbstractServerIconCache.cpp
    ../Core/AbstractServerIconCache.h
    ../Core/QtServerIconCache.cpp
    ../Core/QtServerIconCache.h
    ../Video/QtImage.cpp
    Gui/ui/MainWindow.ui
    Gui/ui/FrameGrabberDlg.ui
    Gui/ui/ResultsWindow.ui
    Gui/ui/LogWindow.ui
    Gui/ui/LoginDialog.ui
    Gui/ui/AboutDialog.ui
    qimageuploader.qrc
)


if(WIN32)
    list(APPEND SOURCE_FILES
        ../Func/GdiPlusInitializer.cpp
    )
endif()

add_executable(qimageuploader ${OPTIONS} ${SOURCE_FILES} ${RESOURCE_LIST})
 
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} gdi32 dwmapi gdiplus)
    if (IU_ENABLE_WEBVIEW2)
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_GLOBAL_WebView2LoaderPreference "Static")
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_USER_PROPS "${CMAKE_SOURCE_DIR}/ImageUploader.Core.props")
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_PACKAGE_REFERENCES "${WEBVIEW2_PACKAGES_REFERENCES}")
    endif()
endif()

target_link_libraries(qimageuploader iucore iuvideo ${COMMON_LIBS_LIST})

if(WIN32)

    add_custom_command(TARGET qimageuploader
        POST_BUILD
        COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/set_binary_version.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" $<TARGET_FILE:qimageuploader> $<CONFIG>
    )
endif()
