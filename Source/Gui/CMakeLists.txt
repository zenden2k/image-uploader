add_definitions(-DIU_WTL_APP -DIU_GUI -DIU_WTL -DGDIPVER=0x0100 -D_WTL_NO_CSTRING -D_ATL_NO_OLD_NAMES)

include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/DX/)
include_directories(${CMAKE_SOURCE_DIR}/../Contrib/Include/BaseClasses/)   

set(SRC_LIST 
    "../Image Uploader.cpp"
    ../3rdpart/ShellPidl.cpp
    ../Core/Images/ImageConverterPrivateBase.cpp
    ../Core/Images/ImageConverterPrivate_gdiplus.cpp
    ../Core/ScreenCapture/Utils.cpp
    ../Core/Scripting/Script.cpp
    ../Core/Scripting/ScriptsManager.cpp
    ../Core/SearchByImage.cpp
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
    ../Core/Images/Utils.cpp
    ../Core/3rdpart/dxerr.cpp
    ../Core/3rdpart/parser.cpp
    ../Core/Video/AbstractFrameGrabber.cpp
    ../Core/Video/AbstractImage.cpp
    ../Core/Video/AbstractVideoFrame.cpp
    ../Core/Video/AvcodecFrameGrabber.cpp
    ../Core/Video/DirectshowFrameGrabber.cpp
    ../Core/Video/DirectShowUtil.cpp
    ../Core/Video/GdiPlusImage.cpp
    ../Core/Video/VideoGrabber.cpp
    ../Core/Video/VideoUtils.cpp
    ../Core/Upload/Filters/ImageConverterFilter.cpp
    ../Core/Upload/Filters/SizeExceedFilter.cpp
    ../Core/Upload/Filters/UrlShorteningFilter.cpp
    ../Core/Upload/Filters/UserFilter.cpp
    ../Func/CmdLine.cpp
    ../Func/DefaultLogger.cpp
    ../Func/DefaultUploadErrorHandler.cpp
    ../Func/GdiPlusInitializer.cpp
    ../Func/MediaInfoHelper.cpp
    ../Func/WtlScriptDialogProvider.cpp
    Components/MyFileDialog.cpp
    Components/NewStyleFileDialog.cpp
    Components/NewStyleFileSaveDialog.cpp
    Components/OldStyleFileDialog.cpp
    Controls/HyperLinkControlAccessible.cpp
    Controls/ImageView.cpp
    Controls/ResultsListView.cpp
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
    Dialogs/MediaInfoDlg.cpp
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
    Controls/CustomEditControl.cpp
    Controls/HistoryTreeControl.cpp
    Controls/HyperLinkControl.cpp
    Controls/IconButton.cpp
    Controls/LogListBox.cpp
    Controls/MyImage.cpp
    Controls/PercentEdit.cpp
    Controls/PictureExWnd.cpp
    Controls/ServerSelectorControl.cpp
    Controls/TabListBox.cpp
    Controls/ThumbsView.cpp
    Controls/WTLBrowserView.cpp
    ../3rdpart/ColorButton.cpp
    ../3rdpart/MemberFunctionCallback.cpp
    ../3rdpart/QColorQuantizer.cpp
    ../3rdpart/Registry.cpp
    ../3rdpart/Unzipper.cpp
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
    ../Func/UpdatePackage.cpp
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
    "../Image Uploader.rc"
    "../res/image uploader.exe.manifest"
)

source_group(TREE "${CMAKE_SOURCE_DIR}" PREFIX "Sources" FILES ${SRC_LIST})

add_executable(GUI WIN32  
    ${SRC_LIST}
)

set_target_properties(GUI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/GUI/)

set(LIBS_LIST base64 libwebpdecoder libwebpdemux 
    gdiplus Ws2_32 avcodec avformat avutil swscale dwmapi winmm wininet IPHLPAPI
    Secur32 Netapi32 Oleacc Winhttp Userenv Psapi
)

message("COMMON_LIBS_LIST libraries: ${COMMON_LIBS_LIST}")

target_link_libraries(GUI iucore 
    ${LIBS_LIST} 
    ${COMMON_LIBS_LIST}
    debug strmbasd optimized strmbase
)

set_target_properties(GUI PROPERTIES LINK_FLAGS "/DELAYLOAD:uxtheme.dll /DELAYLOAD:dwmapi.dll /DELAYLOAD:avcodec-58.dll /DELAYLOAD:avformat-58.dll /DELAYLOAD:avutil-56.dll /DELAYLOAD:swscale-5.dll /DELAYLOAD:Iphlpapi.dll /DELAYLOAD:Netapi32.dll")

add_custom_command(TARGET GUI
    PRE_BUILD
    COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/versioninc.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" $<CONFIG:Release>
)
add_custom_command(TARGET GUI
    POST_BUILD
    COMMAND python "${CMAKE_SOURCE_DIR}/../Utils/Version/set_binary_version.py" "${CMAKE_SOURCE_DIR}/versioninfo.h" "$(TargetDir)$(TargetFileName)" $<CONFIG:Release>
)