add_definitions(-DIU_CLI)

add_executable(CLI 
    main.cpp 
    ConsoleScriptDialogProvider.cpp 
    ConsoleScriptDialogProvider.h
    ../Core/Settings/CliSettings.cpp
)

set_target_properties( CLI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/CLI/)

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(CLI iucore ${LIBS_LIST} ${COMMON_LIBS_LIST})