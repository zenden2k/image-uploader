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

find_package(Qt6 COMPONENTS Widgets REQUIRED)
#find_package(Qt6Core REQUIRED)
#find_package(Qt6Gui REQUIRED)
#find_package(Qt6Widgets REQUIRED)

set(CMAKE_AUTOUIC_SEARCH_PATHS "Gui/ui/")

add_definitions(-DIU_QT)

if (WIN32)
    set(RESOURCE_LIST ../res/qimageuploader.exe.manifest qimageuploader.rc)
    SET(OPTIONS WIN32)
endif()

set(SOURCE_FILES 
    main.cpp
    Gui/mainwindow.cpp
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
    ../Core/Settings/QtGuiSettings.cpp
    ../Video/QtImage.cpp
    Gui/ui/mainwindow.ui 
    Gui/ui/FrameGrabberDlg.ui
    Gui/ui/ResultsWindow.ui
    Gui/ui/LogWindow.ui
    Gui/ui/LoginDialog.ui
	Gui/ui/AboutDialog.ui
    qimageuploader.qrc
)

add_executable(qimageuploader ${OPTIONS} ${SOURCE_FILES} ${RESOURCE_LIST})
 
target_link_libraries(${PROJECT_NAME} Qt6::Core)
target_link_libraries(${PROJECT_NAME} Qt6::Gui)
target_link_libraries(${PROJECT_NAME} Qt6::Widgets)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} gdi32 dwmapi)
endif()

target_link_libraries(qimageuploader iucore iuvideo ${COMMON_LIBS_LIST})

if(WIN32)
    add_custom_command(TARGET qimageuploader
        POST_BUILD
        COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/set_binary_version.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" $<TARGET_FILE:qimageuploader> $<CONFIG>
    )
endif()
