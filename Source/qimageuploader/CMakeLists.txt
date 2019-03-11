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

set(CMAKE_CXX_STANDARD 11)

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
    ../Core/Video/DirectshowFrameGrabber.cpp
    ../Core/Video/VideoGrabber.cpp
    ../Core/Video/AvcodecFrameGrabber.cpp
    ../Core/Video/DirectShowUtil.cpp
    ../Core/3rdpart/dxerr.cpp
    Gui/FrameGrabberDlg.cpp
    Gui/ResultsWindow.cpp
    ../Core/ScreenCapture/ScreenCapture.cpp
    Gui/RegionSelect.cpp
    Gui/models/uploadtreemodel.cpp 
    Gui/controls/MyListWidget.cpp
    ../Core/Video/AbstractImage.cpp 
    ../Core/Video/QtImage.cpp
    QtUploadErrorHandler.cpp
    Gui/ui/mainwindow.ui 
    Gui/ui/FrameGrabberDlg.ui
    Gui/ui/ResultsWindow.ui
    qimageuploader.qrc
)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/BaseClasses/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/DX/)
    SET(OPTIONS WIN32)
endif()

add_executable(qimageuploader ${OPTIONS} ${SOURCE_FILES})
 
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swscale)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} strmbasd gdi32 winmm)
endif()

target_link_libraries(qimageuploader iucore ${COMMON_LIBS_LIST})