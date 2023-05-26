set(SRC_LIST    
    ScreenRecorder/ScreenRecorder.cpp
)

set(HEADER_LIST 
    ScreenRecorder/ScreenRecorder.h
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

if (WIN32)
    set(LIBS_LIST base-classes::base-classes)
endif()

target_link_libraries(iucapture 
    ${LIBS_LIST} 
    ${COMMON_LIBS_LIST}
)

if(MSVC)
    target_compile_options(iucapture PRIVATE "/MP")
endif()

set_target_properties(iucapture PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)