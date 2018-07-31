include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
include_directories(${CMAKE_SOURCE_DIR}/)

include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/pcre/)
include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/squirrel/)
include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/sqplus/)
include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/gumbo/)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/Libs/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Source/boost/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/WTL/)
endif()


link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/../Contrib/Lib/ ${CMAKE_SOURCE_DIR}/../Contrib/Source/boost/stage/lib/)

add_definitions(-DCURL_STATICLIB -DPCRE_STATIC -DTIXML_USE_STL -DIU_CLI -DGOOGLE_GLOG_DLL_DECL= -DENABLE_SYNC -DSCRAT_USE_EXCEPTIONS -DSQTEST_STATIC -DIU_NOWEBBROWSER -DIU_TESTS)


if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

set(SRC_LIST 
    Tests.cpp
   ../Core/Utils/Tests/CoreUtilsTest.cpp
   ../Core/Utils/Tests/CryptoUtilsTest.cpp
   ../Core/Utils/Tests/StringUtilsTest.cpp
   ../Core/Utils/Tests/TextUtilsTest.cpp
   ../Core/Upload/Tests/UploadEngineListTest.cpp
   ../Core/3rdpart/GumboQuery/Tests/GumboTest.cpp
)
if(WIN32)
    list(APPEND SRC_LIST 
        ../Func/Tests/CmdLineTest.cpp 
        ../Func/CmdLine.cpp
        ../Core/Images/Tests/UtilsTest.cpp
        ../Core/Images/Utils.cpp
        ../Func/GdiPlusInitializer.cpp
        ../3rdpart/QColorQuantizer.cpp
        ../3rdpart/QColorQuantizer.cpp
    )
endif()

add_executable(Tests ${SRC_LIST})

set_target_properties( Tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Tests/)

set(LIBS_LIST libiucore squirrel sqstdlib jsoncpp pcre gumbo mega uriparser cryptopp cares libuv sqtest gdiplus libwebpdecoder libwebpdemux)


if(WIN32)
    list(APPEND LIBS_LIST libcurl libeay32 ssleay32 Oleacc libglog_static minizip Userenv Psapi Iphlpapi Winhttp base64)
else()
    list(APPEND LIBS_LIST curl ssl crypto glog pthread boost_filesystem boost_system ncurses)
endif()

target_link_libraries(Tests ${LIBS_LIST})
target_link_libraries(Tests debug gtestd optimized gtest)
