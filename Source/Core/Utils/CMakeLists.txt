cmake_minimum_required(VERSION 2.8) 
																	
project(iuCore)				

include_directories(../../)

include_directories(../../../Contrib/Include)
include_directories(../../../Contrib/Include/Libs)

include_directories(../../../Contrib/Include/pcre)

if(WIN32)
  include_directories(../../../Contrib/Source/boost)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
  set(WIN_TARGET 0x0502) # Windows XP SP2

  set(WIN_IE_VERSION 0x0603) # IE 6 SP2

  add_definitions(-DWINVER=${WIN_TARGET}
                -D_WIN32_WINNT=${WIN_SDK_MIN}
                -D_WIN32_IE=${WIN_IE_VERSION}
				-D_USING_V120_SDK71_)
  
  set(WIN_SDK_MIN 0x0600) # Windows Vista
endif()

add_definitions(-DPCRE_STATIC -DTIXML_USE_STL)
add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "/MP")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" "/MP")
    set(CMAKE_GENERATOR_TOOLSET "v120_xp" CACHE STRING "Platform toolset" FORCE)
endif()

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
    ../3rdpart/base64.cpp
    ../3rdpart/CodePages.cpp
    ../3rdpart/dxerr.cpp
    ../3rdpart/htmlentities.cpp
    ../3rdpart/pcreplusplus.cpp
    ../3rdpart/tinystr.cpp
    ../3rdpart/tinyxml.cpp
    ../3rdpart/tinyxmlerror.cpp
    ../3rdpart/tinyxmlparser.cpp
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