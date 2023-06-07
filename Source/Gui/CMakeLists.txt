add_definitions(-DIU_WTL_APP -DIU_GUI -DIU_WTL)

set(SRC_LIST 
    "../Image Uploader.cpp"
    ../3rdpart/ShellPidl.cpp
    ../Core/Images/ImageConverterPrivateBase.cpp
    ../Core/Images/ImageConverterPrivate_gdiplus.cpp
    ../Core/ScreenCapture/MonitorEnumerator.cpp
    ../Core/ScreenCapture/ScreenshotHelper.cpp
    ../Core/SearchByImage.cpp
    ../Core/SearchByImageTask.cpp
    ../Core/SearchGoogleImages.cpp
    ../Core/SearchYandexImages.cpp
    ../Core/Settings/CommonGuiSettings.cpp
    ../Core/Settings/WtlGuiSettings.cpp
    ../Core/TempFileDeleter.cpp
    ../Core/FileDownloader.cpp
    ../Core/ScreenCapture.cpp
    ../Core/ServerListManager.cpp
    ../Core/Images/ImageConverter.cpp
    ../Core/Images/Thumbnail.cpp
    ../Core/3rdpart/dxerr.cpp
    ../Core/3rdpart/parser.cpp
    ../Core/Upload/Filters/ImageConverterFilter.cpp
    ../Core/Upload/Filters/SizeExceedFilter.cpp
    ../Core/Upload/Filters/UrlShorteningFilter.cpp
    ../Core/Upload/Filters/UserFilter.cpp
    ../Core/BackgroundTask.cpp
    ../Func/CmdLine.cpp
    ../Func/DefaultLogger.cpp
    ../Func/DefaultUploadErrorHandler.cpp
    ../Func/WtlScriptDialogProvider.cpp
    ../Func/ImageGenerator.cpp
    Components/MyFileDialog.cpp
    Components/NewStyleFileDialog.cpp
    Components/NewStyleFileSaveDialog.cpp
    Components/DragndropOverlay.cpp
    Controls/HyperLinkControlAccessible.cpp
    Controls/ImageView.cpp
    Controls/ResultsListView.cpp
    Controls/ProgressRingControl.cpp
    Dialogs/ClearHistoryDlg.cpp
    Dialogs/FolderAddDlg.cpp
    Dialogs/SearchByImageDlg.cpp
    Win7JumpList.cpp
    GuiTools.cpp
    IconBitmapUtils.cpp
    WizardCommon.cpp
    Dialogs/AboutDlg.cpp
    Dialogs/AddDirectoryServerDialog.cpp
    Dialogs/AddFtpServerDialog.cpp
    Dialogs/ContextMenuItemDlg.cpp
    Dialogs/ColorCodeDialog.cpp
    Dialogs/DefaultServersSettings.cpp
    Dialogs/FloatingWindow.cpp
    Dialogs/GeneralSettings.cpp
    Dialogs/HistoryWindow.cpp
    Dialogs/HotkeyEditor.cpp
    Dialogs/HotkeySettings.cpp
    Dialogs/ImageDownloaderDlg.cpp
    Dialogs/ImageReuploaderDlg.cpp
    Dialogs/InputDialog.cpp
    Dialogs/IntegrationSettings.cpp
    Dialogs/LangSelect.cpp
    Dialogs/LoginDlg.cpp
    Dialogs/LogoSettings.cpp
    Dialogs/LogWindow.cpp
    Dialogs/MainDlg.cpp
    Dialogs/NewFolderDlg.cpp
    Dialogs/QuickSetupDlg.cpp
    Dialogs/RegionSelect.cpp
    Dialogs/ResultsPanel.cpp
    Dialogs/ResultsWindow.cpp
    Dialogs/ScreenshotDlg.cpp
    Dialogs/ScreenshotSettingsPage.cpp
    Dialogs/ServerFolderSelect.cpp
    Dialogs/ServerParamsDlg.cpp
    Dialogs/SettingsDlg.cpp
    Dialogs/settingspage.cpp
    Dialogs/ShortenUrlDlg.cpp
    Dialogs/SizeExceed.cpp
    Dialogs/StatusDlg.cpp
    Dialogs/TextViewDlg.cpp
    Dialogs/ThumbEditor.cpp
    Dialogs/ThumbSettingsPage.cpp
    Dialogs/TraySettings.cpp
    Dialogs/UpdateDlg.cpp
    Dialogs/UploadDlg.cpp
    Dialogs/UploadParamsDlg.cpp
    Dialogs/UploadSettings.cpp
    Dialogs/UploadSettingsPage.cpp
    Dialogs/VideoGrabberPage.cpp
    Dialogs/VideoGrabberParams.cpp
    Dialogs/WebViewWindow.cpp
    Dialogs/WelcomeDlg.cpp
    Dialogs/WizardDlg.cpp
    Dialogs/ServerProfileGroupSelectDialog.cpp
    Controls/CustomEditControl.cpp
    Controls/HistoryTreeControl.cpp
    Controls/HyperLinkControl.cpp
    Controls/IconButton.cpp
    Controls/LogListBox.cpp
    Controls/MyImage.cpp
    Controls/PictureExWnd.cpp
    Controls/ServerSelectorControl.cpp
    Controls/MultiServerSelectorControl.cpp
    Controls/TabListBox.cpp
    Controls/ThumbsView.cpp
    Controls/WTLBrowserView.cpp
    Helpers/LangHelper.cpp
    ../3rdpart/ColorButton.cpp
    ../3rdpart/MemberFunctionCallback.cpp
    ../3rdpart/vkCodes.cpp
    ../3rdpart/WinHotkeyCtrl.cpp
    ../Func/Common.cpp
    ../Func/ImageEditorConfigurationProvider.cpp
    ../Func/IuCommonFunctions.cpp
    ../Func/LangClass.cpp
    ../Func/MyDataObject.cpp
    ../Func/MyDropSource.cpp
    ../Func/MyEngineList.cpp
    ../Func/MyUtils.cpp
    ../Func/SystemUtils.cpp
    ../Func/WebUtils.cpp
    ../ImageEditor/Canvas.cpp
    ../ImageEditor/Document.cpp
    ../ImageEditor/DrawingElement.cpp
    ../ImageEditor/DrawingTool.cpp
    ../ImageEditor/MovableElement.cpp
    ../ImageEditor/MovableElements.cpp
    ../ImageEditor/Region.cpp
    ../ImageEditor/Gui/ColorsDelegate.cpp
    ../ImageEditor/Gui/ImageEditorView.cpp
    ../ImageEditor/Gui/ImageEditorWindow.cpp
    ../ImageEditor/Gui/InputBoxControl.cpp
    ../ImageEditor/Gui/TextParamsWindow.cpp
    ../ImageEditor/Gui/Toolbar.cpp
    ../ImageEditor/DrawingTools/AbstractDrawingTool.cpp
    ../ImageEditor/DrawingTools/BlurTool.cpp
    ../ImageEditor/DrawingTools/BrushTool.cpp
    ../ImageEditor/DrawingTools/ColorPickerTool.cpp
    ../ImageEditor/DrawingTools/CropTool.cpp
    ../ImageEditor/DrawingTools/MarkerTool.cpp
    ../ImageEditor/DrawingTools/MoveAndResizeTool.cpp
    ../ImageEditor/DrawingTools/PenTool.cpp
    ../ImageEditor/DrawingTools/SelectionTool.cpp
    ../ImageEditor/DrawingTools/TextTool.cpp
    ../ImageEditor/DrawingTools/VectorElementTool.cpp
    ../ImageEditor/Helpers/FontEnumerator.cpp
    UploadListModel.cpp
    ../ServerListTool/ServersCheckerDlg.cpp 
    ../ServerListTool/ServersChecker.cpp
    ../ServerListTool/ServersCheckerModel.cpp
    ../ServerListTool/Helpers.cpp
    ../ServerListTool/ServerListView.cpp
)

set(RESOURCE_LIST  "../Image Uploader.rc"
    "../res/image uploader.exe.manifest")
    
set(HEADER_LIST 
    ../3rdpart/ShellPidl.h
    ../Core/Images/ImageConverterPrivateBase.h
    ../Core/Images/ImageConverterPrivate_gdiplus.h
    ../Core/ScreenCapture/MonitorEnumerator.h
    ../Core/ScreenCapture/ScreenshotHelper.h
    ../Core/Scripting/Script.h
    ../Core/Scripting/ScriptsManager.h
    ../Core/SearchByImage.h
    ../Core/SearchByImageTask.h
    ../Core/SearchGoogleImages.h
    ../Core/SearchYandexImages.h
    ../Core/Settings/CommonGuiSettings.h
    ../Core/Settings/WtlGuiSettings.h
    ../Core/TempFileDeleter.h
    ../Core/FileDownloader.h
    ../Core/ScreenCapture.h
    ../Core/ServerListManager.h
    ../Core/Images/ImageConverter.h
    ../Core/Images/Thumbnail.h
    ../Core/Images/Utils.h
    ../Core/3rdpart/dxerr.h
    ../Core/3rdpart/parser.h
    ../Core/Upload/Filters/ImageConverterFilter.h
    ../Core/Upload/Filters/SizeExceedFilter.h
    ../Core/Upload/Filters/UrlShorteningFilter.h
    ../Core/Upload/Filters/UserFilter.h
    ../Func/CmdLine.h
    ../Func/DefaultLogger.h
    ../Func/DefaultUploadErrorHandler.h
    ../Func/GdiPlusInitializer.h
    ../Func/WtlScriptDialogProvider.h
    ../Func/ImageGenerator.h
    Components/MyFileDialog.h
    Components/NewStyleFileDialog.h
    Components/NewStyleFileSaveDialog.h
    Components/DragndropOverlay.cpp
    Controls/HyperLinkControlAccessible.h
    Controls/ImageView.h
    Controls/ResultsListView.h
    Controls/ProgressRingControl.h
	Controls/CustomTreeControl.h
    Dialogs/ClearHistoryDlg.h
    Dialogs/FolderAddDlg.h
    Dialogs/SearchByImageDlg.h
    Dialogs/ColorCodeDialog.h
    Win7JumpList.h
    GuiTools.h
    IconBitmapUtils.h
    WizardCommon.h
    Dialogs/AboutDlg.h
    Dialogs/AddDirectoryServerDialog.h
    Dialogs/AddFtpServerDialog.h
    Dialogs/ContextMenuItemDlg.h
    Dialogs/DefaultServersSettings.h
    Dialogs/FloatingWindow.h
    Dialogs/GeneralSettings.h
    Dialogs/HistoryWindow.h
    Dialogs/HotkeyEditor.h
    Dialogs/HotkeySettings.h
    Dialogs/ImageDownloaderDlg.h
    Dialogs/ImageReuploaderDlg.h
    Dialogs/InputDialog.h
    Dialogs/IntegrationSettings.h
    Dialogs/LangSelect.h
    Dialogs/LoginDlg.h
    Dialogs/LogoSettings.h
    Dialogs/LogWindow.h
    Dialogs/MainDlg.h
    Dialogs/NewFolderDlg.h
    Dialogs/QuickSetupDlg.h
    Dialogs/RegionSelect.h
    Dialogs/ResultsPanel.h
    Dialogs/ResultsWindow.h
    Dialogs/ScreenshotDlg.h
    Dialogs/ScreenshotSettingsPage.h
    Dialogs/ServerFolderSelect.h
    Dialogs/ServerParamsDlg.h
    Dialogs/SettingsDlg.h
    Dialogs/settingspage.h
    Dialogs/ShortenUrlDlg.h
    Dialogs/SizeExceed.h
    Dialogs/StatusDlg.h
    Dialogs/TextViewDlg.h
    Dialogs/ThumbEditor.h
    Dialogs/ThumbSettingsPage.h
    Dialogs/TraySettings.h
    Dialogs/UpdateDlg.h
    Dialogs/UploadDlg.h
    Dialogs/UploadParamsDlg.h
    Dialogs/UploadSettings.h
    Dialogs/UploadSettingsPage.h
    Dialogs/VideoGrabberPage.h
    Dialogs/VideoGrabberParams.h
    Dialogs/WebViewWindow.h
    Dialogs/WelcomeDlg.h
    Dialogs/WizardDlg.h
    Dialogs/ServerProfileGroupSelectDialog.h
    Controls/CustomEditControl.h
    Controls/HistoryTreeControl.h
    Controls/HyperLinkControl.h
    Controls/IconButton.h
    Controls/LogListBox.h
    Controls/MyImage.h
    Controls/PictureExWnd.h
    Controls/ServerSelectorControl.h
    Controls/MultiServerSelectorControl.h
    Controls/TabListBox.h
    Controls/ThumbsView.h
    Controls/WTLBrowserView.h
    Helpers/LangHelper.h
    ../3rdpart/ColorButton.h
    ../3rdpart/MemberFunctionCallback.h
    ../3rdpart/QColorQuantizer.h
    ../3rdpart/Registry.h
    ../3rdpart/Unzipper.h
    ../3rdpart/vkCodes.h
    ../3rdpart/WinHotkeyCtrl.h
    ../Func/Common.h
    ../Func/ImageEditorConfigurationProvider.h
    ../Func/IuCommonFunctions.h
    ../Func/LangClass.h
    ../Func/MyDataObject.h
    ../Func/MyDropSource.h
    ../Func/MyEngineList.h
    ../Func/MyUtils.h
    ../Func/SystemUtils.h
    ../Func/WebUtils.h
    ../ImageEditor/Canvas.h
    ../ImageEditor/Document.h
    ../ImageEditor/DrawingElement.h
    ../ImageEditor/DrawingTool.h
    ../ImageEditor/MovableElement.h
    ../ImageEditor/MovableElements.h
    ../ImageEditor/Region.h
    ../ImageEditor/Gui/ColorsDelegate.h
    ../ImageEditor/Gui/ImageEditorView.h
    ../ImageEditor/Gui/ImageEditorWindow.h
    ../ImageEditor/Gui/InputBoxControl.h
    ../ImageEditor/Gui/TextParamsWindow.h
    ../ImageEditor/Gui/Toolbar.h
    ../ImageEditor/DrawingTools/AbstractDrawingTool.h
    ../ImageEditor/DrawingTools/BlurTool.h
    ../ImageEditor/DrawingTools/BrushTool.h
    ../ImageEditor/DrawingTools/ColorPickerTool.h
    ../ImageEditor/DrawingTools/CropTool.h
    ../ImageEditor/DrawingTools/MarkerTool.h
    ../ImageEditor/DrawingTools/MoveAndResizeTool.h
    ../ImageEditor/DrawingTools/PenTool.h
    ../ImageEditor/DrawingTools/SelectionTool.h
    ../ImageEditor/DrawingTools/TextTool.h
    ../ImageEditor/DrawingTools/VectorElementTool.h
    ../ImageEditor/Helpers/FontEnumerator.h
    UploadListModel.h
    ../ServerListTool/ServersChecker.h
    ../ServerListTool/ServersCheckerModel.h
    ../ServerListTool/Helpers.h
    ../ServerListTool/ServerListView.h
    ../resource.h
)
	
if (IU_ENABLE_MEDIAINFO) 
	list (APPEND SRC_LIST ../Func/MediaInfoHelper.cpp  Dialogs/MediaInfoDlg.cpp)
	list (APPEND HEADER_LIST ../Func/MediaInfoHelper.h  Dialogs/MediaInfoDlg.h)
endif()

source_group(TREE "${CMAKE_SOURCE_DIR}" PREFIX "Sources" FILES ${SRC_LIST} ${HEADER_LIST})
source_group("Resource Files" FILES ${RESOURCE_LIST})

add_executable(GUI WIN32  
    ${SRC_LIST} ${HEADER_LIST} ${RESOURCE_LIST}
)

set_target_properties(GUI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/GUI/)

set(LIBS_LIST 
    gdiplus Ws2_32 dwmapi wininet IPHLPAPI
    Secur32 Netapi32 Winhttp Userenv Psapi uxtheme Urlmon
    minizip::minizip
)

if (IU_ENABLE_MEDIAINFO)
	list(APPEND LIBS_LIST MediaInfoLib::MediaInfoLib)
endif()	

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(GUI iucore iuvideo
    ${LIBS_LIST} 
    ${COMMON_LIBS_LIST}
)
  
set_target_properties(GUI PROPERTIES OUTPUT_NAME "Image Uploader")

if (WIN32 AND IU_ENABLE_FFMPEG AND IU_FFMPEG_STANDALONE)
    set_target_properties(GUI PROPERTIES LINK_FLAGS "/DELAYLOAD:avcodec-58.dll /DELAYLOAD:avformat-58.dll /DELAYLOAD:avutil-56.dll /DELAYLOAD:swscale-5.dll")
endif()

target_precompile_headers(GUI PRIVATE ../stdafx.h)

add_custom_command(TARGET GUI
    PRE_BUILD
    COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/versioninc.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" $<CONFIG>
)
add_custom_command(TARGET GUI
    POST_BUILD
    COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/set_binary_version.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" $<TARGET_FILE:GUI> $<CONFIG>
)
