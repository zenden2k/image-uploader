
add_definitions(-DSQTEST_STATIC -DIU_NOWEBBROWSER -DIU_TESTS)

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
   ../Core/DownloadTaskTest.cpp
)
if(WIN32)
    list(APPEND SRC_LIST 
        ../Func/Tests/CmdLineTest.cpp 
        ../Func/CmdLine.cpp
        ../Func/Tests/WinUtilsTest.cpp 
        ../Core/Images/Tests/UtilsTest.cpp
        ../Core/Images/Tests/ImageConverterTest.cpp
        ../Core/Images/ImageConverterPrivate_gdiplus.cpp
        ../Core/Images/ImageConverterPrivateBase.cpp
        ../Core/Images/ImageConverter.cpp
        ../Core/Images/Thumbnail.cpp
        ../Core/3rdpart/parser.cpp
        ../Func/MyUtils.cpp
        ../3rdpart/QColorQuantizer.cpp
		../Core/FileDownloader.cpp
        ../Core/FileDownloaderTest.cpp
        ../Video/Tests/VideoGrabberTest.cpp
        ../Core/Settings/CliSettings.cpp
    )
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/DX/)
endif()

add_executable(Tests ${SRC_LIST})

set_target_properties( Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Tests/)
if(WIN32 AND IU_ENABLE_FFMPEG)
    set_target_properties(Tests PROPERTIES LINK_FLAGS "/DELAYLOAD:avcodec-58.dll /DELAYLOAD:avformat-58.dll /DELAYLOAD:avutil-56.dll /DELAYLOAD:swscale-5.dll")
endif()

set(LIBS_LIST "")

if(WIN32)
    list(APPEND LIBS_LIST gdiplus WebP::webpdemux WebP::webpdecoder winmm urlmon)
	if (IU_ENABLE_FFMPEG)
		list(APPEND LIBS_LIST avformat avutil swscale avcodec)
	endif()
    target_link_libraries(Tests base-classes::base-classes)
else()
    #list(APPEND LIBS_LIST icuuc icui18n gtest)
endif()

target_link_libraries(Tests iucore iuvideo ${COMMON_LIBS_LIST} ${LIBS_LIST} )

target_link_libraries(Tests GTest::GTest)

