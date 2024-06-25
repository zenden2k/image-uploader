add_definitions(-DIU_CLI)

if (WIN32)
    set(RESOURCE_LIST "../res/CLI.exe.manifest" CLI.rc)
endif()

add_executable(CLI 
    main.cpp 
    ConsoleScriptDialogProvider.cpp 
    ConsoleScriptDialogProvider.h
    ../Core/Settings/CliSettings.cpp
    ConsoleScriptFunctionsImpl.cpp
    ${RESOURCE_LIST}
)

if (IU_ENABLE_WEBVIEW2)
    set_target_properties(CLI PROPERTIES VS_GLOBAL_WebView2LoaderPreference "Static")
    set_target_properties(CLI PROPERTIES VS_USER_PROPS "${CMAKE_SOURCE_DIR}/ImageUploader.Core.props")
    set_target_properties(CLI PROPERTIES VS_PACKAGE_REFERENCES "${WEBVIEW2_PACKAGES_REFERENCES}")
endif()

set_target_properties(CLI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/CLI/)

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(CLI PRIVATE iucore ${LIBS_LIST} ${COMMON_LIBS_LIST})

if(WIN32)
    add_custom_command(TARGET CLI
        POST_BUILD
        COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/set_binary_version.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" $<TARGET_FILE:CLI> $<CONFIG>
    )
endif()
