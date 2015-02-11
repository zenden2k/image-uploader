/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Func/Settings.h"
#include <Shlobj.h>
#include "Core/SettingsManager.h"
#include "Func/myutils.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Common/CmdLine.h"
#include "3rdpart/Registry.h"
#include <Core/Utils/StringUtils.h>
#include <Core/Video/VideoUtils.h>
#include <Func/WinUtils.h>
#include <Func/Common.h>

#define VIDEO_FORMATS _T("avi\0mpg\0mpeg\0vob\0divx\0flv\0wmv\0asf\0mkv\0mov\0ts\0mp2\0mp4\0")_T("3gp\0rm\0mpeg2ts\0\0")

#ifndef IU_SERVERLISTTOOL
	#include "Gui/Dialogs/FloatingWindow.h"
#endif

#define SETTINGS_FILE_NAME _T("settings.xml")

const TCHAR CSettings::VideoEngineDirectshow[] = _T("Directshow");
const TCHAR CSettings::VideoEngineFFmpeg[]     = _T("FFmpeg");
const TCHAR CSettings::VideoEngineAuto[]       = _T("Auto");

COLORREF CSettings::DefaultLinkColor = RGB(0x0C,0x32, 0x50);

ServerProfile::ServerProfile() {

}

ServerProfile::ServerProfile(CString newServerName){
	serverName_ = newServerName;
}

void ServerProfile::setProfileName(CString newProfileName) {
	profileName_ = newProfileName;
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

ServerSettingsStruct ServerProfile::serverSettings() {
	return Settings.ServersSettings[serverName_][profileName_];
}

CUploadEngineData* ServerProfile::uploadEngineData() const {
	return _EngineList->byName(serverName_);
}

 /* CString support for  SettingsManager */
inline std::string myToString(const CString& value)
{
	return WCstringToUtf8(value);
}

inline void myFromString(const std::string& text, CString& value)
{
	value = Utf8ToWCstring(text);
}

template<class T> std::string myToString(const EnumWrapper<T>& value)
{
	return IuCoreUtils::toString (value.value_);
}

template<class T> void myFromString(const std::string& text,  EnumWrapper<T>& value)
{
	value = static_cast<T>(atoi(text.c_str()));
}

/* LOGFONT serialization support */
inline std::string myToString(const LOGFONT& value) {
	CString res;
	WinUtils::FontToString( &value, res );
	return WCstringToUtf8(res);
}

inline void myFromString(const std::string& text, LOGFONT& value) {
	CString wide_text = Utf8ToWCstring(text);
	LOGFONT font;
	WinUtils::StringToFont(wide_text, &font);
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


void ImageUploadParams::bind(SettingsNode& n){
	SettingsNode & node = n["ImageUploadParams"];
	node.n_bind(UseServerThumbs);
	node.n_bind(CreateThumbs);
	node.n_bind(ProcessImages);
	node.n_bind(ImageProfileName);
	node.n_bind(UseDefaultThumbSettings);
	node.n_bind(Size);

}

CSettings Settings;

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

CString CSettings::getShellExtensionFileName() const {
	CString file = WinUtils::GetAppFolder() + (WinUtils::IsWindows64Bit() ? _T("ExplorerIntegration64.dll") : _T("ExplorerIntegration.dll"));
	return file;
}

void RegisterShellExtension(bool Register) {
	CString moduleName = Settings.getShellExtensionFileName();
	if ( !WinUtils::FileExists( moduleName ) ) {
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

	if ( WinUtils::IsVista() && !WinUtils::IsElevated() ) {
		TempInfo.lpVerb = _T("runas");
	} else {
		TempInfo.lpVerb = _T("open");
	}
	TempInfo.lpFile       = _T("regsvr32");
	TempInfo.lpParameters = CString((Register ? _T("") : _T("/u "))) + _T("/s \"") + moduleName + _T("\"");
	TempInfo.lpDirectory  = s;
	TempInfo.nShow        = SW_NORMAL;
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
	if (WinUtils::IsDirectory(WinUtils::GetAppFolder() + _T("Data"))) {
		DataFolder     = WinUtils::GetAppFolder() + _T("Data\\");
		SettingsFolder = DataFolder;
		return;
	}

	SettingsFolder =  WinUtils::GetApplicationDataPath() + _T("\\Image Uploader\\");
	{
		CRegistry Reg;
		CString lang;

		Reg.SetRootKey(HKEY_CURRENT_USER);
		if (Reg.SetKey("Software\\Zenden.ws\\Image Uploader", false))
		{
			CString dir = Reg.ReadString("DataPath");

			if (!dir.IsEmpty() && WinUtils::IsDirectory(dir))
			{
				DataFolder = dir;
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

			if (!dir.IsEmpty() && WinUtils::IsDirectory(dir))
			{
				DataFolder = dir;
				return;
			}
		}
	}

	if (WinUtils::FileExists(WinUtils::GetCommonApplicationDataPath() + SETTINGS_FILE_NAME)) {
		DataFolder = WinUtils::GetCommonApplicationDataPath() + _T("Image Uploader\\");
	}
	else {
		DataFolder = WinUtils::GetApplicationDataPath() + _T("Image Uploader\\");
	}
}

CSettings::CSettings()
#ifndef IU_SHELLEXT
	//: ServerID(UploadProfile.ServerID), QuickServerID(UploadProfile.QuickServerID)
#endif
{
	FindDataFolder();
	if (!WinUtils::IsDirectory(DataFolder))
	{
		CreateDirectory(DataFolder, 0);
	}
	if (!WinUtils::IsDirectory(SettingsFolder))
	{
		CreateDirectory(SettingsFolder, 0);
	}
	CString copyFrom = WinUtils::GetAppFolder() + SETTINGS_FILE_NAME;
	CString copyTo = DataFolder + SETTINGS_FILE_NAME;
	if ( WinUtils::FileExists(copyFrom) && !WinUtils::FileExists(copyTo) ) {
		MoveFile(copyFrom, copyTo);
	}
	
	if ( !IsFFmpegAvailable() ) {
		VideoSettings.Engine = VideoEngineDirectshow;
	}
	// Default values of settings
	ExplorerCascadedMenu = true;
#ifndef IU_SHELLEXT
	LastUpdateTime = 0;
	WatchClipboard = true;
	ShowTrayIcon = false;
	ShowTrayIcon_changed = false;
	*m_Directory = 0;
	UseTxtTemplate = false;
	UseDirectLinks = true;

	CodeLang = 0;
	ConfirmOnExit = 1;
	ExplorerContextMenu = false;
	ExplorerVideoContextMenu = false;
	ExplorerContextMenu_changed = false;
	ThumbsPerLine = 4;
	SendToContextMenu_changed = false;
	SendToContextMenu = 0;
	QuickUpload = 0;
	ParseSubDirs = 1;
	FileRetryLimit = 3;
	ShowUploadErrorDialog = true;
	ActionRetryLimit = 2;
	ImageEditorPath = _T("mspaint.exe \"%1\"");
	AutoCopyToClipboard = false;
	AutoShowLog = true;
	UploadBufferSize = 65536;

	WinUtils::StringToFont(_T("Tahoma,7,b,204"), &ThumbSettings.ThumbFont);
	WinUtils::StringToFont(_T("Tahoma,8,,204"), &VideoSettings.Font);

	ThumbSettings.CreateThumbs = true;
	ThumbSettings.ThumbWidth = 180;
	ThumbSettings.ThumbHeight = 140;
	ThumbSettings.DrawFrame = true;
	ThumbSettings.ThumbAddImageSize  = true;
	ThumbSettings.FrameColor = RGB( 0, 74, 111);
	ThumbSettings.BackgroundColor = RGB( 255, 255, 255);
	ThumbSettings.ThumbColor1 =  RGB( 13, 86, 125);
	ThumbSettings.ThumbColor2 = RGB( 6, 174, 255);
	ThumbSettings.UseServerThumbs = false;
	ThumbSettings.ScaleByHeight = false;
	ThumbSettings.ThumbTextColor = RGB( 255, 255, 255);
	ThumbSettings.ThumbAlpha = 120;
	ThumbSettings.Text = _T("%width%x%height% (%size%)");
	ThumbSettings.Format = ThumbCreatingParams::tfJPEG;
	ThumbSettings.FileName = "default";
	ThumbSettings.Quality = 85;

	VideoSettings.Columns = 3;
	VideoSettings.TileWidth =  200;
	VideoSettings.GapWidth = 5;
	VideoSettings.GapHeight = 7;
	VideoSettings.NumOfFrames = 8;
	VideoSettings.JPEGQuality =  100;
	VideoSettings.UseAviInfo = TRUE;
	VideoSettings.ShowMediaInfo = TRUE;
	VideoSettings.TextColor = RGB(0, 0, 0);
	
	VideoSettings.Engine = IsFFmpegAvailable() ? VideoEngineAuto : VideoEngineDirectshow;
	ConnectionSettings.UseProxy =  FALSE;
	ConnectionSettings.ProxyPort = 0;
	ConnectionSettings.NeedsAuth = false;
	ConnectionSettings.ProxyType = 0;

	ScreenshotSettings.Format =  1;
	ScreenshotSettings.Quality = 85;
	ScreenshotSettings.WindowHidingDelay = 450;
	ScreenshotSettings.Delay = 1;
	ScreenshotSettings.brushColor = RGB(255, 0, 0);
	ScreenshotSettings.ShowForeground = false;
	ScreenshotSettings.FilenameTemplate = _T("screenshot %y-%m-%d %i");
	ScreenshotSettings.CopyToClipboard = false;
	ScreenshotSettings.RemoveCorners = true;
	ScreenshotSettings.AddShadow = true;
	ScreenshotSettings.RemoveBackground = false;

	if (!WinUtils::IsVista())
	{
		TrayIconSettings.LeftClickCommand = 0; // without action
		TrayIconSettings.LeftDoubleClickCommand = 12; // add images
	}
	else
	{
		TrayIconSettings.LeftClickCommand = 12; // without action
		TrayIconSettings.LeftDoubleClickCommand = 0; // add images
	}
	TrayIconSettings.RightClickCommand = 1; // context menu
	TrayIconSettings.MiddleClickCommand = 7; // region screenshot
	TrayIconSettings.DontLaunchCopy = FALSE;
	TrayIconSettings.TrayScreenshotAction = 0;
	Hotkeys_changed = false;

	/* binding settings */
	SettingsNode& general = mgr_["General"];
	general.n_bind(Language);
	general.n_bind(ExplorerContextMenu);
	general.n_bind(ExplorerVideoContextMenu);
	general.n_bind(ExplorerCascadedMenu);
	general.n_bind(LastUpdateTime);
	general.n_bind(ConfirmOnExit);
	general.n_bind(SendToContextMenu);
	general.n_bind(ParseSubDirs);
	general.n_bind(ImageEditorPath);
	general.n_bind(AutoStartup);
	general.n_bind(ShowTrayIcon);
	general.n_bind(AutoCopyToClipboard);
	general.n_bind(AutoShowLog);
	general.n_bind(ImagesFolder);
	general.n_bind(VideoFolder);
	general.n_bind(WatchClipboard);
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

	SettingsNode& image = mgr_["Image"];
	image["CurrentProfile"].bind(CurrentConvertProfileName);
	image.nm_bind(UploadProfile, KeepAsIs);

	SettingsNode& thumbnails = mgr_["Thumbnails"];
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
	thumbnails["Text"]["@ThumbAlpha"].bind(ThumbSettings.ThumbAlpha);

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

	SettingsNode& tray = mgr_["TrayIcon"];
	tray.nm_bind(TrayIconSettings, LeftDoubleClickCommand);
	tray.nm_bind(TrayIconSettings, LeftClickCommand);
	tray.nm_bind(TrayIconSettings, RightClickCommand);
	tray.nm_bind(TrayIconSettings, MiddleClickCommand);
	tray.nm_bind(TrayIconSettings, DontLaunchCopy);
	tray.nm_bind(TrayIconSettings, TrayScreenshotAction);

	SettingsNode& history = mgr_["History"];
	history.nm_bind(HistorySettings, EnableDownloading);

	SettingsNode& upload = mgr_["Uploading"];
	upload["Server"]["@Name"].bind(imageServer.serverName_);
	upload["Server"]["@Profile"].bind(imageServer.profileName_);	
	imageServer.imageUploadParams.bind(upload["Server"]);

	upload["FileServer"]["@Name"].bind(fileServer.serverName_);
	upload["FileServer"]["@Profile"].bind(fileServer.profileName_);
	fileServer.imageUploadParams.bind(upload["FileServer"]);

	upload.n_bind(QuickUpload);
	upload["QuickServer"]["@Name"].bind(quickServer.serverName_);
	upload["QuickServer"]["@Profile"].bind(quickServer.profileName_);	
	quickServer.imageUploadParams.bind(upload["QuickServer"]);

	upload.n_bind(CodeLang);
	upload.n_bind(ThumbsPerLine);
	upload.n_bind(UploadBufferSize);
	upload.n_bind(UseDirectLinks);
	upload.n_bind(UseTxtTemplate);
	upload.n_bind(CodeType);
	upload.n_bind(FileRetryLimit);
	upload.n_bind(ShowUploadErrorDialog);
	upload.n_bind(ActionRetryLimit);
	SettingsNode& proxy = upload["Proxy"];
	proxy["@UseProxy"].bind(ConnectionSettings.UseProxy);
	proxy["@NeedsAuth"].bind(ConnectionSettings.NeedsAuth);
	proxy.nm_bind(ConnectionSettings, ServerAddress);
	proxy.nm_bind(ConnectionSettings, ProxyPort);
	proxy.nm_bind(ConnectionSettings, ProxyType);
	proxy.nm_bind(ConnectionSettings, ProxyUser);
	proxy.nm_bind(ConnectionSettings, ProxyPassword);
	ConvertProfiles["Default"] = ImageConvertingParams();
	CurrentConvertProfileName = "Default";
}

#endif

bool CSettings::LoadSettings(LPCTSTR szDir, bool LoadFromRegistry) {
	CString FileName = szDir ? CString(szDir) : SettingsFolder + _T("Settings.xml");
	if ( !WinUtils::FileExists( FileName ) )
		return true;
	ZSimpleXml xml;
	xml.LoadFromFile( WCstringToUtf8( FileName ) );
	mgr_.loadFromXmlNode( xml.getRoot("ImageUploader").GetChild("Settings") );

	ZSimpleXmlNode settingsNode = xml.getRoot( "ImageUploader" ).GetChild( "Settings" );

	std::string temp;
	if ( !settingsNode["Image"]["Format"].IsNull() ) {
		// for compatibility with old version configuration file
		LoadConvertProfile( "Old profile", settingsNode );
	}

	LoadConvertProfiles( settingsNode.GetChild("Image").GetChild("Profiles") );
	LoadAccounts( xml.getRoot( "ImageUploader" ).GetChild( "Settings" ).GetChild( "ServersParams" ) );

	// Loading some settings from registry
	if ( LoadFromRegistry ) {
		CRegistry Reg;
		Reg.SetRootKey( HKEY_LOCAL_MACHINE );
		if ( Reg.SetKey("Software\\Zenden.ws\\Image Uploader", false ) ) {
			ExplorerContextMenu = Reg.ReadBool("ExplorerContextMenu");
		}
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
	if ( LoadFromRegistry ) {
		if ( !IsUrlHandlerRegistered() ) {
			if ( WinUtils::IsVista() && !WinUtils::IsElevated() ) {
				ApplyRegistrySettings();
			} else {
				CSettings::ApplyRegSettingsRightNow();
			}
		}
	}
	return true;
}

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
	lstrcpy(Buffer + 1, Extension); // ��������� ������ ���� ".ext"
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
			WriteLog(logWarning, TR("���������"), CString(TR(
			                                                 "�� ���� ������� ������ � ������� ��� ���������� ")) +
			         Extension + _T("\r\n") + WinUtils::FormatWindowsErrorMessage(res));
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

	bool CSettings::SaveSettings()
	{
		ZSimpleXml xml;
		mgr_.saveToXmlNode(xml.getRoot("ImageUploader").GetChild("Settings"));
		SaveConvertProfiles(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("Image").GetChild("Profiles"));

		SaveAccounts(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("ServersParams"));
		xml.SaveToFile(WCstringToUtf8(SettingsFolder + SETTINGS_FILE_NAME));

#ifndef IU_SERVERLISTTOOL
		CRegistry Reg;
		Reg.SetRootKey(HKEY_CURRENT_USER);
		// if(ExplorerContextMenu)
		{
			bool canCreateRegistryKey = ( ExplorerContextMenu );
			if ( Reg.SetKey("Software\\Zenden.ws\\Image Uploader", canCreateRegistryKey ) ) {
				Reg.WriteBool( "ExplorerCascadedMenu", ExplorerCascadedMenu );
				// Reg.WriteBool("ExplorerContextMenu", ExplorerContextMenu);
				Reg.WriteBool( "ExplorerVideoContextMenu", ExplorerVideoContextMenu );
				Reg.WriteString( "Language", Language );
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
			if ( WinUtils::IsVista() && !WinUtils::IsElevated() ) {
				// Start new elevated process 
				ApplyRegistrySettings();
			} else {
				// Process has already admin rights
				ApplyRegSettingsRightNow();
			}
		}

		ExplorerContextMenu_changed = false;
		SendToContextMenu_changed = false;

		if ( !WinUtils::IsVista() || WinUtils::IsElevated() ) {
			MessageBox(0, _T("Registering URL handler"), 0, 0);
			RegisterURLHandler();
		}
		if (ShowTrayIcon_changed)
		{
			ShowTrayIcon_changed = false;
			if (ShowTrayIcon)
			{
				if (!IsRunningFloatingWnd())
				{
					CmdLine.AddParam(_T("/tray"));
					floatWnd->CreateTrayIcon();
				}
			}
			else
			{
				HWND TrayWnd = FindWindow(0, CFloatingWindow::WindowTitle);
				if (TrayWnd) {
					::SendMessage( TrayWnd, CFloatingWindow::WM_CLOSETRAYWND, 0, 0 );
				}
			}
		}
		else if (ShowTrayIcon)
		{
			HWND TrayWnd = FindWindow(0, CFloatingWindow::WindowTitle);
			if (TrayWnd)
				SendMessage(TrayWnd, CFloatingWindow::WM_RELOADSETTINGS,  (floatWnd->m_hWnd) ? 1 : 0, (Settings.Hotkeys_changed) ? 0 : 1);
		}

		Settings.Hotkeys_changed  = false;
#endif
		return true;
	}

// The following code should  be deleted in next releases
	void CSettings::ApplyRegSettingsRightNow()
	{
		// Applying Startup settings
		// EnableAutostartup(AutoStartup);
		RegisterShellExtension( ExplorerContextMenu );
		CSettings::RegisterURLHandler();

		// if(SendToContextMenu_changed)
		{
			CString ShortcutName = WinUtils::GetSendToPath() + _T("\\Image Uploader.lnk");

			if (SendToContextMenu )
			{
				if (WinUtils::FileExists(ShortcutName))
					DeleteFile(ShortcutName);

				WinUtils::CreateShortCut(ShortcutName, CmdLine.ModuleName(), WinUtils::GetAppFolder(), _T(
				                  " /upload"), 0, SW_SHOW, CmdLine.ModuleName(), 0);
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
					   AddToExplorerContextMenu(szList, (/*ExplorerImagesContextMenu?TR("��������� �����������"):*/ 0),
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
					   AddToExplorerContextMenu(szList, (/*ExplorerVideoContextMenu?TR("������� � Image Uploader"):*/ 0),
					                            szFileName,
					                            false);
					szList += lstrlen(szList) + 1;
				}
			}
		}
	}

	ServerSettingsStruct& CSettings::ServerByName(CString name)
	{
		return ServersSettings[name][_T("")];
	}

	ServerSettingsStruct& CSettings::ServerByUtf8Name(std::string name)
	{
		return ServerByName(Utf8ToWCstring(name));
	}

	bool CSettings::LoadAccounts(ZSimpleXmlNode root)
	{
		std::vector<ZSimpleXmlNode> servers;
		root.GetChilds("Server", servers);

		for (size_t i = 0; i < servers.size(); i++)
		{
			std::string server_name = servers[i].Attribute("Name");
			std::string profileName = servers[i].Attribute("ProfileName");
			std::vector<std::string> attribs;
			servers[i].GetAttributes(attribs);
			CString wideName = Utf8ToWCstring(server_name);
			CString wideProfileName = Utf8ToWCstring(profileName);
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
						ServersSettings[wideName][wideProfileName].params[attribName] = value;
				}
			}
			ServersSettings[wideName][wideProfileName].authData.DoAuth = servers[i].AttributeBool("Auth");

			std::string encodedLogin = servers[i].Attribute("Login");
			CEncodedPassword login;
			login.fromEncodedData(encodedLogin.c_str());
			ServersSettings[wideName][wideProfileName].authData.Login = WCstringToUtf8(login);

			std::string encodedPass = servers[i].Attribute("Password");
			CEncodedPassword pass;
			pass.fromEncodedData(encodedPass.c_str());
			ServersSettings[wideName][wideProfileName].authData.Password = WCstringToUtf8(pass);
		}
		return true;
	}

	bool CSettings::SaveAccounts(ZSimpleXmlNode root)
	{
		std::map <CString, std::map<CString,ServerSettingsStruct> >::iterator it2;
		std::map<CString,ServerSettingsStruct>::iterator it;
		for (it2 = ServersSettings.begin(); it2!= ServersSettings.end(); ++it2)
		{
			for ( it = it2->second.begin(); it != it2->second.end(); ++it) {
				if ( it->second.isEmpty()) {
					continue;
				}
				ZSimpleXmlNode serverNode = root.CreateChild("Server");
				serverNode.SetAttribute("Name", WCstringToUtf8(it2->first));
				serverNode.SetAttribute("ProfileName", WCstringToUtf8(it->first));

				std::map <std::string, std::string>::iterator param;
				for (param = it->second.params.begin(); param != it->second.params.end(); ++param)
				{
					serverNode.SetAttribute("_" + param->first, param->second);
				}

				serverNode.SetAttributeBool("Auth", it->second.authData.DoAuth);

				CEncodedPassword login(Utf8ToWCstring(it->second.authData.Login));
				serverNode.SetAttribute("Login", WCstringToUtf8(login.toEncodedData()));

				CEncodedPassword pass(Utf8ToWCstring(it->second.authData.Password));
				serverNode.SetAttribute("Password", WCstringToUtf8(pass.toEncodedData()));
			}
		}
		return true;
	}

	bool CSettings::LoadConvertProfiles(ZSimpleXmlNode root)
	{
		std::vector<ZSimpleXmlNode> profiles;
		root.GetChilds("Profile", profiles);

		for (size_t i = 0; i < profiles.size(); i++)
		{
			LoadConvertProfile("", profiles[i]);
		}
		return true;
	}

	bool CSettings::LoadConvertProfile(const CString& name, ZSimpleXmlNode profileNode)
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

	bool CSettings::SaveConvertProfiles(ZSimpleXmlNode root)
	{
		std::map<CString, ImageConvertingParams>::iterator it;
		for (it = ConvertProfiles.begin(); it != ConvertProfiles.end(); ++it)
		{
			ZSimpleXmlNode profile = root.CreateChild("Profile");
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
		image.nm_bind(params, AllowImageEnlarging);
		image["Logo"].bind(params.LogoFileName);
		image["Logo"]["@LogoPosition"].bind(params.LogoPosition);
		image["Logo"]["@LogoBlend"].bind(params.LogoBlend);
		image["Text"].bind(params.Text);
		image["Text"]["@TextPosition"].bind(params.TextPosition);
		image["Text"]["@TextColor"].bind(params.TextColor);
		image["Text"]["@StrokeColor"].bind(params.StrokeColor);
		image["Text"]["@Font"].bind(params.Font);
	}

CSettings::~CSettings() {
}

void CSettings::Uninstall() {
	BOOL b;
	if (WinUtils::IsVista() && WinUtils::IsElevated()) {
		RunIuElevated("/uninstall");
		return;
	}
	AutoStartup = false;
	SendToContextMenu  = false;
	RegisterShellExtension(false);
	EnableAutostartup(false);
	CRegistry Reg;
	Reg.SetRootKey(HKEY_CURRENT_USER);
	Reg.DeleteKey( "Software\\Zenden.ws\\Image Uploader" );
	Reg.SetRootKey( HKEY_LOCAL_MACHINE );
	Reg.DeleteKey( "Software\\Zenden.ws\\Image Uploader" );

	CString ShortcutName = WinUtils::GetSendToPath() + _T("\\Image Uploader.lnk");
	DeleteFile(ShortcutName);
}

void CSettings::EnableAutostartup(bool enable) {
	CRegistry Reg;
	Reg.SetRootKey( HKEY_CURRENT_USER );
	bool canCreateRegistryKey = enable;

	if ( Reg.SetKey( "Software\\Zenden.ws\\Image Uploader", canCreateRegistryKey ) ) {
		Reg.WriteBool( "AutoStartup", enable );
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

bool CSettings::IsFFmpegAvailable() {
	CString appFolder = WinUtils::GetAppFolder();
	return WinUtils::FileExists( appFolder + "avcodec-53.dll") 
		 && WinUtils::FileExists( appFolder + "avformat-53.dll")
		 && WinUtils::FileExists( appFolder + "avutil-51.dll")
		 && WinUtils::FileExists( appFolder + "swscale-2.dll");
}

CString CSettings::prepareVideoDialogFilters() {
	CString result;
	std::vector<std::string>& extensions = VideoUtils::Instance().videoFilesExtensions;
	for ( int i = 0; i < extensions.size(); i++ ) {
		result += CString("*.") + CString(extensions[i].c_str()) + _T(";");
	}
	return result;
}

bool CSettings::RegisterURLHandler() {
	CRegistry Reg;
	Reg.SetRootKey( HKEY_CLASSES_ROOT );
	bool canCreateRegistryKey = true;
	CString handlerName = URLHandlerName();

	if ( Reg.SetKey( handlerName, canCreateRegistryKey ) ) {
		Reg.WriteString(_T(""), _T("URL:imageuploader Protocol") );
		Reg.WriteString(_T("URL Protocol"), _T("") );
		CRegistry Reg2;
		Reg2.SetRootKey(HKEY_CLASSES_ROOT);
		if ( Reg2.SetKey( handlerName + "\\shell\\open\\command", true ) ) {
			CString command;
			command.Format(_T("\"%s\" /openUrl \"%%1\""), WinUtils::GetAppFileName());
			Reg2.WriteString( _T(""),  command);
		}
	}
	return true;
}

bool CSettings::IsUrlHandlerRegistered() {
	CRegistry Reg;
	Reg.SetRootKey( HKEY_CLASSES_ROOT );
	if ( Reg.SetKey( URLHandlerName() + "\\shell\\open\\command", false ) ) {
		CString command = Reg.ReadString(_T(""));
		
		CCmdLine cmdLine(command);
		if ( cmdLine.GetCount() < 3 ) {
			return false;
		}
		CString appFileName = cmdLine[0];

		if ( WinUtils::FileExists( appFileName ) ) {
			return true;
		}
	} 
	return false;
}

CString CSettings::URLHandlerName() {
	return _T("imageuploader");
}

CString CSettings::ServerName() {
	return imageServer.serverName();
}
CString CSettings::QuickServerName() {
	return quickServer.serverName();
}
CString CSettings::FileServerName() {
	return fileServer.serverName();
}

int CSettings::ServerID() {
	return _EngineList->GetUploadEngineIndex(ServerName());
}
int CSettings::QuickServerID() {
	return _EngineList->GetUploadEngineIndex(QuickServerName());
}
int CSettings::FileServerID() {
	return _EngineList->GetUploadEngineIndex(FileServerName());
}
CEncodedPassword::CEncodedPassword() {
}

CEncodedPassword::CEncodedPassword(CString d) { 
	data_ = d; 
}

CString CEncodedPassword::toEncodedData() const {
	CString res;
	EncodeString(data_, res);
	return res;
}
void CEncodedPassword::fromPlainText(CString data) {
	data = data_;
}

void CEncodedPassword::fromEncodedData(CString data) {
	DecodeString(data, data_);
}
	
CEncodedPassword::operator CString&() {
	return data_;
}

CEncodedPassword::operator const TCHAR*() {
	return data_;
}

CEncodedPassword& CEncodedPassword::operator=(const CString& text) {
	data_ = text;
	return *this;
}