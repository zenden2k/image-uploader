set(SRC_LIST    
    HistoryManagerImpl.cpp
)

set(HEADER_LIST 
	HistoryManagerImpl.h
)

set(LIB_LIST SQLite::SQLite3)
    
add_library(iu_history STATIC ${SRC_LIST} ${HEADER_LIST})

target_link_libraries(iu_history 
    iu_core
    ${COMMON_LIBS_LIST}
    ${LIB_LIST}
)

target_compile_definitions(iu_history PRIVATE _WIN7_PLATFORM_UPDATE)

set_target_properties(iu_history PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
