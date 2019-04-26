
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

add_definitions(-DIU_CLI)

add_executable(CLI 
    main.cpp 
    ConsoleScriptDialogProvider.cpp 
    ConsoleScriptDialogProvider.h
    ../Core/Settings/CliSettings.cpp
)

set_target_properties( CLI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/CLI/)

set(LIBS_LIST )

#if(WIN32)
#    list(APPEND LIBS_LIST)
#else()
#    list(APPEND LIBS_LIST)
#endif()

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(CLI iucore ${LIBS_LIST} ${COMMON_LIBS_LIST})

#target_link_libraries(CLI optimized iucore.lib)