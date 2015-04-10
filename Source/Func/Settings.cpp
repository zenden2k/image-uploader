/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "Func/Settings.h"
#ifdef _WIN32
#include <atlheaders.h>
#include <Shlobj.h>
#endif
#include "Core/SettingsManager.h"
#if !defined(IU_CLI) && !defined(IU_IMAGEEDITOR)
#include "Func/myutils.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Common/CmdLine.h"
#include "3rdpart/Registry.h"
#include <Core/Video/VideoUtils.h>
#include "WinUtils.h"
#include <Core/AppParams.h>
#include <Core/Images/Utils.h>
#endif
#include <stdlib.h>

#include <Core/Utils/StringUtils.h>

#if !defined(IU_SERVERLISTTOOL) && !defined(IU_CLI) && !defined(IU_IMAGEEDITOR)
	#include "Gui/Dialogs/FloatingWindow.h"
#endif

#ifndef CheckBounds
	#define CheckBounds(n, a, b, d) {if ((n < a) || (n > b)) n = d; }
#endif

#define SETTINGS_FILE_NAME _T("settings.xml")

#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL) && !defined(IU_IMAGEEDITOR)
 const TCHAR CSettings::VideoEngineDirectshow[] = _T("Directshow");
 const TCHAR CSettings::VideoEngineFFmpeg[]     = _T("FFmpeg");
 const TCHAR CSettings::VideoEngineAuto[]       = _T("Auto");

 COLORREF CSettings::DefaultLinkColor = RGB(0x0C,0x32, 0x50);
#endif
/* CString support for  SettingsManager */
#ifdef _WIN32
inline std::string myToString(const CString& value)
{
	return IuCoreUtils::WstringToUtf8((LPCTSTR)value);
}

inline void myFromString(const std::string& text, CString& value)
{
	value = IuCoreUtils::Utf8ToWstring(text).c_str();
}
#ifndef IU_CLI

inline std::string myToString(const Gdiplus::Color& value)
{
	char buffer[30];
	sprintf(buffer,"rgba(%d,%d,%d,%1.4f)", (int)value.GetR(), (int)value.GetG(), (int)value.GetB(), (float)(value.GetA()/255.0));
	return buffer;
}

inline void myFromString(const std::string& text, Gdiplus::Color& value)
{
	value = StringToColor(text);
}
#endif
#endif

template<class T> std::string myToString(const EnumWrapper<T>& value)
{
	return IuCoreUtils::toString (value.value_);
}

template<class T> void myFromString(const std::string& text,  EnumWrapper<T>& value)
{
	value = static_cast<T>(atoi(text.c_str()));
}

#if !defined(IU_CLI) && !defined(IU_SHELLEXT)


ServerProfile::ServerProfile() {
			UseDefaultSettings = true;
}

ServerProfile::ServerProfile(CString newServerName){
	serverName_ = newServerName;
			UseDefaultSettings = true;
}

void ServerProfile::setProfileName(CString newProfileName) {
	profileName_ = newProfileName;
			UseDefaultSettings = true;
}

CString ServerProfile::profileName() const {
	return profileName_;
}

void ServerProfile::setServerName(CString newServerName){
	serverName_ = newServerName;
}

CString ServerProfile::serverName() const {
	return serverName_;
}

std::string ServerProfile::folderTitle() const
{
	return folderTitle_;
}

void ServerProfile::setFolderTitle(std::string newTitle)
{
	folderTitle_ = newTitle;
}

std::string ServerProfile::folderId() const
{
	return folderId_;
}

void ServerProfile::setFolderId(std::string newId)
{
	folderId_ = newId;
}

std::string ServerProfile::folderUrl() const
{
	return folderUrl_;
}

void ServerProfile::setFolderUrl(std::string newUrl)
{
	folderUrl_ = newUrl;
}

bool ServerProfile::isNull()
{
	return serverName_.IsEmpty();
}

void ServerProfile::clearFolderInfo()
{
	folderUrl_.clear();
	folderTitle_.clear();
	folderId_.clear();
}

#ifndef IU_SERVERLISTTOOL
void ServerProfile::bind(SettingsNode& serverNode)
{
	serverNode["@Name"].bind(serverName_);
	serverNode["@FolderId"].bind(folderId_);
	//MessageBoxA(0,folderTitle_.c_str(),0,0);
	serverNode["@FolderTitle"].bind(folderTitle_);
	serverNode["@FolderUrl"].bind(folderUrl_);
	serverNode["@ProfileName"].bind(profileName_);
	serverNode["@UseDefaultSettings"].bind(UseDefaultSettings);
	imageUploadParams.bind(serverNode);
}
#endif

ImageUploadParams ServerProfile::getImageUploadParams()
{
	#ifndef IU_SERVERLISTTOOL
	if ( UseDefaultSettings && &Settings.imageServer  != this) {
		return Settings.imageServer.imageUploadParams;
	}
#endif
	return imageUploadParams;
}

ImageUploadParams& ServerProfile::getImageUploadParamsRef()
{
	return imageUploadParams;
}

void ServerProfile::setImageUploadParams(ImageUploadParams iup)
{
	imageUploadParams = iup;
}

ServerSettingsStruct& ServerProfile::serverSettings() {
	ServerSettingsStruct& res = Settings.ServersSettings[WCstringToUtf8((LPCTSTR)serverName_)][WCstringToUtf8((LPCTSTR)profileName_)];
	res.setParam("FolderID", folderId_);
	res.setParam("FolderUrl", folderUrl_);
	res.setParam("FolderTitle", folderTitle_);
	return res;
}

CUploadEngineData* ServerProfile::uploadEngineData() const {
	return _EngineList->byName(serverName_);
}


/* LOGFONT serialization support */
inline std::string myToString(const LOGFONT& value) {
	CString res;
	FontToString( &value, res );
	return WCstringToUtf8(res);
}

inline void myFromString(const std::string& text, LOGFONT& value) {
	CString wide_text = Utf8ToWCstring(text);
	LOGFONT font;
	StringToFont(wide_text, &font);
	value = font;
}

inline std::string myToString(const CEncodedPassword& value) {
	return WCstringToUtf8(value.toEncodedData());
}

inline void myFromString(const std::string& text, CEncodedPassword& value) {
	value.fromEncodedData(Utf8ToWCstring(text));
}

inline std::string myToString(const CHotkeyList& value) {
	return WCstringToUtf8(value.toString());
}

inline void myFromString(const std::string& text, CHotkeyList& value) {
	value.DeSerialize(Utf8ToWCstring(text));
}

#endif

CSettings Settings;

#if !defined  (IU_CLI)
void RunIuElevated(CString params) {
	SHELLEXECUTEINFO TempInfo = {0};

	TCHAR buf[MAX_PATH];
	GetModuleFileName( 0, buf, MAX_PATH - 1 );
	CString s = WinUtils::GetAppFolder();

	CString Command = CString(buf);
	TempInfo.cbSize       = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask        = 0;
	TempInfo.hwnd         = NULL;
	TempInfo.lpVerb       = _T("runas");
	TempInfo.lpFile       = Command;
	TempInfo.lpParameters = params;
	TempInfo.lpDirectory  = s;
	TempInfo.nShow = SW_NORMAL;

	::ShellExecuteEx(&TempInfo);
}
#endif
#if !defined(IU_SERVERLISTTOOL) && !defined  (IU_CLI) && !defined(IU_SHELLEXT)

/*
	This function starts a new process of Image Uploader with admin rights (Windows Vista and later)
	The created process registers shell extensions and terminates
*/
void ApplyRegistrySettings() {
	SHELLEXECUTEINFO TempInfo = {0};

	TCHAR buf[MAX_PATH];
	GetModuleFileName( 0, buf, MAX_PATH - 1 );
	CString s = WinUtils::GetAppFolder();

	CString Command = CString(buf);
	TempInfo.cbSize       = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask        = 0;
	TempInfo.hwnd         = NULL;
	TempInfo.lpVerb       = _T("runas");
	TempInfo.lpFile       = Command;
	TempInfo.lpParameters = _T(" /integration");
	TempInfo.lpDirectory  = s;
	TempInfo.nShow        = SW_NORMAL;

	::ShellExecuteEx( &TempInfo );
}
#endif

#if !defined(IU_CLI) &&!defined(IU_SERVERLISTTOOL)
CString CSettings::getShellExtensionFileName() const {
	CString file = WinUtils::GetAppFolder() + (WinUtils::IsWindows64Bit() ? _T("ExplorerIntegration64.dll") : _T("ExplorerIntegration.dll"));
	return file;
}

void RegisterShellExtension(bool Register) {
	CString moduleName = Settings.getShellExtensionFileName();
	if ( !FileExists( moduleName ) ) {
		return;
	}

	CRegistry Reg;
	Reg.SetRootKey( HKEY_LOCAL_MACHINE );

	bool canCreateRegistryKey = Register;
	
	if ( Reg.SetKey( "Software\\Zenden.ws\\Image Uploader", canCreateRegistryKey ) ) {
		Reg.WriteBool( "ExplorerContextMenu", Register );
	}

	SHELLEXECUTEINFO TempInfo = {0};
	CString s = WinUtils::GetAppFolder();
	TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask  = SEE_MASK_NOCLOSEPROCESS;
	TempInfo.hwnd   = NULL;
	BOOL b = FALSE;
	IsElevated( &b );
	if ( IsVista() && !b ) {
		TempInfo.lpVerb = _T("runas");
	} else {
		TempInfo.lpVerb = _T("open");
	}
	TempInfo.lpFile       = _T("regsvr32");
	TempInfo.lpParameters = CString((Register ? _T("") : _T("/u "))) + _T("/s \"") + moduleName + _T("\"");
	TempInfo.lpDirectory  = s;
	TempInfo.nShow        = SW_NORMAL;
	//MessageBox(0,TempInfo.lpParameters,0,0);
	::ShellExecuteEx(&TempInfo);
	WaitForSingleObject( TempInfo.hProcess, INFINITE );
	CloseHandle( TempInfo.hProcess );
}


/*
	Determine where data folder is situated
	and store it's path into DataFolder member
*/
void CSettings::FindDataFolder()
{
	AppParams* params = AppParams::instance();
	if (IsDirectory(WinUtils::GetAppFolder() + _T("Data"))) {
		DataFolder     = WinUtils::GetAppFolder() + _T("Data\\");
		SettingsFolder = IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(DataFolder));
		
		params->setDataDirectory(IuStringUtils::Replace(IuCoreUtils::WstringToUtf8((LPCTSTR)DataFolder), "\\", "/"));
		params->setSettingsDirectory(IuStringUtils::Replace(SettingsFolder, "\\", "/"));
		IsPortable = true;
		return;
	}

	SettingsFolder =  IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(GetApplicationDataPath() + _T("Image Uploader\\")));
	
	params->setSettingsDirectory(IuStringUtils::Replace(SettingsFolder, "\\", "/"));
	#if !defined(IU_SERVERLISTTOOL) && !defined  (IU_CLI) && !defined(IU_SHELLEXT)
	{
		CRegistry Reg;
		CString lang;

		Reg.SetRootKey(HKEY_CURRENT_USER);
		if (Reg.SetKey("Software\\Zenden.ws\\Image Uploader", false))
		{
			CString dir = Reg.ReadString("DataPath");

			if (!dir.IsEmpty() && IsDirectory(dir))
			{
				DataFolder = dir;
				params->setDataDirectory(IuStringUtils::Replace(IuCoreUtils::WstringToUtf8((LPCTSTR)DataFolder), "\\", "/"));
				return;
			}
		}
	}
	{
		CRegistry Reg;
		Reg.SetRootKey(HKEY_LOCAL_MACHINE);
		if (Reg.SetKey("Software\\Zenden.ws\\Image Uploader", false))
		{
			CString dir = Reg.ReadString("DataPath");

			if (!dir.IsEmpty() && IsDirectory(dir))
			{
				DataFolder = dir;
				params->setDataDirectory(IuStringUtils::Replace(IuCoreUtils::WstringToUtf8((LPCTSTR)DataFolder), "\\", "/"));
				return;
			}
		}
	}

	if (FileExists(GetCommonApplicationDataPath() + SETTINGS_FILE_NAME)) {
		DataFolder = GetCommonApplicationDataPath() + _T("Image Uploader\\");
		params->setDataDirectory(IuStringUtils::Replace(IuCoreUtils::WstringToUtf8((LPCTSTR)DataFolder), "\\", "/"));
	}
	else 
		#endif
	
	{
		DataFolder = GetApplicationDataPath() + _T("Image Uploader\\");
		params->setDataDirectory(IuStringUtils::Replace(IuCoreUtils::WstringToUtf8((LPCTSTR)DataFolder), "\\", "/"));
	}
}
#endif

CSettings::CSettings()
{
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	IsPortable = false;
	FindDataFolder();
	if (!IsDirectory(DataFolder))
	{
		CreateDirectory(DataFolder, 0);
	}
	if (!IsDirectory(IuCoreUtils::Utf8ToWstring(SettingsFolder).c_str()))
	{
		CreateDirectory(IuCoreUtils::Utf8ToWstring(SettingsFolder).c_str(), 0);
	}
	BOOL isElevated = false;
	IsElevated(&isElevated);
	if ( isElevated  || CmdLine.IsOption(L"afterupdate")) {
		WinUtils::MakeDirectoryWritable(DataFolder);
	}
#endif
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	CString copyFrom = WinUtils::GetAppFolder() + SETTINGS_FILE_NAME;
	CString copyTo = DataFolder + SETTINGS_FILE_NAME;
	if (FileExists(copyFrom) && !FileExists(copyTo))
	{
		MoveFile(copyFrom, copyTo);
	}

	
		
	// Default values of settings
	ExplorerCascadedMenu = true;
	ConnectionSettings.UseProxy =  FALSE;
	ConnectionSettings.ProxyPort = 0;
	ConnectionSettings.NeedsAuth = false;
	ConnectionSettings.ProxyType = 0;
	#endif
	LastUpdateTime = 0;
	UploadBufferSize = /*65536*/1024*1024;
	FileRetryLimit = 3;
	ActionRetryLimit = 2;
#if !defined(IU_SHELLEXT) && !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	if ( !IsFFmpegAvailable() ){
		VideoSettings.Engine = VideoEngineDirectshow;
	}
	
	WatchClipboard = true;
	ShowTrayIcon = false;
	ShowTrayIcon_changed = false;
	*m_Directory = 0;
	UseTxtTemplate = false;
	UseDirectLinks = true;
	DropVideoFilesToTheList = false;
	CodeLang = 0;
	ConfirmOnExit = 1;
	ExplorerContextMenu = false;
	ExplorerVideoContextMenu = true;
	ExplorerContextMenu_changed = false;
	ThumbsPerLine = 4;
	SendToContextMenu_changed = false;
	SendToContextMenu = 0;
	QuickUpload = 1;
	ParseSubDirs = 1;
	UseNewIcon = false;
	RememberImageServer = true;
    RememberFileServer = true;
	
	ShowUploadErrorDialog = true;

	ImageEditorPath = _T("mspaint.exe \"%1\"");
	AutoCopyToClipboard = false;
	AutoShowLog = true;
	

//	StringToFont(_T("Tahoma,7,b,204"), &ThumbSettings.ThumbFont);
	StringToFont(_T("Tahoma,8,,204"), &VideoSettings.Font);

	/*ThumbSettings.CreateThumbs = true;
	ThumbSettings.ThumbWidth = 180;
	ThumbSettings.ThumbHeight = 140;
	ThumbSettings.DrawFrame = true;
	ThumbSettings.ThumbAddImageSize  = true;
	ThumbSettings.FrameColor = RGB( 0, 74, 111);
	ThumbSettings.BackgroundColor = RGB( 255, 255, 255);f
	ThumbSettings.ThumbColor1 =  RGB( 13, 86, 125);
	ThumbSettings.ThumbColor2 = RGB( 6, 174, 255);
	ThumbSettings.UseServerThumbs = false;
	ThumbSettings.ScaleByHeight = false;
	ThumbSettings.ThumbTextColor = RGB( 255, 255, 255);
	ThumbSettings.ThumbAlpha = 120;
	ThumbSettings.Text = _T("%width%x%height% (%size%)");
	ThumbSettings.Format = ThumbCreatingParams::tfJPEG;
	ThumbSettings.FileName = "default";
	ThumbSettings.Quality = 85;*/

	VideoSettings.Columns = 3;
	VideoSettings.TileWidth =  200;
	VideoSettings.GapWidth = 5;
	VideoSettings.GapHeight = 7;
	VideoSettings.NumOfFrames = 8;
	VideoSettings.JPEGQuality =  100;
	VideoSettings.UseAviInfo = TRUE;
	VideoSettings.ShowMediaInfo = TRUE;
	VideoSettings.TextColor = RGB(0, 0, 0);
	VideoSettings.SnapshotsFolder = IuCoreUtils::Utf8ToWstring(Settings.SettingsFolder).c_str() + CString(_T("Snapshots"));
	VideoSettings.SnapshotFileTemplate = _T("%f%_%cx%_%cy%_%uid%\\grab_%i%.png");
	
	VideoSettings.Engine = IsFFmpegAvailable() ? VideoEngineAuto : VideoEngineDirectshow;
	

	ScreenshotSettings.Format =  1;
	ScreenshotSettings.Quality = 85;
	ScreenshotSettings.WindowHidingDelay = 450;
	ScreenshotSettings.Delay = 1;
	ScreenshotSettings.brushColor = RGB(255, 0, 0);
	ScreenshotSettings.ShowForeground = false;
	ScreenshotSettings.FilenameTemplate = _T("screenshot %y-%m-%d %i");
	ScreenshotSettings.CopyToClipboard = false;
	ScreenshotSettings.RemoveCorners = !WinUtils::IsWindows8orLater();
	ScreenshotSettings.AddShadow = false;
	ScreenshotSettings.RemoveBackground = false;
	ScreenshotSettings.OpenInEditor = true;
	ScreenshotSettings.UseOldRegionScreenshotMethod = false;

	TrayIconSettings.LeftClickCommand = 0; // without action
	TrayIconSettings.LeftDoubleClickCommand = 12; 

	TrayIconSettings.RightClickCommand = 1; // context menu
	TrayIconSettings.MiddleClickCommand = 7; // region screenshot
	TrayIconSettings.DontLaunchCopy = FALSE;
	TrayIconSettings.ShortenLinks = FALSE;
	TrayIconSettings.TrayScreenshotAction = TRAY_SCREENSHOT_OPENINEDITOR;

	ImageEditorSettings.BackgroundColor = Gdiplus::Color(255,255,255);
	ImageEditorSettings.ForegroundColor = Gdiplus::Color(255,0,0);
	ImageEditorSettings.PenSize = 12;
	ImageEditorSettings.RoundingRadius = ImageEditorSettings.PenSize;
	StringToFont(_T("Arial,12,b,204"), &ImageEditorSettings.Font);
    
    


	ImageReuploaderSettings.PasteHtmlOnCtrlV = true;
	Hotkeys_changed = false;
#endif
	
	/* binding settings */
	SettingsNode& general = mgr_["General"];
		general.n_bind(LastUpdateTime);
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
		general.n_bind(Language);
		general.n_bind(ExplorerContextMenu);
		/*general.n_bind(ExplorerVideoContextMenu);
		general.n_bind(ExplorerCascadedMenu);*/
#endif
		#if !defined(IU_SHELLEXT) && !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	


	general.n_bind(ConfirmOnExit);
	general.n_bind(SendToContextMenu);
	general.n_bind(ParseSubDirs);
	general.n_bind(ImageEditorPath);
	//general.n_bind(AutoStartup);
	general.n_bind(ShowTrayIcon);
	general.n_bind(AutoCopyToClipboard);
	general.n_bind(AutoShowLog);
	general.n_bind(ImagesFolder);
	general.n_bind(VideoFolder);
	general.n_bind(WatchClipboard);
	general.n_bind(UseNewIcon);
	general.n_bind(RememberFileServer);
	general.n_bind(RememberImageServer);
	#ifndef IU_SERVERLISTTOOL
	general.n_bind(Hotkeys);
	#endif
	SettingsNode& screenshot = mgr_["Screenshot"];
	screenshot.nm_bind(ScreenshotSettings, Delay);
	screenshot.nm_bind(ScreenshotSettings, Format);
	screenshot.nm_bind(ScreenshotSettings, Quality);
	screenshot.nm_bind(ScreenshotSettings, ShowForeground);
	screenshot.nm_bind(ScreenshotSettings, FilenameTemplate);
	screenshot.nm_bind(ScreenshotSettings, Folder);
	screenshot.nm_bind(ScreenshotSettings, AddShadow);
	screenshot.nm_bind(ScreenshotSettings, RemoveBackground);
	screenshot.nm_bind(ScreenshotSettings, RemoveCorners);
	screenshot.nm_bind(ScreenshotSettings, CopyToClipboard);
	screenshot.nm_bind(ScreenshotSettings, brushColor);
	screenshot.nm_bind(ScreenshotSettings, WindowHidingDelay);
	screenshot.nm_bind(ScreenshotSettings, OpenInEditor);
	screenshot.nm_bind(ScreenshotSettings, UseOldRegionScreenshotMethod);

	SettingsNode& imageEditor = mgr_["ImageEditor"];
	imageEditor.nm_bind(ImageEditorSettings, ForegroundColor);
	imageEditor.nm_bind(ImageEditorSettings, BackgroundColor);
	imageEditor.nm_bind(ImageEditorSettings, PenSize);
	imageEditor.nm_bind(ImageEditorSettings, RoundingRadius);
	imageEditor.nm_bind(ImageEditorSettings, Font);

	SettingsNode& image = mgr_["Image"];
	image["CurrentProfile"].bind(CurrentConvertProfileName);
	image.nm_bind(UploadProfile, KeepAsIs);

	/*SettingsNode& thumbnails = mgr_["Thumbnails"];
	thumbnails.nm_bind(ThumbSettings, FileName);
	thumbnails.nm_bind(ThumbSettings, CreateThumbs);
	thumbnails.nm_bind(ThumbSettings, ThumbWidth);
	thumbnails.nm_bind(ThumbSettings, ThumbHeight);
	thumbnails.nm_bind(ThumbSettings, ScaleByHeight);
	thumbnails.nm_bind(ThumbSettings, FrameColor);
	thumbnails.nm_bind(ThumbSettings, ThumbColor1);
	thumbnails.nm_bind(ThumbSettings, ThumbColor2);
	thumbnails.nm_bind(ThumbSettings, UseServerThumbs);
	thumbnails.nm_bind(ThumbSettings, ThumbAddImageSize);
	thumbnails.nm_bind(ThumbSettings, DrawFrame);
	thumbnails.nm_bind(ThumbSettings, Quality);
	thumbnails.nm_bind(ThumbSettings, Format);
	thumbnails.nm_bind(ThumbSettings, Text);
	thumbnails["Text"]["@Color"].bind(ThumbSettings.ThumbTextColor);
	thumbnails["Text"]["@Font"].bind(ThumbSettings.ThumbFont);
	thumbnails["Text"]["@TextOverThumb"].bind(ThumbSettings.TextOverThumb);
	thumbnails["Text"]["@ThumbAlpha"].bind(ThumbSettings.ThumbAlpha);*/

	SettingsNode& video = mgr_["VideoGrabber"];
	video.nm_bind(VideoSettings, Columns);
	video.nm_bind(VideoSettings, TileWidth);
	video.nm_bind(VideoSettings, GapWidth);
	video.nm_bind(VideoSettings, GapHeight);
	video.nm_bind(VideoSettings, NumOfFrames);
	video.nm_bind(VideoSettings, JPEGQuality);
	video.nm_bind(VideoSettings, ShowMediaInfo);
	video.nm_bind(VideoSettings, TextColor);
	video.nm_bind(VideoSettings, Font);
	video.nm_bind(VideoSettings, Engine);
	video.nm_bind(VideoSettings, SnapshotsFolder);
	video.nm_bind(VideoSettings, SnapshotFileTemplate);

	SettingsNode& tray = mgr_["TrayIcon"];
	tray.nm_bind(TrayIconSettings, LeftDoubleClickCommand);
	tray.nm_bind(TrayIconSettings, LeftClickCommand);
	tray.nm_bind(TrayIconSettings, RightClickCommand);
	tray.nm_bind(TrayIconSettings, MiddleClickCommand);
	tray.nm_bind(TrayIconSettings, DontLaunchCopy);
	tray.nm_bind(TrayIconSettings, ShortenLinks);
	tray.nm_bind(TrayIconSettings, TrayScreenshotAction);

	SettingsNode& history = mgr_["History"];
	history.nm_bind(HistorySettings, EnableDownloading);



	SettingsNode& imageReuploader = mgr_["ImageReuploader"];
	imageReuploader.nm_bind(ImageReuploaderSettings, PasteHtmlOnCtrlV);
	#endif

	SettingsNode& upload = mgr_["Uploading"];
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	upload.n_bind(ServerName);
	upload.n_bind(FileServerName);
//	upload.n_bind(UrlShorteningServer);
	upload.n_bind(QuickUpload);
	upload.n_bind(QuickServerName);
	upload.n_bind(CodeLang);
	upload.n_bind(ThumbsPerLine);
	upload.n_bind(UseDirectLinks);
	upload.n_bind(UseTxtTemplate);
	upload.n_bind(DropVideoFilesToTheList);
	upload.n_bind(CodeType);
	upload.n_bind(ShowUploadErrorDialog);
	
	imageServer.bind(upload["Server"]);
	fileServer.bind(upload["FileServer"]);
	quickScreenshotServer.bind(upload["QuickScreenshotServer"]);
	contextMenuServer.bind(upload["ContextMenuServer"]);
	urlShorteningServer.bind(upload["UrlShorteningServer"]);


	ConvertProfiles["Default"] = ImageConvertingParams();
	CurrentConvertProfileName = "Default";
#endif
	upload.n_bind(UploadBufferSize);
	upload.n_bind(FileRetryLimit);

	upload.n_bind(ActionRetryLimit);
#if  !defined  (IU_CLI) && !defined(IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
	SettingsNode& proxy = upload["Proxy"];
	proxy["@UseProxy"].bind(ConnectionSettings.UseProxy);
	proxy["@NeedsAuth"].bind(ConnectionSettings.NeedsAuth);
	proxy.nm_bind(ConnectionSettings, ServerAddress);
	proxy.nm_bind(ConnectionSettings, ProxyPort);
	proxy.nm_bind(ConnectionSettings, ProxyType);
	proxy.nm_bind(ConnectionSettings, ProxyUser);
	proxy.nm_bind(ConnectionSettings, ProxyPassword);;
#endif
}



bool CSettings::LoadSettings(std::string szDir, std::string fileName, bool LoadFromRegistry ) {
	fileName_ = !szDir.empty() ? szDir + ( (!fileName .empty())? fileName : "Settings.xml") 
		: SettingsFolder + ( !fileName.empty() ? fileName : "Settings.xml");
	//std::cout<< fileName_;
	//MessageBoxA(0,fileName_.c_str(),0,0);
	if ( !IuCoreUtils::FileExists( fileName_)  ) {
		return true;
	}
	SimpleXml xml;
	xml.LoadFromFile( fileName_ );
	mgr_.loadFromXmlNode( xml.getRoot("ImageUploader").GetChild("Settings") );
    imageServer.getImageUploadParamsRef().UseDefaultThumbSettings = false;
	SimpleXmlNode settingsNode = xml.getRoot( "ImageUploader" ).GetChild( "Settings" );

#if !defined(IU_CLI) && !defined( IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
	if ( Language == _T("T\u00FCrk\u00E7e") ) {  //fixes
		Language = _T("Turkish"); 
    } else if ( Language == _T("\u0423\u043A\u0440\u0430\u0457\u043D\u0441\u044C\u043A\u0430.lng") ){
        Language = _T("Ukrainian"); 
    }
	std::string temp;
	if ( !settingsNode["Image"]["Format"].IsNull() ) {
		// for compatibility with old version configuration file
		LoadConvertProfile( "Old profile", settingsNode );
	}

	if ( CmdLine.IsOption(_T("afterinstall") )) {
		if ( CmdLine.IsOption(_T("usenewicon") )) {
			UseNewIcon = true;
		} else {
			UseNewIcon = false;
		}
		SaveSettings();
	}
	
	// Migrating from 1.3.0 to 1.3.1 (added ImageEditor has been addded)
	if (settingsNode["ImageEditor"].IsNull() ) {
		if ( Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD ) {
			TrayIconSettings.TrayScreenshotAction = TRAY_SCREENSHOT_OPENINEDITOR;
		}
		
	}
	LoadConvertProfiles( settingsNode.GetChild("Image").GetChild("Profiles") );
	LoadServerProfiles( settingsNode.GetChild("Uploading").GetChild("ServerProfiles") );
#endif
	LoadAccounts( xml.getRoot( "ImageUploader" ).GetChild( "Settings" ).GetChild( "ServersParams" ) );
#if !defined(IU_CLI) && !defined( IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
	// Fixing profies
	if ( !imageServer.profileName().IsEmpty() &&  ServersSettings[WCstringToUtf8(imageServer.serverName())].find(WCstringToUtf8(imageServer.profileName())) == ServersSettings[WCstringToUtf8(imageServer.serverName())].end() ) {
		imageServer.setProfileName("");
	}

	if ( !fileServer.profileName().IsEmpty() &&  ServersSettings[WCstringToUtf8(fileServer.serverName())].find(WCstringToUtf8(fileServer.profileName())) == ServersSettings[WCstringToUtf8(fileServer.serverName())].end() ) {
		fileServer.setProfileName("");
	}
	if ( !contextMenuServer.profileName().IsEmpty() &&  ServersSettings[WCstringToUtf8(contextMenuServer.serverName())].find(WCstringToUtf8(contextMenuServer.profileName())) == ServersSettings[WCstringToUtf8(contextMenuServer.serverName())].end() ) {
		contextMenuServer.setProfileName("");
	}

	if ( !quickScreenshotServer.profileName().IsEmpty() &&  ServersSettings[WCstringToUtf8(quickScreenshotServer.serverName())].find(WCstringToUtf8(quickScreenshotServer.profileName())) == ServersSettings[WCstringToUtf8(quickScreenshotServer.serverName())].end() ) {
		quickScreenshotServer.setProfileName("");
	}

	if ( !urlShorteningServer.profileName().IsEmpty() &&  ServersSettings[WCstringToUtf8(urlShorteningServer.serverName())].find(WCstringToUtf8(urlShorteningServer.profileName())) == ServersSettings[WCstringToUtf8(urlShorteningServer.serverName())].end() ) {
		urlShorteningServer.setProfileName("");
	}
	if ( UploadBufferSize == 65536) {
		UploadBufferSize = 1024 * 1024;
	}
#endif
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	// Loading some settings from registry
	if ( LoadFromRegistry ) {
		CRegistry Reg;
		Reg.SetRootKey( HKEY_LOCAL_MACHINE );
		if ( Reg.SetKey("Software\\Zenden.ws\\Image Uploader", false ) ) {
			ExplorerContextMenu = Reg.ReadBool("ExplorerContextMenu", false);

		} else {
			ExplorerContextMenu = false;
		}
	}
	CRegistry Reg;
	Reg.SetRootKey( HKEY_CURRENT_USER );
	if ( Reg.SetKey("Software\\Zenden.ws\\Image Uploader", false ) ) {
		ExplorerCascadedMenu = Reg.ReadBool( "ExplorerCascadedMenu", true);
		ExplorerVideoContextMenu = Reg.ReadBool( "ExplorerVideoContextMenu", true);
	} 
	CRegistry Reg2;
	Reg2.SetRootKey(HKEY_CURRENT_USER);
	if ( Reg2.SetKey("Software\\Zenden.ws\\Image Uploader", false ) ) {
		AutoStartup = Reg2.ReadBool("AutoStartup", false);
	}

	if ( VideoSettings.Engine != VideoEngineDirectshow &&  VideoSettings.Engine != VideoEngineFFmpeg && VideoSettings.Engine != VideoEngineAuto   ){
		VideoSettings.Engine = VideoEngineAuto;
	}
	if ( !IsFFmpegAvailable() ){
		VideoSettings.Engine = VideoEngineDirectshow;
	}
#endif
	return true;
}

#if !defined(IU_CLI) && !defined(IU_SHELLEXT)

#define MY_CLSID _T("{535E39BD-5883-454C-AFFC-C54B66B18206}")

	bool RegisterClsId()
	{
		TCHAR Buffer[MAX_PATH + 1] = _T("CLSID\\");
		HKEY Key = 0;

		lstrcat(Buffer, MY_CLSID);
		RegCreateKeyEx(HKEY_CLASSES_ROOT, Buffer, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &Key, NULL);

		if (!Key)
			return false;

		HKEY TempKey = 0;
		RegCreateKeyEx(Key, _T("LocalServer32"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &TempKey, NULL);

		GetModuleFileName(0, Buffer, MAX_PATH);

		RegSetValueEx(TempKey,  0, 0, REG_SZ, (LPBYTE) Buffer, (lstrlen(Buffer) + 1) * sizeof(TCHAR));

		RegCloseKey(TempKey);

		RegCreateKeyEx(Key, _T("ProgID"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &TempKey, NULL);
		lstrcpy(Buffer, _T("ImageUploader.ContextMenuHandler.1"));
		RegSetValueEx(TempKey,  0, 0, REG_SZ, (LPBYTE) Buffer, (lstrlen(Buffer) + 1) * sizeof(TCHAR));
		RegCloseKey(TempKey);

		RegCloseKey(Key);

		Key = 0;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		                 _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, KEY_WRITE,
		                 &Key) == ERROR_SUCCESS)
		{
			lstrcpy(Buffer, _T("ImageUploader ContextMenuHandler"));
			RegSetValueEx(Key, MY_CLSID,  0, REG_SZ, (LPBYTE) Buffer, (lstrlen(Buffer) + 1) * sizeof(TCHAR));
			RegCloseKey(Key);
		}
		return true;
	}

	bool UnRegisterClsId() // Deleting CLSID record from registry
	{
		TCHAR Buffer[MAX_PATH + 1] = _T("CLSID\\");
		lstrcat(Buffer, MY_CLSID);
		return SHDeleteKey(HKEY_CLASSES_ROOT, Buffer) == ERROR_SUCCESS;
	}

/* Obsolete function; will be removed in future */
int AddToExplorerContextMenu(LPCTSTR Extension, LPCTSTR Title, LPCTSTR Command, bool DropTarget) {
	HKEY ExtKey = NULL;
	TCHAR Buffer[MAX_PATH];

	Buffer[0] = _T('.');
	lstrcpy(Buffer + 1, Extension); // Формируем строку вида ".ext"
		RegCreateKeyEx(HKEY_CLASSES_ROOT, Buffer, 0, 0, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, 0, &ExtKey, NULL);

		TCHAR ClassName[MAX_PATH] = _T("\0");
		DWORD BufSize = sizeof(ClassName) / sizeof(TCHAR);
		DWORD Type = REG_SZ;
		RegQueryValueEx(ExtKey, 0, 0, &Type, (LPBYTE)&ClassName, &BufSize); // Retrieving classname for extension
		RegCloseKey(ExtKey);

		if (Title == 0)
		{
			// Deleting
			wsprintf(Buffer, _T("%s\\shell\\iuploader"), (LPCTSTR)ClassName);
			SHDeleteKey(HKEY_CLASSES_ROOT, Buffer);
			return 0;
		}

		wsprintf(Buffer, _T("%s\\shell\\iuploader\\command"), (LPCTSTR)ClassName);

		if (!lstrlen(Buffer))
			return false;
		wsprintf(Buffer, _T("%s\\shell\\iuploader"), (LPCTSTR)ClassName);
		DWORD res  = RegCreateKeyEx(HKEY_CLASSES_ROOT, Buffer, 0, 0, REG_OPTION_NON_VOLATILE,  KEY_WRITE, 0, &ExtKey,
		                            NULL);

		if (res != ERROR_SUCCESS)
		{
			WriteLog(logWarning, TR("Настройки"), CString(TR(
			                                                 "Не могу создать запись в реестре для расширения ")) +
			         Extension + _T("\r\n") + DisplayError(res));
			return 0;
		}

		RegSetValueEx(ExtKey, 0, 0, REG_SZ, (LPBYTE)Title, (lstrlen(Title) + 1) * sizeof(TCHAR));

		HKEY CommandKey;

		if (RegCreateKeyEx(ExtKey, _T("command"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &CommandKey,
		                   NULL) != ERROR_SUCCESS)
		{
		}
		HKEY DropTargetKey;
		if (DropTarget)
		{
			if (RegCreateKeyEx(ExtKey, _T("DropTarget"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &DropTargetKey,
			                   NULL) != ERROR_SUCCESS)
			{
				return 0;
			}
		}

		RegCloseKey(ExtKey);

		RegSetValueEx(CommandKey, 0, 0,  REG_SZ, (LPBYTE)Command, (lstrlen(Command) + 1) * sizeof(TCHAR));
		RegCloseKey(CommandKey);

		if (DropTarget)
		{
			RegSetValueEx(DropTargetKey, _T("Clsid"), 0, REG_SZ,  (LPBYTE)MY_CLSID, (lstrlen(MY_CLSID) + 1) * sizeof(TCHAR));
		}

		RegCloseKey(DropTargetKey);
		return 1; // That's means ALL OK! :)
	}
#endif

	bool CSettings::SaveSettings()
	{
		SimpleXml xml;
		mgr_.saveToXmlNode(xml.getRoot("ImageUploader").GetChild("Settings"));
#if !defined(IU_SERVERLISTTOOL) && !defined  (IU_CLI) && !defined(IU_SHELLEXT)
		SaveConvertProfiles(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("Image").GetChild("Profiles"));
		SaveServerProfiles( xml.getRoot("ImageUploader").GetChild("Settings").GetChild("Uploading").GetChild("ServerProfiles") );
#endif
		SaveAccounts(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("ServersParams"));
		//std::cerr << "Saving setting to "<< IuCoreUtils::WstringToUtf8((LPCTSTR)fileName_);
		xml.SaveToFile(fileName_);

#if !defined(IU_SERVERLISTTOOL) && !defined(IU_CLI)
		CRegistry Reg;
		Reg.SetRootKey(HKEY_CURRENT_USER);
		// if(ExplorerContextMenu)
		{
			bool canCreateRegistryKey = ( ExplorerContextMenu );
			if ( Reg.SetKey("Software\\Zenden.ws\\Image Uploader", canCreateRegistryKey ) ) {
				if ( ExplorerContextMenu ) {
					Reg.WriteBool( "ExplorerCascadedMenu", ExplorerCascadedMenu );
					Reg.WriteBool("ExplorerContextMenu", ExplorerContextMenu);
					Reg.WriteBool( "ExplorerVideoContextMenu", ExplorerVideoContextMenu );
					Reg.WriteString( "Language", Language );
				} else {
					Reg.DeleteValue("ExplorerCascadedMenu");
					Reg.DeleteValue("ExplorerContextMenu");
					Reg.DeleteValue("ExplorerVideoContextMenu");
					Reg.DeleteValue("Language");
				}
			}
		}
		/*else
		{
		   //Reg.DeleteKey("Software\\Zenden.ws\\Image Uploader");
		}*/
		EnableAutostartup(AutoStartup);

		if (SendToContextMenu_changed   || ExplorerContextMenu_changed) {
			AutoStartup_changed = false;
			BOOL b;
			if ( IsVista() && IsElevated(&b) != S_OK ) {
				// Start new elevated process 
				ApplyRegistrySettings();
			} else {
				// Process has already admin rights
				ApplyRegSettingsRightNow();
			}
		}

		ExplorerContextMenu_changed = false;
		SendToContextMenu_changed = false;

		if (ShowTrayIcon_changed)
		{
			ShowTrayIcon_changed = false;
			if (ShowTrayIcon)
			{
				if (!IsRunningFloatingWnd())
				{
					CmdLine.AddParam(_T("/tray"));
					floatWnd.CreateTrayIcon();
				}
			}
			else
			{
				HWND TrayWnd = FindWindow(0, _T("ImageUploader_TrayWnd"));
				if (TrayWnd) {
					::SendMessage( TrayWnd, WM_CLOSETRAYWND, 0, 0 );
				}
			}
		}
		else if (ShowTrayIcon)
		{
			HWND TrayWnd = FindWindow(0, _T("ImageUploader_TrayWnd"));
			if (TrayWnd)
				SendMessage(TrayWnd, WM_RELOADSETTINGS,  (floatWnd.m_hWnd) ? 1 : 0, (Settings.Hotkeys_changed) ? 0 : 1);
		}

		Settings.Hotkeys_changed  = false;
#endif
		return true;
	}

#if !defined(IU_SERVERLISTTOOL) && !defined(IU_CLI)
// The following code should  be deleted in next releases
	void CSettings::ApplyRegSettingsRightNow()
	{
		// Applying Startup settings
		// EnableAutostartup(AutoStartup);
		RegisterShellExtension( ExplorerContextMenu );

		// if(SendToContextMenu_changed)
		{
			CString ShortcutName = GetSendToPath() + _T("\\Image Uploader.lnk");

			if (SendToContextMenu )
			{
				if (FileExists(ShortcutName))
					DeleteFile(ShortcutName);

				CreateShortCut(ShortcutName, CmdLine.ModuleName(), WinUtils::GetAppFolder(), _T(
				                  " /fromcontextmenu /upload"), 0, SW_SHOW, CmdLine.ModuleName(), 0);
			}
			else
			{
				DeleteFile(ShortcutName);
			}
		}

		// if(ExplorerImagesContextMenu_changed || ExplorerVideoContextMenu_changed)
		{
			TCHAR szFileName[MAX_PATH + 8] = _T("\"");

			GetModuleFileName(0, szFileName + 1, MAX_PATH);
			lstrcat(szFileName, _T("\" \"%1\""));

			LPTSTR szList = _T("jpg\0jpeg\0png\0bmp\0gif");
			int Res;

			UnRegisterClsId();

			// if(ExplorerContextMenu_changed)
			{
				while ((*szList) != 0)
				{
					Res =
					   AddToExplorerContextMenu(szList, (/*ExplorerImagesContextMenu?TR("Загрузить изображения"):*/ 0),
					                            szFileName,
					                            true);
					szList += lstrlen(szList) + 1;
				}
			}

			// if( ExplorerVideoContextMenu_changed)
			{
				szList = VIDEO_FORMATS;
				while ((*szList) != 0)
				{
					Res =
					   AddToExplorerContextMenu(szList, (/*ExplorerVideoContextMenu?TR("Открыть в Image Uploader"):*/ 0),
					                            szFileName,
					                            false);
					szList += lstrlen(szList) + 1;
				}
			}
		}
	}
#endif
#if !defined(IU_SERVERLISTTOOL) && !defined(IU_CLI)
	ServerSettingsStruct& CSettings::ServerByName(CString name)
	{
		return ServersSettings[IuCoreUtils::WstringToUtf8((LPCTSTR)name)].begin()->second;
	}

#endif
	ServerSettingsStruct& CSettings::ServerByUtf8Name(std::string name)
	{
		return ServersSettings[name].begin()->second;
	}

	bool CSettings::LoadAccounts(SimpleXmlNode root)
	{
		std::vector<SimpleXmlNode> servers;
		root.GetChilds("Server", servers);

		for (size_t i = 0; i < servers.size(); i++)
		{
			std::string server_name = servers[i].Attribute("Name");
			std::vector<std::string> attribs;
			servers[i].GetAttributes(attribs);
			ServerSettingsStruct tempSettings;

			for (size_t j = 0; j < attribs.size(); j++)
			{
				std::string attribName = attribs[j];

				if (attribName.empty())
					continue;
				if ( attribName.substr(0, 1) == "_")
				{
					std::string value = servers[i].Attribute(attribName);
					attribName = attribName.substr(1, attribName.size() - 1);
					if (!value.empty())
						tempSettings.params[attribName] = value;
				}
			}
			tempSettings.authData.DoAuth = servers[i].AttributeBool("Auth");
#if !defined  (IU_CLI) && !defined(IU_SHELLEXT)
			

			std::string encodedLogin = servers[i].Attribute("Login");
			CEncodedPassword login;
			login.fromEncodedData(encodedLogin.c_str());
			tempSettings.authData.Login = WCstringToUtf8(login);

			std::string encodedPass = servers[i].Attribute("Password");
			CEncodedPassword pass;
			pass.fromEncodedData(encodedPass.c_str());
			tempSettings.authData.Password = WCstringToUtf8(pass);
#else
	tempSettings.authData.Login =servers[i].Attribute("Login");
#endif

			tempSettings.defaultFolder.setId( servers[i].Attribute("DefaultFolderId"));
			tempSettings.defaultFolder.viewUrl = servers[i].Attribute("DefaultFolderUrl");
			tempSettings.defaultFolder.setTitle(servers[i].Attribute("DefaultFolderTitle"));

				ServersSettings[server_name][tempSettings.authData.Login] = tempSettings ;
		}
		return true;
	}

	bool CSettings::SaveAccounts(SimpleXmlNode root)
	{
		ServerSettingsMap::iterator it1;
		for (it1 = ServersSettings.begin(); it1 != ServersSettings.end(); ++it1)
		{
			std::map <std::string, ServerSettingsStruct>::iterator it;
			for (it = it1->second.begin(); it !=it1->second.end(); ++it)
			{
				ServerSettingsStruct & sss = it->second;
				if ( sss.isEmpty() ) {
					continue;
				}
				SimpleXmlNode serverNode = root.CreateChild("Server");
				
				serverNode.SetAttribute("Name", it1->first);

				std::map <std::string, std::string>::iterator param;
				for (param = it->second.params.begin(); param != sss.params.end(); ++param)
				{
					if (  param->first == "FolderID" || param->first == "FolderUrl" || param->first == "FolderTitle") {
						continue;
					}
					serverNode.SetAttribute("_" + param->first, param->second);
				}
				serverNode.SetAttributeBool("Auth", sss.authData.DoAuth);

	#if !defined  (IU_CLI) && !defined(IU_SHELLEXT)
				

				CEncodedPassword login(Utf8ToWCstring(sss.authData.Login));
				serverNode.SetAttribute("Login", WCstringToUtf8(login.toEncodedData()));

				CUploadEngineData* ued = _EngineList->byName(Utf8ToWCstring(it->first));
				if ( !ued || ued->NeedPassword ) { 
					CEncodedPassword pass(Utf8ToWCstring(it->second.authData.Password));
					serverNode.SetAttribute("Password", WCstringToUtf8(pass.toEncodedData()));
				}
	#else
		serverNode.SetAttribute("Login", sss.authData.Login);
	#endif
				if ( !it->second.defaultFolder.getId().empty() ) {
					serverNode.SetAttributeString("DefaultFolderId", sss.defaultFolder.getId());
					serverNode.SetAttributeString("DefaultFolderUrl", sss.defaultFolder.viewUrl);
					serverNode.SetAttributeString("DefaultFolderTitle", sss.defaultFolder.getTitle());
				}

			}
		}
		return true;
	}
#if !defined(IU_CLI) && !defined( IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
	bool CSettings::LoadServerProfiles(SimpleXmlNode root)
	{
		std::vector<SimpleXmlNode> servers;
		root.GetChilds("ServerProfile", servers);

		for (size_t i = 0; i < servers.size(); i++)
		{
			SimpleXmlNode serverProfileNode = servers[i];
			std::string profileName = serverProfileNode.Attribute("ServerProfileId");
			ServerProfile sp;
			SettingsManager mgr;
			sp.bind(mgr.root());
			
			mgr.loadFromXmlNode(serverProfileNode);
			ServerProfiles[Utf8ToWCstring(profileName)] = sp;
		}
		return true;
	}

	bool CSettings::SaveServerProfiles(SimpleXmlNode root)
	{
		for ( ServerProfilesMap::iterator it = ServerProfiles.begin(); it != ServerProfiles.end(); ++it) {
			SimpleXmlNode serverProfileNode = root.CreateChild("ServerProfile");
		
			std::string profileName = WCstringToUtf8(it->first);
			
			//ServerProfile sp = ;
			SettingsManager mgr;
			it->second.bind(mgr.root());
			mgr["@ServerProfileId"].bind(profileName);

			mgr.saveToXmlNode(serverProfileNode);
		}
		return true;
	}

#endif

#if !defined  (IU_CLI) && !defined(IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
	bool CSettings::LoadConvertProfiles(SimpleXmlNode root)
	{
		std::vector<SimpleXmlNode> profiles;
		root.GetChilds("Profile", profiles);

		for (size_t i = 0; i < profiles.size(); i++)
		{
			LoadConvertProfile("", profiles[i]);
		}
		return true;
	}

	bool CSettings::LoadConvertProfile(const CString& name, SimpleXmlNode profileNode)
	{
		SettingsManager mgr;
		ImageConvertingParams params;
		std::string saveTo = profileNode.Attribute("Name");
		if (!name.IsEmpty())
			saveTo = WCstringToUtf8(name);
		SettingsNode& image = mgr["Image"];
		BindConvertProfile(image, params);
		mgr.loadFromXmlNode(profileNode);
		ConvertProfiles[Utf8ToWCstring( saveTo)] = params;
		return true;
	}

	bool CSettings::SaveConvertProfiles(SimpleXmlNode root)
	{
		std::map<CString, ImageConvertingParams>::iterator it;
		for (it = ConvertProfiles.begin(); it != ConvertProfiles.end(); ++it)
		{
			SimpleXmlNode profile = root.CreateChild("Profile");
			ImageConvertingParams& params = it->second;
			profile.SetAttribute("Name", WCstringToUtf8(it->first));
			SettingsManager mgr;
			SettingsNode& image = mgr["Image"];
			BindConvertProfile(image, params);
			mgr.saveToXmlNode(profile);
		}
		return true;
	}

	void CSettings::BindConvertProfile(SettingsNode& image, ImageConvertingParams& params)
	{
		image.nm_bind(params, Quality);
		image.nm_bind(params, Format);
		image["NewWidth"].bind(params.strNewWidth);
		image["NewHeight"].bind(params.strNewHeight);
		image.nm_bind(params, AddLogo);
		image.nm_bind(params, AddText);
		image.nm_bind(params, ResizeMode);
		image.nm_bind(params, SmartConverting);
		image.nm_bind(params, PreserveExifInformation);
		image["Logo"].bind(params.LogoFileName);
		image["Logo"]["@LogoPosition"].bind(params.LogoPosition);
		image["Logo"]["@LogoBlend"].bind(params.LogoBlend);
		image["Text"].bind(params.Text);
		image["Text"]["@TextPosition"].bind(params.TextPosition);
		image["Text"]["@TextColor"].bind(params.TextColor);
		image["Text"]["@StrokeColor"].bind(params.StrokeColor);
		image["Text"]["@Font"].bind(params.Font);
	}
#endif

CSettings::~CSettings() {
}

#if !defined(IU_SERVERLISTTOOL) && !defined  (IU_CLI) && !defined(IU_SHELLEXT)
void CSettings::Uninstall() {
	BOOL b;
	if (IsVista() && IsElevated(&b) != S_OK) {
		RunIuElevated("/uninstall");
		return;
	}
	AutoStartup = false;
	SendToContextMenu  = false;
	RegisterShellExtension(false);
	EnableAutostartup(false);
	CRegistry Reg;
	Reg.SetRootKey(HKEY_CURRENT_USER);
	Reg.DeleteWithSubkeys("Software\\Zenden.ws\\Image Uploader\\ContextMenuItems");
	Reg.DeleteKey( "Software\\Zenden.ws\\Image Uploader" );
	Reg.DeleteKey( "Software\\Zenden.ws" ); // Will not delete if contains subkeys
	Reg.SetRootKey( HKEY_LOCAL_MACHINE );
	Reg.DeleteKey( "Software\\Zenden.ws\\Image Uploader" );
	Reg.DeleteKey( "Software\\Zenden.ws" ); // Will not delete if contains subkeys
	WinUtils::RemoveBrowserKey();

	CString ShortcutName = GetSendToPath() + _T("\\Image Uploader.lnk");
	DeleteFile(ShortcutName);
}

void CSettings::EnableAutostartup(bool enable) {
	CRegistry Reg;
	Reg.SetRootKey( HKEY_CURRENT_USER );
	bool canCreateRegistryKey = enable;

	if ( Reg.SetKey( "Software\\Zenden.ws\\Image Uploader", canCreateRegistryKey ) ) {
		if ( enable ) {
			Reg.WriteBool( "AutoStartup", enable );
		} else {
			Reg.DeleteValue("AutoStartup");
		}
	}

	if ( enable ) {
		HKEY hKey;
		CString StartupCommand = _T("\"") + CmdLine.ModuleName() + _T("\" /tray");
		LONG lRet, lRetOpen;
		lRet = RegOpenKeyEx( HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 
		                      0, KEY_WRITE, &hKey );
		if (!lRet) {
			lRetOpen = RegSetValueEx( hKey, _T("ImageUploader"), NULL, REG_SZ, (BYTE*)(LPCTSTR)StartupCommand,
			                            (StartupCommand.GetLength() + 1) * sizeof(TCHAR));
		}
		RegCloseKey( hKey );
	} else { 
		// deleting from Startup (autorun)
		HKEY hKey;
		LONG lRet;
		lRet = RegOpenKeyEx( HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_WRITE,
									&hKey );
		RegDeleteValue( hKey, _T("ImageUploader") );
	}
}

int CSettings::getServerID()
{
	return _EngineList->GetUploadEngineIndex(getServerName());
}

int CSettings::getQuickServerID()
{
	return _EngineList->GetUploadEngineIndex(contextMenuServer.serverName());
}

int CSettings::getFileServerID()
{
		return _EngineList->GetUploadEngineIndex(getFileServerName());
}


bool CSettings::IsFFmpegAvailable() {
	CString appFolder = WinUtils::GetAppFolder();
	return FileExists( appFolder + "avcodec-56.dll");
}

CString CSettings::prepareVideoDialogFilters() {
	CString result;
	std::vector<std::string>& extensions = VideoUtils::Instance().videoFilesExtensions;
	for ( int i = 0; i < extensions.size(); i++ ) {
		result += CString("*.") + CString(extensions[i].c_str()) + _T(";");
	}
	return result;
}
CString CSettings::getServerName() {
	return imageServer.serverName();
}
CString CSettings::getQuickServerName() {
	return contextMenuServer.serverName();
}
CString CSettings::getFileServerName() {
	return fileServer.serverName();
}

CString CSettings::getSettingsFileName() const
{
	return IuCoreUtils::Utf8ToWstring(fileName_).c_str();
}

void ImageUploadParams::bind(SettingsNode& n){
	SettingsNode & node = n["ImageUploadParams"];
	node.n_bind(UseServerThumbs);
	node.n_bind(CreateThumbs);

	node.n_bind(ProcessImages);
	node.n_bind(ImageProfileName);
	node.n_bind(UseDefaultThumbSettings);
	SettingsNode & thumb = node["Thumb"];
	thumb.nm_bind(Thumb, TemplateName);
	thumb.nm_bind(Thumb, Size);
	thumb["ResizeMode"].bind((int&)Thumb.ResizeMode);
	thumb.nm_bind(Thumb, AddImageSize);
	thumb.nm_bind(Thumb, DrawFrame);
	thumb.nm_bind(Thumb, Quality);
	thumb.nm_bind(Thumb, Format);
	thumb.nm_bind(Thumb, Text);
}

ThumbCreatingParams ImageUploadParams::getThumb()
{
	if ( UseDefaultThumbSettings && &Settings.imageServer.imageUploadParams  != this) {
		return Settings.imageServer.imageUploadParams.Thumb;
	}
	return Thumb;
}

ThumbCreatingParams& ImageUploadParams::getThumbRef()
{	
	return Thumb;
}

void ImageUploadParams::setThumb(ThumbCreatingParams tcp)
{
	Thumb = tcp;
}

#endif
