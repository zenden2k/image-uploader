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

add_definitions(-DCURL_STATICLIB -DPCRE_STATIC -DTIXML_USE_STL -DIU_CLI -DGOOGLE_GLOG_DLL_DECL= -DENABLE_SYNC)


if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include)
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

add_executable(CLI 
    main.cpp 
    ConsoleScriptDialogProvider.cpp 
    ConsoleScriptDialogProvider.h
)

set_target_properties( CLI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/CLI/)

set(LIBS_LIST libiucore squirrel sqstdlib jsoncpp pcre gumbo mega uriparser cryptopp cares libuv)


if(WIN32)
    list(APPEND LIBS_LIST libcurl libeay32 ssleay32 Oleacc libglog_static minizip Userenv Psapi Iphlpapi Winhttp)
else()
    list(APPEND LIBS_LIST curl ssl crypto glog pthread boost_filesystem boost_system ncurses)
endif()

target_link_libraries(CLI ${LIBS_LIST})

#target_link_libraries(CLI optimized iucore.lib)