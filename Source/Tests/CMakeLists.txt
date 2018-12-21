
add_definitions(-DSQTEST_STATIC -DIU_NOWEBBROWSER -DIU_TESTS)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

set(SRC_LIST 
    Tests.cpp
    TestHelpers.cpp
    sqtest.cpp
   ../Core/Utils/Tests/CoreUtilsTest.cpp
   ../Core/Utils/Tests/CryptoUtilsTest.cpp
   ../Core/Utils/Tests/StringUtilsTest.cpp
   ../Core/Utils/Tests/TextUtilsTest.cpp
   ../Core/Upload/Tests/UploadEngineListTest.cpp
   ../Core/Upload/Tests/ScriptUploadEngineTest.cpp
   ../Core/Upload/Tests/DefaultUploadEngineTest.cpp
   ../Core/3rdpart/GumboQuery/Tests/GumboTest.cpp
)
if(WIN32)
    list(APPEND SRC_LIST 
        ../Func/Tests/CmdLineTest.cpp 
        ../Func/CmdLine.cpp
        ../Func/Tests/WinUtilsTest.cpp 
        ../Core/Images/Tests/UtilsTest.cpp
        ../Func/GdiPlusInitializer.cpp
        ../3rdpart/QColorQuantizer.cpp
        ../3rdpart/QColorQuantizer.cpp
		../Core/FileDownloader.cpp
        ../Core/FileDownloaderTest.cpp
        ../Core/Video/AvcodecFrameGrabber.cpp
        ../Core/Video/AbstractFrameGrabber.cpp
        ../Core/Video/DirectshowFrameGrabber.cpp
        ../Core/Video/AbstractVideoFrame.cpp
        ../Core/Video/DirectShowUtil.cpp
        ../Core/Video/AbstractImage.cpp
        ../Core/Video/GdiPlusImage.cpp
        ../Core/Video/Tests/VideoGrabberTest.cpp
        ../Core/3rdpart/dxerr.cpp
    )
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/BaseClasses/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/DX/)
endif()

add_executable(Tests ${SRC_LIST})

set_target_properties( Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Tests/)

set(LIBS_LIST  "")

if(WIN32)
    list(APPEND LIBS_LIST gdiplus libwebpdecoder libwebpdemux base64 avformat avutil swscale avcodec winmm)
    target_link_libraries(Tests debug strmbasd optimized strmbase)
else()
    list(APPEND LIBS_LIST icuuc icui18n)
endif()

target_link_libraries(Tests iucore ${COMMON_LIBS_LIST} ${LIBS_LIST} )
target_link_libraries(Tests debug gmockd optimized gmock)
