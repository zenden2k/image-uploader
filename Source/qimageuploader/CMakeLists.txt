cmake_minimum_required(VERSION 3.9.0)

project(qimageuploader)

if (WIN32)
    set (CMAKE_PREFIX_PATH "d:\\Qt\\5.11.2\\msvc2015\\")
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
 
set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOUIC_SEARCH_PATHS "Gui/ui/")

add_definitions(-DIU_QT)

set(SOURCE_FILES 
    main.cpp
    Gui/mainwindow.cpp
    ../Core/Video/AbstractFrameGrabber.cpp
    ../Core/Video/AbstractVideoFrame.cpp
    ../Core/Video/VideoGrabber.cpp
    ../Core/Video/AvcodecFrameGrabber.cpp
    Gui/FrameGrabberDlg.cpp
    Gui/ResultsWindow.cpp
    Gui/LogWindow.cpp
    Gui/LoginDialog.cpp
    ../Core/ScreenCapture/ScreenCapture.cpp
    Gui/RegionSelect.cpp
    Gui/models/uploadtreemodel.cpp 
    Gui/controls/MyListWidget.cpp
    ../Core/Video/AbstractImage.cpp 
    ../Core/Video/QtImage.cpp
    QtUploadErrorHandler.cpp
    QtDefaultLogger.cpp
    QtScriptDialogProvider.cpp
    Gui/controls/ServerSelectorWidget.cpp
    ../Core/Settings/QtGuiSettings.cpp
    Gui/ui/mainwindow.ui 
    Gui/ui/FrameGrabberDlg.ui
    Gui/ui/ResultsWindow.ui
    Gui/ui/LogWindow.ui
    Gui/ui/LoginDialog.ui
    qimageuploader.qrc
)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/BaseClasses/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/DX/)
    SET(OPTIONS WIN32)
    list(APPEND SOURCE_FILES 
        qimageuploader.rc 
        ../Core/Video/DirectshowFrameGrabber.cpp
        ../Core/Video/DirectshowFrameGrabber2.cpp
        ../Core/Video/DirectshowVideoFrame.cpp
        ../Core/Video/NoDirectVobSub.cpp
        ../Core/Video/GrabFilter.cpp
        ../Core/Video/DirectShowUtil.cpp
        ../Core/3rdpart/dxerr.cpp)
endif()

add_executable(qimageuploader ${OPTIONS} ${SOURCE_FILES})
 
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swscale)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} gdi32 winmm dwmapi debug strmbasd optimized strmbase)
endif()

target_link_libraries(qimageuploader iucore ${COMMON_LIBS_LIST})