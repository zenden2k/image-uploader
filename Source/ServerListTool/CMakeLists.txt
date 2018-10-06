
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

add_definitions(-DIU_SERVERLISTTOOL)

add_executable(ServersChecker WIN32  
    ServerListTool.cpp 
    MainDlg.cpp 
    MainDlg.h
    ServerListTool.rc
    resource.h
    ../Func/MyEngineList.cpp
    ../Func/WtlScriptDialogProvider.cpp
    ../Func/LangClass.cpp
    ../Func/MyUtils.cpp
    ../Func/MyEngineList.cpp
    ../Gui/Dialogs/InputDialog.cpp
    ../Gui/Controls/MyImage.cpp
    ../Gui/Dialogs/LogWindow.cpp
    ../Gui/Controls/LogListBox.cpp
    ../Func/DefaultLogger.cpp
    ../Func/DefaultUploadErrorHandler.cpp
    ../Core/Images/Utils.cpp
    ../3rdpart/QColorQuantizer.cpp
    ../Core/Video/VideoUtils.cpp
    ../Core/FileDownloader.cpp
)

set_target_properties(ServersChecker PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/ServersChecker/)

set(LIBS_LIST base64 libwebpdecoder libwebpdemux)

#if(WIN32)
#    list(APPEND LIBS_LIST)
#else()
#    list(APPEND LIBS_LIST)
#endif()

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(ServersChecker libiucore ${LIBS_LIST} ${COMMON_LIBS_LIST})

#target_link_libraries(CLI optimized iucore.lib)