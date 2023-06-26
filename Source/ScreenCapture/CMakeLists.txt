set(SRC_LIST    
    ScreenRecorder/ScreenRecorder.cpp
    ScreenRecorder/ArgsBuilder/FFmpegInputArgs.cpp
)

set(HEADER_LIST 
    ScreenRecorder/ScreenRecorder.h
    ScreenRecorder/ArgsBuilder/FFmpegArgsBuilder.h
    ScreenRecorder/ArgsBuilder/FFmpegArgs.h
    ScreenRecorder/ArgsBuilder/FFmpegInputArgs.h
    ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h
    ScreenRecorder/VideoCodecs/FFmpegVideoCodec.h
    ScreenRecorder/VideoCodecs/NvencVideoCodec.h
    ScreenRecorder/VideoCodecs/X264VideoCodec.h
    ScreenRecorder/FFmpegSettings.h
    ScreenRecorder/Sources/FFmpegSource.h
    ScreenRecorder/Sources/DDAGrabSource.h
    ScreenRecorder/Sources/GDIGrabSource.h
)
    
if(WIN32)
    SET(OPTIONS WIN32)
    list(APPEND SRC_LIST 
        
    )
    list(APPEND HEADER_LIST 
        
    )
endif()

source_group(TREE "${CMAKE_SOURCE_DIR}" PREFIX "Sources" FILES ${SRC_LIST} ${HEADER_LIST})

add_library(iucapture STATIC ${SRC_LIST} ${HEADER_LIST})

target_link_libraries(iucapture 
    iucore
    ${COMMON_LIBS_LIST}
)

if(MSVC)
    target_compile_options(iucapture PRIVATE "/MP")
endif()

set_target_properties(iucapture PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
