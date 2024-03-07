set(SRC_LIST    
    VideoGrabber.cpp
    FrameGrabberException.cpp
    AbstractFrameGrabber.cpp
    AbstractVideoFrame.cpp
)

set(HEADER_LIST 
    VideoGrabber.h
    FrameGrabberException.h
    AbstractFrameGrabber.h
    AbstractVideoFrame.h
)
    
if(WIN32)
    SET(OPTIONS WIN32)
    list(APPEND SRC_LIST 
        DirectshowFrameGrabber.cpp
        DirectshowFrameGrabber2.cpp
        DirectshowVideoFrame.cpp
        NoDirectVobSub.cpp
        GrabFilter.cpp
        DirectShowUtil.cpp
        ../Core/3rdpart/dxerr.cpp
    )
    list(APPEND HEADER_LIST 
        DirectshowFrameGrabber.h
        DirectshowFrameGrabber2.h
        DirectshowVideoFrame.h
        NoDirectVobSub.h
        GrabFilter.h
        DirectShowUtil.h
        ../Core/3rdpart/dxerr.h
    )
endif()

if (IU_ENABLE_FFMPEG)
	list(APPEND SRC_LIST AvcodecFrameGrabber.cpp)
    list(APPEND HEADER_LIST AvcodecFrameGrabber.h)
endif()	

source_group(TREE "${CMAKE_SOURCE_DIR}" PREFIX "Sources" FILES ${SRC_LIST} ${HEADER_LIST})

add_library(iuvideo STATIC ${SRC_LIST} ${HEADER_LIST})

target_include_directories(iuvideo PRIVATE ${CMAKE_SOURCE_DIR}/../Contrib/Include/DX/)

#if (WIN32)
#    set(LIBS_LIST base-classes::base-classes)
#endif()

if (IU_ENABLE_FFMPEG)
    if(NOT IU_FFMPEG_STANDALONE)
        target_link_libraries(iuvideo PRIVATE ffmpeg::avcodec ffmpeg::avformat ffmpeg::avutil ffmpeg::swscale)
    else()
	    target_link_libraries(iuvideo PUBLIC avcodec avformat avutil swscale)
    endif()	
endif()	

if (WIN32)
    target_link_libraries(iuvideo PUBLIC base-classes::base-classes Winmm.lib)
endif()

target_link_libraries(iuvideo PUBLIC
    iucore
)

target_link_libraries(iuvideo PRIVATE
    ${COMMON_LIBS_LIST}
)

if(MSVC)
#    target_compile_options(iuvideo PRIVATE "/MP")
endif()

set_target_properties(iuvideo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)