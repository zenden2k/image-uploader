cmake_minimum_required(VERSION 2.8) 
																	
project(iuCore)				

include_directories(../../)

add_definitions(-DPCRE_STATIC -DTIXML_USE_STL)

set(SOURCE_FILES
    ConsoleUtils.cpp
    CoreUtils.cpp
    CryptoUtils.cpp
    DesktopUtils.cpp
    GlobalMutex.cpp
    SimpleXml.cpp
    StringUtils.cpp
    SystemUtils.cpp
    TextUtils.cpp
)	

set(HEADER_FILES 
    ConsoleUtils.h
    CoreTypes.h
    CoreUtils.h
    CryptoUtils.h
    CryptoUtils_unix.h
    CryptoUtils_win.h
    DesktopUtils.h
    EnumUtils.h
    GlobalMutex.h
    GlobalMutex_unix.h
    GlobalMutex_win.h
    SimpleXml.h
    Singleton.h
    StringUtils.h
    SystemUtils.h
    SystemUtils_unix.h
    SystemUtils_win.h
    TextUtils.h
    utils_unix.h
    utils_win.h
)
	
add_library(iuCore STATIC ${SOURCE_FILES})

target_link_libraries(iuCore pcre)