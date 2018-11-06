cmake_minimum_required(VERSION 2.8)							

#set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/c_flag_overrides.cmake)
#set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cxx_flag_overrides.cmake)

if (CMAKE_VERSION VERSION_LESS "3.1")
#    include(CheckCXXCompilerFlag)
#    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
#    if(COMPILER_SUPPORTS_CXX11)
#        message("Compiler supports C++11.")
#        
#    elseif(COMPILER_SUPPORTS_CXX0X)
#        message("Compiler supports c++0x.")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#    else()
#        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
#    endif()
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    message("Stadart set to 11")
else ()
    set (CMAKE_CXX_STANDARD 11)
endif ()


#use_cxx11()
set(CMAKE_SKIP_RPATH 1)   
project(ImageUploader)	

#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.01")
if (WIN32)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DCURL_STATICLIB -DPCRE_STATIC -DGOOGLE_GLOG_DLL_DECL=)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32_IE=0x0603 -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -D_ATL_XP_TARGETING)
endif(MSVC)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

if (NOT WIN32) 
    find_package(Boost COMPONENTS filesystem system locale thread program_options REQUIRED)
    find_package(OpenSSL REQUIRED)
    message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
    add_definitions(-DUSE_OPENSSL -DUSE_FREEIMAGE)
#    find_package(jsoncpp REQUIRED)
#   if(jsoncpp_FOUND)
#         message("jsoncpp found")
#         set(LIBJSONCPP_INCLUDE_DIR $<TARGET_PROPERTY:jsoncpp_lib_static,INTERFACE_INCLUDE_DIRECTORIES>)
#    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/squirrel/)
include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/sqplus/)
include_directories(${CMAKE_SOURCE_DIR}/)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/Libs/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Source/boost/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/WTL/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/gumbo/)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/pcre/)
endif()


link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_definitions(-DTIXML_USE_STL -DENABLE_SYNC -DSCRAT_USE_EXCEPTIONS -D_SQ64 -DIU_CLI)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
    link_directories(${CMAKE_SOURCE_DIR}/../Contrib/Lib/ ${CMAKE_SOURCE_DIR}/../Contrib/Source/boost/stage/lib/)
else()
    include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
endif()

# libiucore depends on these libraries
set(COMMON_LIBS_LIST squirrel sqstdlib pcre gumbo mega uriparser cryptopp cares jsoncpp)

if(WIN32)
    list(APPEND COMMON_LIBS_LIST libcurl libeay32 ssleay32 Oleacc libglog_static minizip Userenv Psapi Iphlpapi Winhttp libuv base64 libwebpdecoder libwebpdemux)
else()
    list(APPEND COMMON_LIBS_LIST curl ssl crypto glog pthread uv rt sqlite3 freeimage gflags ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
endif()

add_subdirectory(Core)	
add_subdirectory(CLI)	
if(WIN32)
    add_subdirectory(ServerListTool)
endif()
add_subdirectory(Tests)	
	