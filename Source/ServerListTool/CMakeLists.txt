add_definitions(-DIU_SERVERLISTTOOL -DIU_WTL)

add_executable(ServersChecker WIN32  
    ServerListTool.cpp 
    MainDlg.cpp 
    MainDlg.h
    ServersCheckerSettingsDlg.cpp
    ServersCheckerSettings.cpp
    ServerListTool.rc
    resource.h
    ServersChecker.cpp
    ServersCheckerModel.cpp
    Helpers.cpp
    ServerListView.cpp
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
    ../3rdpart/QColorQuantizer.cpp
    ../Core/FileDownloader.cpp
    ../Core/Settings/CliSettings.cpp
	../Gui/Helpers/LangHelper.cpp
	../Gui/Components/MyFileDialog.cpp
	../Gui/Components/NewStyleFileDialog.cpp
	../Gui/Components/NewStyleFileSaveDialog.cpp
    res/ServersChecker.exe.manifest
)

set_target_properties(ServersChecker PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/ServersChecker/)

set(LIBS_LIST WebP::webpdemux WebP::webpdecoder)

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(ServersChecker iucore ${LIBS_LIST} ${COMMON_LIBS_LIST})