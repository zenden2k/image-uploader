cmake_minimum_required(VERSION 3.1.0)							

#set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_SOURCE_DIR}/c_flag_overrides.cmake)
#set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cxx_flag_overrides.cmake)

set(CMAKE_CXX_STANDARD 17)

#use_cxx11()
set(CMAKE_SKIP_RPATH 1)   
project(ImageUploader)	

set(IU_ENABLE_MEGANZ ON CACHE BOOL "Enable support of mega.nz")
set(IU_USE_OPENSSL ON CACHE BOOL "Use OpenSSL library for secured connections")
if(WIN32)
	set(IU_ENABLE_WEBVIEW2 OFF CACHE BOOL "Enable support of Microsoft Edge WebView2")
endif()
set(IU_ENABLE_FFMPEG OFF CACHE BOOL "Use FFmpeg libraries")
set(IU_BUILD_QIMAGEUPLOADER OFF CACHE BOOL "Enable build of imageuploader-qt")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

include(conan.cmake)

conan_add_remote(NAME zenden2k INDEX 1
            URL http://conan.zenden2k.com)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if (MSVC)
    set(IU_STATIC_RUNTIME ON CACHE BOOL "Use static runtime")
    add_definitions("/MP")
    add_compile_options(/wd4091) # disable warning C4091: 'typedef ': ignored on left of '*' when no variable is declared
endif()

if (MSVC AND IU_STATIC_RUNTIME) 
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
endif()

if (IU_ENABLE_FFMPEG)
	add_definitions(-DIU_ENABLE_FFMPEG)
endif()


if (WIN32)
# -DPCRE_STATIC 
    add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DGOOGLE_GLOG_DLL_DECL=)
    if (IU_USE_OPENSSL)
        add_definitions(-DUSE_OPENSSL)
    endif()
	if (IU_ENABLE_WEBVIEW2)
        add_definitions(-DIU_ENABLE_WEBVIEW2)
    endif()
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWINVER=0x0601 -D_WIN32_WINNT=0x0601)
endif(MSVC)

SET(COMMON_DEPS boost/1.78.0
				openssl/1.1.1l
				libcurl/7.79.1
				pcre/8.45
				uriparser/0.9.5
				zlib/1.2.11
				minizip/1.2.11	
				jsoncpp/1.9.5
				libssh2/1.9.0 
				sqlite3/3.36.0
				base64/0.4.0
				glog/0.5.0
				cryptopp/8.5.0
				c-ares/1.17.2
				libuv/1.42.0
				libwebp/1.2.1
				libmediainfo/21.09
				gtest/1.10.0
				gumbo-parser/0.10.1@zenden2k/stable
				squirrel/3.0.0@zenden2k/stable

)

if (IU_ENABLE_MEGANZ)
	list(APPEND COMMON_DEPS megaio/3.5.2@zenden2k/stable)
endif()

if (WIN32)
	list(APPEND COMMON_DEPS wtl/10.0.9163 
			base-classes/1.0.0@zenden2k/stable
	)
endif()	
	
if (WIN32)
	list(APPEND COMMON_DEPS wtl/10.0.9163 base-classes/1.0.0@zenden2k/stable)
endif()	
	
conan_cmake_configure(REQUIRES "${COMMON_DEPS}"
						GENERATORS cmake_find_package_multi
						OPTIONS boost:shared=False 
						libcurl:with_libssh2=True
						)

# ffmpeg package supports Windows x64 only as target system, so we have to build ffmpeg separately
SET(msvc_link_setting "")

SET(msvc_link_setting "")

foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
	if (MSVC)
		if (${TYPE} STREQUAL "Debug")
			SET(msvc_link_setting compiler.runtime=MTd)
		else()
			SET(msvc_link_setting compiler.runtime=MT)
		endif()	
	endif()	
    conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})
    conan_cmake_install(PATH_OR_REFERENCE .
                        BUILD missing
                        SETTINGS ${settings}
						SETTINGS ${msvc_link_setting})
endforeach()

set(CMAKE_MODULE_PATH ${CONAN_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

find_package(Boost CONFIG COMPONENTS program_options REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(PCRE CONFIG REQUIRED)
find_package(uriparser CONFIG REQUIRED)
find_package(ZLIB CONFIG REQUIRED)
find_package(uriparser CONFIG REQUIRED)
if(WIN32)
	find_package(wtl CONFIG REQUIRED)
endif()
find_package(base64 CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(Libssh2 CONFIG REQUIRED)
find_package(SQLite3 CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(c-ares CONFIG REQUIRED)
find_package(libuv CONFIG REQUIRED)
find_package(WebP CONFIG REQUIRED)
find_package(MediaInfoLib CONFIG REQUIRED) 
if(IU_ENABLE_MEGANZ)
	find_package(megaio CONFIG REQUIRED) 
endif()	
find_package(gumbo-parser CONFIG REQUIRED) 
find_package(GTest CONFIG REQUIRED)
find_package(squirrel CONFIG COMPONENTS sqstdlib REQUIRED) 
if(WIN32)
	find_package(base-classes CONFIG REQUIRED)
endif()	
  
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
include_directories(${CMAKE_SOURCE_DIR}/)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/Libs/)
	include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/win32/)
endif()

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
 
add_definitions(-DTIXML_USE_STL -DSCRAT_USE_EXCEPTIONS -D_SQ64 -DBOOST_BIND_NO_PLACEHOLDERS -DBOOST_ALL_NO_LIB -DURI_STATIC_BUILD)

if(IU_ENABLE_MEGANZ)
    add_definitions(-DIU_ENABLE_MEGANZ)
endif()

if(WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
    link_directories(${CMAKE_SOURCE_DIR}/../Contrib/Lib/)
else()
    include_directories(${CONAN_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
endif()

set(COMMON_LIBS_LIST Boost::Boost CURL::libcurl uriparser::uriparser ZLIB::ZLIB minizip::minizip jsoncpp::jsoncpp Libssh2::Libssh2 glog::glog PCRE::PCRE SQLite::SQLite3 squirrel::squirrel squirrel::sqstdlib gumbo-parser::gumbo-parser base64::base64)
if (IU_ENABLE_MEGANZ)
    list(APPEND COMMON_LIBS_LIST megaio::megaio cryptopp::cryptopp-static c-ares::c-ares MediaInfoLib::MediaInfoLib)
endif()

if (IU_ENABLE_MEGANZ)
	list(APPEND COMMON_LIBS_LIST libuv::libuv)
endif()
    
if (IU_USE_OPENSSL)
	list(APPEND COMMON_LIBS_LIST OpenSSL::OpenSSL)
endif()

if(WIN32)
	list(APPEND COMMON_LIBS_LIST wtl::wtl Oleacc Userenv Psapi Iphlpapi Winhttp WebP::webpdemux WebP::webpdecoder Crypt32 Wldap32 Urlmon)
	
    
	if (IU_ENABLE_WEBVIEW2)
		list(APPEND COMMON_LIBS_LIST WebView2Guid.lib WebView2LoaderStatic.lib version.lib)
	endif()	
else()
    list(APPEND COMMON_LIBS_LIST dl base64::base64  OpenSSL::OpenSSL pthread rt gflags)
    SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

add_subdirectory(Core)	
add_subdirectory(CLI)
if(WIN32)
    add_subdirectory(Gui)
    add_subdirectory(ServerListTool)
endif()

if(IU_BUILD_QIMAGEUPLOADER)
	add_subdirectory(qimageuploader)	
endif()	
add_subdirectory(Tests)	
