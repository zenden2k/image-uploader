set(SRC_LIST    
    ScreenRecorder/ScreenRecorder.cpp
    ScreenRecorder/FFmpegScreenRecorder.cpp
    ScreenRecorder/FFmpegOptionsManager.cpp
    ScreenRecorder/ArgsBuilder/FFmpegInputArgs.cpp
)

set(HEADER_LIST 
    ScreenRecorder/Common.h
    ScreenRecorder/ScreenRecorder.h
    ScreenRecorder/FFmpegScreenRecorder.h
    ScreenRecorder/FFmpegOptionsManager.h
    ScreenRecorder/FFmpegOptions.h
    ScreenRecorder/FFmpegCodec.h
    ScreenRecorder/ArgsBuilder/FFmpegArgsBuilder.h
    ScreenRecorder/ArgsBuilder/FFmpegArgs.h
    ScreenRecorder/ArgsBuilder/FFmpegInputArgs.h
    ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h
    ScreenRecorder/VideoCodecs/FFmpegVideoCodec.h
    ScreenRecorder/VideoCodecs/NvencVideoCodec.h
    ScreenRecorder/VideoCodecs/QsvVideoCodec.h
    ScreenRecorder/VideoCodecs/AmfVideoCodec.h
    ScreenRecorder/VideoCodecs/X264VideoCodec.h
    ScreenRecorder/VideoCodecs/VP9VideoCodec.h
    ScreenRecorder/VideoCodecs/GifVideoCodec.h
    ScreenRecorder/AudioCodecs/AacAudioCodec.h
    ScreenRecorder/AudioCodecs/Mp3AudioCodec.h
    ScreenRecorder/Sources/FFmpegSource.h
    ScreenRecorder/Sources/DDAGrabSource.h
    ScreenRecorder/Sources/GDIGrabSource.h
    ScreenRecorder/Sources/DirectShowSource.h
)

set(LIB_LIST )
    
if(WIN32)
    SET(OPTIONS WIN32)
    list(APPEND SRC_LIST 
        ScreenRecorder/DXGIScreenRecorder.cpp
        ScreenRecorder/DXGIOptionsManager.cpp
    )
    list(APPEND HEADER_LIST 
        ScreenRecorder/DXGIScreenRecorder.h
        ScreenRecorder/DXGIOptionsManager.h
        ScreenRecorder/DXGIOptions.h
        3rdpart/capture.hpp
    )
     list(APPEND LIB_LIST 
        d3d11 Mfplat mfreadwrite mfuuid strmiids windowscodecs propsys wmcodecdspuuid dxguid d2d1 Mf dxgi
    )

endif()

source_group(TREE "${CMAKE_SOURCE_DIR}" PREFIX "Sources" FILES ${SRC_LIST} ${HEADER_LIST})

add_library(iucapture STATIC ${SRC_LIST} ${HEADER_LIST})

target_link_libraries(iucapture 
    iucore
    ${COMMON_LIBS_LIST}
    ${LIB_LIST}
)
target_compile_definitions(iucapture PRIVATE _WIN7_PLATFORM_UPDATE)

if(MSVC)
    target_compile_options(iucapture PRIVATE "/MP")
endif()

set_target_properties(iucapture PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
