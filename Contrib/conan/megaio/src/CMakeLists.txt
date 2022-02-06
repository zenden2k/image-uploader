cmake_minimum_required(VERSION 3.7.2)
project(megaio CXX)

# This file enables building with cmake on windows  
#
# use cases
#  - generate a visual studio solution and projects, eg: cmake . -G "Visual Studio 15 2017"
#  - or for 64 bit: cmake . -G "Visual Studio 15 2017 Win64"
#  - or set your build options before VS project generation by using the gui, eg:  cmake-gui.exe .
#  - you can set up to build both 32 bit and 64 bit this way:
#       make subfolders '32' and '64' of this cmake folder
#       execute 'cmake-gui  ..' in each.  Select the cmake folder as the 'source code' folder, and the appropriate 32 or 64 folder as 'where to build the binaries' folder
#       for each cmake-gui configuration, choose the suitable 32/64 bit compiler, and corresponding setting for the build_64_bit variable.
#       then Configure, Generate, Open Project (all buttons in the gui) for each.

#indicate which dependent libraries to use in the build
set (USE_CRYPTOPP 1 CACHE TYPE BOOL)
set (USE_OPENSSL 1 CACHE TYPE BOOL)
set (USE_CURL 1 CACHE TYPE BOOL)
set (USE_SQLITE 1 CACHE TYPE BOOL)
set (USE_MEDIAINFO 1 CACHE TYPE BOOL)
set (USE_FREEIMAGE 0 CACHE TYPE BOOL)
set (USE_SODIUM 0 CACHE TYPE BOOL)
set (ENABLE_SYNC 1 CACHE TYPE BOOL)
set (ENABLE_CHAT 0 CACHE TYPE BOOL)
set (HAVE_FFMPEG 1 CACHE TYPE BOOL)
set (USE_WEBRTC 0 CACHE TYPE BOOL)
set (USE_LIBUV 0 CACHE TYPE BOOL)
set (USE_QT 0 CACHE TYPE BOOL)
set (USE_PDFIUM 0  CACHE TYPE BOOL)

if (USE_QT)
    set( USE_CPPTHREAD 0)
else()
    set( USE_CPPTHREAD 1)
ENDIF()

if (WIN32)
    set (NO_READLINE 1 CACHE TYPE BOOL)
    set (UNCHECKED_ITERATORS 0 CACHE TYPE BOOL)
    set(USE_PREBUILT_3RDPARTY 1 CACHE TYPE BOOL)
    set(HAVE_LIBUV ${USE_LIBUV})
    IF (USE_WEBRTC)
        # IF ("${WebRtcDir}" STREQUAL "")
            # SET (WebRtcDir "${Mega3rdPartyDir}/libwebrtc/build32debug")
        # ENDIF()
    ENDIF()
else(WIN32)
    set(NO_READLINE 0)
    set(USE_PREBUILT_3RDPARTY 0)
    set(HAVE_LIBUV 0)
endif(WIN32)

if (USE_WEBRTC)
    # if ("${WebRtcDir}" STREQUAL "")
        # SET(WebRtcDir ${Mega3rdPartyDir}/libwebrtc/build32debug)
    # endif()
endif()

if (ENABLE_CHAT AND NOT USE_SODIUM)
    message(FATAL_ERROR "ENABLE_CHAT (${ENABLE_CHAT}) requires USE_SODIUM (${USE_SODIUM})")
endif()

if (WIN32)
    if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
        SET(build_64_bit 1)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        SET(build_64_bit 0)
    endif()
else()
    set (build_64_bit 1 CACHE TYPE BOOL)
endif()

if(build_64_bit)
    project (MegaSDK64 LANGUAGES CXX C)
else(build_64_bit)
    project (MegaSDK32 LANGUAGES CXX C)
endif(build_64_bit)

IF(WIN32)
    set(MegaDir "${CMAKE_CURRENT_LIST_DIR}/../")
    if ("${Mega3rdPartyDir}" STREQUAL "")
        set(Mega3rdPartyDir "${MegaDir}/../3rdParty" CACHE TYPE STRING)
    endif()
ELSE(WIN32)
    set(MegaDir "${CMAKE_CURRENT_LIST_DIR}/")
    if ("${Mega3rdPartyDir}" STREQUAL "")
        set (Mega3rdPartyDir "${MegaDir}/../3rdParty/")
    endif()
ENDIF(WIN32)

#include(conan.cmake)

#IF(WIN32)
#    if (UNCHECKED_ITERATORS)
#        if(build_64_bit)
#            set(vcpkg_dir "${Mega3rdPartyDir}/vcpkg/installed/x64-windows-static-uncheckediterators")
#        else(build_64_bit)
#            set(vcpkg_dir "${Mega3rdPartyDir}/vcpkg/installed/x86-windows-static-uncheckediterators")
#        endif(build_64_bit)
#    else() 
#        if(build_64_bit)
#            set(vcpkg_dir "${Mega3rdPartyDir}/vcpkg/installed/x64-windows-static")
#        else(build_64_bit)
#            set(vcpkg_dir "${Mega3rdPartyDir}/vcpkg/installed/x86-windows-static")
#        endif(build_64_bit)
#    endif()
#ELSE(WIN32)
#    set(vcpkg_dir "${Mega3rdPartyDir}/vcpkg/installed/x64-linux")
#    include_directories( "${vcpkg_dir}/include" )
#    link_directories( "${vcpkg_dir}/lib" )
#ENDIF(WIN32)


if (NOT CMAKE_BUILD_TYPE)
    message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
else(NOT CMAKE_BUILD_TYPE)
    message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
endif(NOT CMAKE_BUILD_TYPE)

#windows projects usually need _DEBUG and/or DEBUG set rather than NDEBUG not set
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")

# node deletion in debug under VC++ is pretty slow without this.  However libraries we depend on need to be built with the same setting or linking fails 
# (hence the build3rdParty script using the xNN-windows-static-uncheckediterators triplets)
if (UNCHECKED_ITERATORS OR NOT USE_PREBUILT_3RDPARTY)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=0" )
endif()

# accurate __cplusplus macro for vc++, selecting c++17 here for windows builds though the MEGA SDK library must build for older c++ standards also
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++17")
    add_definitions( -DNOMINMAX )
endif()

IF(WIN32)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
ENDIF(WIN32)

include(CheckIncludeFile)
include(CheckFunctionExists)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(uv.h HAVE_LIBUV)
check_function_exists(aio_write, HAVE_AIO_RT)
check_function_exists(fdopendir, HAVE_FDOPENDIR)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} 
						${CMAKE_MODULE_PATH})		

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})			

IF(WIN32)
    #if(USE_PREBUILT_3RDPARTY)
		find_package(ZLIB REQUIRED)
        
        IF(USE_CRYPTOPP)
            find_package(cryptopp REQUIRED)
        ENDIF(USE_CRYPTOPP)

        IF(USE_CURL)
            find_package(CURL REQUIRED)
			find_package(c-ares REQUIRED)
        ENDIF(USE_CURL)
        
        IF(USE_SODIUM)
            find_package(sodium REQUIRED)
        ENDIF(USE_SODIUM)
        
        IF(USE_OPENSSL)
            find_package(OpenSSL REQUIRED)
        ENDIF(USE_OPENSSL)
        
        IF(USE_MEDIAINFO)
			find_package(MediaInfoLib REQUIRED)
            #ImportVcpkgLibrary(mediainfo    "${prebuilt_dir}/include" "${prebuilt_dir}/libs/x32d/MediaInfo.lib" "${prebuilt_dir}/libs/x32/MediaInfo.lib")
            #ImportVcpkgLibrary(zen      	"${prebuilt_dir}/include" "${prebuilt_dir}/libs/x32d/ZenLib.lib" "${prebuilt_dir}/libs/x32/ZenLib.lib")
        ENDIF(USE_MEDIAINFO)
        
        IF(HAVE_FFMPEG)
			#find_package(ffmpeg CONFIG REQUIRED)
            #ImportVcpkgLibrary(ffmpeg_avformat "${prebuilt_dir}/include/ffmpeg" "${prebuilt_dir}/libs/x32d/avformat.lib" "${prebuilt_dir}/libs/x32/avformat.lib")
            #ImportVcpkgLibrary(ffmpeg_avutil "${prebuilt_dir}/include/ffmpeg" "${prebuilt_dir}/libs/x32d/avutil.lib" "${prebuilt_dir}/libs/x32/avutil.lib")
            #ImportVcpkgLibrary(ffmpeg_avcodec "${prebuilt_dir}/include/ffmpeg" "${prebuilt_dir}/libs/x32d/avcodec.lib" "${prebuilt_dir}/libs/x32/avcodec.lib")
            #ImportVcpkgLibrary(ffmpeg_avfilter "${prebuilt_dir}/include/ffmpeg" "${prebuilt_dir}/libs/x32d/avfilter.lib" "${prebuilt_dir}/libs/x32/avfilter.lib")
            #ImportVcpkgLibrary(ffmpeg_avdevice "${prebuilt_dir}/include/ffmpeg" "${prebuilt_dir}/libs/x32d/avdevice.lib" "${prebuilt_dir}/libs/x32/avdevice.lib")
        ENDIF(HAVE_FFMPEG)
        
        IF(USE_LIBUV)
			find_package(libuv REQUIRED)
            SET(HAVE_LIBUV 1)
        ENDIF(USE_LIBUV)
        
        IF(USE_WEBRTC)
			find_package(webrtc REQUIRED)
            add_definitions( -DWEBRTC_WIN )
        ENDIF()
    
		find_package(gtest REQUIRED)
		

        # IF(USE_FREEIMAGE)
            # ImportVcpkgLibrary(freeimage    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/FreeImaged.lib"  "${vcpkg_dir}/lib/FreeImage.lib")
            # ImportVcpkgLibrary(freeimage_Iex "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/Iex-2_2.lib"  "${vcpkg_dir}/lib/Iex-2_2.lib")
            # ImportVcpkgLibrary(freeimage_IexMath "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/IexMath-2_2.lib"  "${vcpkg_dir}/lib/IexMath-2_2.lib")
            # ImportVcpkgLibrary(freeimage_IlmImf "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/IlmImf-2_2.lib"  "${vcpkg_dir}/lib/IlmImf-2_2.lib")
            # ImportVcpkgLibrary(freeimage_IlmImfUtil "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/IlmImfUtil-2_2.lib"  "${vcpkg_dir}/lib/IlmImfUtil-2_2.lib")
            # ImportVcpkgLibrary(freeimage_IlmThread "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/IlmThread-2_2.lib"  "${vcpkg_dir}/lib/IlmThread-2_2.lib")
            # ImportVcpkgLibrary(freeimage_jpeg "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/jpeg.lib"  "${vcpkg_dir}/lib/jpeg.lib")
            # ImportVcpkgLibrary(freeimage_turbojpeg "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/turbojpeg.lib"  "${vcpkg_dir}/lib/turbojpeg.lib")
            # ImportVcpkgLibrary(freeimage_jpegxr "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/jpegxrd.lib"  "${vcpkg_dir}/lib/jpegxr.lib")
            # ImportVcpkgLibrary(freeimage_jxrglue "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/jxrglued.lib"  "${vcpkg_dir}/lib/jxrglue.lib")
            # ImportVcpkgLibrary(freeimage_openjp2 "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/openjp2.lib"  "${vcpkg_dir}/lib/openjp2.lib")
            # ImportVcpkgLibrary(freeimage_half "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/half.lib"  "${vcpkg_dir}/lib/half.lib")
            # ImportVcpkgLibrary(freeimage_jasper "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/jasper.lib"  "${vcpkg_dir}/lib/jasper.lib")
            # ImportVcpkgLibrary(freeimage_libpng "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/libpng16d.lib"  "${vcpkg_dir}/lib/libpng16.lib")
            # ImportVcpkgLibrary(freeimage_lzma    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/lzma.lib"  "${vcpkg_dir}/lib/lzma.lib")
            # ImportVcpkgLibrary(freeimage_lcms2    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/lcms2d.lib"  "${vcpkg_dir}/lib/lcms2.lib")
            # ImportVcpkgLibrary(freeimage_raw    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/rawd.lib"  "${vcpkg_dir}/lib/raw.lib")
            # ImportVcpkgLibrary(freeimage_tiff    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/tiffd.lib"  "${vcpkg_dir}/lib/tiff.lib")
            # ImportVcpkgLibrary(freeimage_tiffxx    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/tiffxxd.lib"  "${vcpkg_dir}/lib/tiffxx.lib")
            # ImportVcpkgLibrary(freeimage_webp    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/webpd.lib"  "${vcpkg_dir}/lib/webp.lib")
            # ImportVcpkgLibrary(freeimage_webpdecoder    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/webpdecoderd.lib"  "${vcpkg_dir}/lib/webpdecoder.lib")
            # ImportVcpkgLibrary(freeimage_webpdemux    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/webpdemuxd.lib"  "${vcpkg_dir}/lib/webpdemux.lib")
            # ImportVcpkgLibrary(freeimage_webpmux    "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/webpmuxd.lib"  "${vcpkg_dir}/lib/webpmux.lib")
        # ENDIF(USE_FREEIMAGE)

        #IF(HAVE_FFMPEG)
            # ImportVcpkgLibrary(ffmpeg_avformat "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/avformat.lib" "${vcpkg_dir}/lib/avformat.lib")
            # ImportVcpkgLibrary(ffmpeg_avutil "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/avutil.lib" "${vcpkg_dir}/lib/avutil.lib")
            # ImportVcpkgLibrary(ffmpeg_avcodec "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/avcodec.lib" "${vcpkg_dir}/lib/avcodec.lib")
            # ImportVcpkgLibrary(ffmpeg_avfilter "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/avfilter.lib" "${vcpkg_dir}/lib/avfilter.lib")
            # ImportVcpkgLibrary(ffmpeg_avdevice "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/avdevice.lib" "${vcpkg_dir}/lib/avdevice.lib")
            # ImportVcpkgLibrary(ffmpeg_swscale  "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/swscale.lib"  "${vcpkg_dir}/lib/swscale.lib")
            # ImportVcpkgLibrary(ffmpeg_swresample "${vcpkg_dir}/include" "${vcpkg_dir}/debug/lib/swresample.lib"  "${vcpkg_dir}/lib/swresample.lib")
        #ENDIF(HAVE_FFMPEG)

        IF(USE_SQLITE)
			find_package(SQLite3 REQUIRED)
        ENDIF(USE_SQLITE)

    
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DCURL_STATICLIB -DCARES_STATICLIB -DWIN32_LEAN_AND_MEAN -DUNICODE -DSODIUM_STATIC -DPCRE_STATICWIN32 -D_CONSOLE )
    SET(Mega_PlatformSpecificIncludes ${MegaDir}/include/mega/$<IF:${USE_CURL},wincurl,win32>)
    SET(Mega_PlatformSpecificLibs ws2_32 winhttp Shlwapi Secur32.lib  $<${USE_WEBRTC}:Wldap32.lib> )

    SET(Mega_PlatformSpecificFiles ${MegaDir}/src/win32/console.cpp 
    ${MegaDir}/src/win32/consolewaiter.cpp 
    ${MegaDir}/src/win32/fs.cpp 
    $<IF:${USE_CURL},${MegaDir}/src/posix/net.cpp,${MegaDir}/src/win32/net.cpp>
    ${MegaDir}/src/win32/waiter.cpp 
    $<${USE_CPPTHREAD}:${MegaDir}/src/thread/cppthread.cpp>
    $<${USE_QT}:${MegaDir}/src/thread/qtthread.cpp >
    )


ELSE(WIN32)

    add_definitions(-DUSE_PTHREAD )

    check_include_file(glob.h HAVE_GLOB_H)
    if (HAVE_GLOB_H)
        set(GLOB_H_FOUND 1)
    else()
        set(GLOB_H_FOUND 0)   #some versions on some platforms leave it undefined if not found
    endif()

    SET(Mega_PlatformSpecificFiles $<$<NOT:${GLOB_H_FOUND}>:${MegaDir}/src/mega_glob.c> ${MegaDir}/src/posix/console.cpp ${MegaDir}/src/posix/consolewaiter.cpp ${MegaDir}/src/posix/fs.cpp ${MegaDir}/src/posix/net.cpp ${MegaDir}/src/posix/waiter.cpp ${MegaDir}/src/thread/posixthread.cpp )
    SET(Mega_PlatformSpecificIncludes ${MegaDir}/include/mega/posix)
    SET(Mega_PlatformSpecificLibs crypto pthread rt z dl termcap stdc++fs)
    IF(USE_WEBRTC)
        #ImportVcpkgLibrary(webrtc "${WebRtcDir}/include" "${WebRtcDir}/lib/webrtc.lib" "${WebRtcDir}/lib/webrtc.lib")
        add_definitions( -DWEBRTC_POSIX )
    ENDIF()

ENDIF(WIN32)

configure_file ("${MegaDir}/contrib/cmake/config.h.in" "${MegaDir}/include/mega/config.h" )


SET(Mega_CryptoFiles ${MegaDir}/src/crypto/cryptopp.cpp ${MegaDir}/src/crypto/sodium.cpp)
SET(Mega_DbFiles ${MegaDir}/src/db/sqlite.cpp ${MegaDir}/src/db/sqlite.cpp )
SET(Mega_GfxFiles ${MegaDir}/src/gfx/external.cpp ${MegaDir}/src/gfx/freeimage.cpp ) 

add_library(megaio STATIC
            ${MegaDir}/src/attrmap.cpp 
            ${MegaDir}/src/autocomplete.cpp 
            ${MegaDir}/src/backofftimer.cpp 
            ${MegaDir}/src/base64.cpp 
            ${MegaDir}/src/command.cpp 
            ${MegaDir}/src/commands.cpp 
            ${MegaDir}/src/db.cpp 
            ${MegaDir}/src/file.cpp 
            ${MegaDir}/src/fileattributefetch.cpp 
            ${MegaDir}/src/filefingerprint.cpp 
            ${MegaDir}/src/filesystem.cpp 
            ${MegaDir}/src/gfx.cpp 
            ${MegaDir}/src/http.cpp 
            ${MegaDir}/src/json.cpp 
            ${MegaDir}/src/logging.cpp 
            ${MegaDir}/src/mediafileattribute.cpp 
            ${MegaDir}/src/mega_ccronexpr.cpp 
            ${MegaDir}/src/mega_http_parser.cpp 
            ${MegaDir}/src/mega_utf8proc.cpp 
            ${MegaDir}/src/mega_zxcvbn.cpp 
            ${MegaDir}/src/megaapi.cpp 
            ${MegaDir}/src/megaapi_impl.cpp 
            ${MegaDir}/src/megaclient.cpp 
            ${MegaDir}/src/node.cpp 
            ${MegaDir}/src/pendingcontactrequest.cpp 
            ${MegaDir}/src/proxy.cpp 
            ${MegaDir}/src/pubkeyaction.cpp 
            ${MegaDir}/src/raid.cpp 
            ${MegaDir}/src/request.cpp
            ${MegaDir}/src/serialize64.cpp 
            ${MegaDir}/src/share.cpp 
            ${MegaDir}/src/sharenodekeys.cpp 
            ${MegaDir}/src/sync.cpp 
            ${MegaDir}/src/testhooks.cpp 
            ${MegaDir}/src/transfer.cpp 
            ${MegaDir}/src/transferslot.cpp 
            ${MegaDir}/src/treeproc.cpp 
            ${MegaDir}/src/user.cpp 
            ${MegaDir}/src/useralerts.cpp 
            ${MegaDir}/src/utils.cpp 
            ${MegaDir}/src/waiterbase.cpp 
            ${Mega_PlatformSpecificFiles} ${Mega_CryptoFiles} ${Mega_DbFiles} ${Mega_GfxFiles}  
            $<${USE_LIBUV}:${MegaDir}/src/mega_evt_tls.cpp> 
            $<${USE_QT}:${MegaDir}/src/gfx/qt.cpp>)

target_include_directories(megaio PRIVATE ${MegaDir}/include ${Mega_PlatformSpecificIncludes})
target_include_directories(megaio PUBLIC ${MegaDir}/include ${Mega_PlatformSpecificIncludes})
target_link_libraries(megaio PUBLIC ZLIB::ZLIB 
                $<${USE_CRYPTOPP}:cryptopp::cryptopp-static> 
                $<${USE_SODIUM}:sodium> 
                $<$<AND:${USE_OPENSSL},$<NOT:${USE_WEBRTC}>>:ssl> 
                $<$<AND:${USE_OPENSSL},$<NOT:${USE_WEBRTC}>>:crypto> 
                $<${USE_WEBRTC}:webrtc::webrtc> 
                $<${USE_CURL}:CURL::libcurl>  
                $<${USE_CURL}:c-ares::c-ares> 
                $<${USE_SQLITE}:SQLite::SQLite3>
                $<${USE_MEDIAINFO}:MediaInfoLib::MediaInfoLib> $<${USE_MEDIAINFO}:ZenLib::ZenLib> 
                $<${USE_LIBUV}:uv> 
                $<${USE_FREEIMAGE}:freeimage> $<${USE_FREEIMAGE}:freeimage_IlmImf> $<${USE_FREEIMAGE}:freeimage_IlmImfUtil> $<${USE_FREEIMAGE}:freeimage_IlmThread> $<${USE_FREEIMAGE}:freeimage_Iex> $<${USE_FREEIMAGE}:freeimage_IexMath>
                                              $<${USE_FREEIMAGE}:freeimage_jpeg> $<${USE_FREEIMAGE}:freeimage_turbojpeg> $<${USE_FREEIMAGE}:freeimage_jpegxr> $<${USE_FREEIMAGE}:freeimage_jxrglue> $<${USE_FREEIMAGE}:freeimage_openjp2>
                                              $<${USE_FREEIMAGE}:freeimage_lzma>  $<${USE_FREEIMAGE}:freeimage_lcms2>  $<${USE_FREEIMAGE}:freeimage_raw>  
                                              $<${USE_FREEIMAGE}:freeimage_tiff>  $<${USE_FREEIMAGE}:freeimage_tiffxx>  
                                              $<${USE_FREEIMAGE}:freeimage_jasper>  $<${USE_FREEIMAGE}:freeimage_libpng>  $<${USE_FREEIMAGE}:freeimage_half>  
                                              $<${USE_FREEIMAGE}:freeimage_webp> $<${USE_FREEIMAGE}:freeimage_webpdecoder>  $<${USE_FREEIMAGE}:freeimage_webpdemux>  $<${USE_FREEIMAGE}:freeimage_webpmux>                                               
                $<${HAVE_FFMPEG}:ffmpeg_avformat> $<${HAVE_FFMPEG}:ffmpeg_avcodec> $<${HAVE_FFMPEG}:ffmpeg_avutil> $<${HAVE_FFMPEG}:ffmpeg_avfilter> $<${HAVE_FFMPEG}:ffmpeg_avdevice> $<${HAVE_FFMPEG}:ffmpeg_avdevice > 
                $<${HAVE_FFMPEG}:ffmpeg_swscale>  $<${HAVE_FFMPEG}:ffmpeg_swresample>  
                ${Mega_PlatformSpecificLibs})
target_compile_definitions(megaio PUBLIC 
                $<${USE_MEDIAINFO}:USE_MEDIAINFO> 
                $<${USE_SQLITE}:USE_SQLITE> 
                $<${USE_CRYPTOPP}:USE_CRYPTOPP> 
                $<${USE_OPENSSL}:USE_OPENSSL> 
                $<${USE_CURL}:USE_CURL> 
                $<${USE_SODIUM}:USE_SODIUM>
                $<${ENABLE_SYNC}:ENABLE_SYNC> 
                $<${ENABLE_CHAT}:ENABLE_CHAT> 
                $<${NO_READLINE}:NO_READLINE>
                $<${USE_FREEIMAGE}:USE_FREEIMAGE> 
                $<${HAVE_FFMPEG}:HAVE_FFMPEG>
                $<${HAVE_LIBUV}:HAVE_LIBUV>
                $<${USE_CPPTHREAD}:USE_CPPTHREAD>
                $<${USE_QT}:USE_QT>
                $<${USE_PDFIUM}:HAVE_PDFIUM>)

if(WIN32)
	target_link_libraries(megaio PUBLIC crypt32.lib)
endif(WIN32)


if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4201")  # nameless struct/union (nonstandard)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100")  # unreferenced formal parameter
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4706")  # assignment within conditional
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4458")  # identifier hides class member
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4324")  # structure was padded due to alignment specifier (common in Sodium)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4456")  # declaration hides previous local declaration
    #TODO: remove some of those gradually.  also consider: /wd4503 /wd4996 /wd4702
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wno-unused-parameter")
endif()


#add_library(megaio megaio.cpp)
