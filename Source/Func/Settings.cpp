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
#ifdef _WIN32
#include <Shlobj.h>
#endif
#include "Core/SettingsManager.h"
#ifndef IU_CLI
#include "Func/myutils.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Common/CmdLine.h"
#include "3rdpart/Registry.h"
#include <Core/Video/VideoUtils.h>
#include "WinUtils.h"
#endif
#include <stdlib.h>

#include <Core/Utils/StringUtils.h>

#if !defined(IU_SERVERLISTTOOL) && !defined(IU_CLI)
	#include "Gui/Dialogs/FloatingWindow.h"
#endif

#ifndef CheckBounds
	#define CheckBounds(n, a, b, d) {if ((n < a) || (n > b)) n = d; }
#endif

#define SETTINGS_FILE_NAME _T("settings.xml")

#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
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
	if (IsDirectory(WinUtils::GetAppFolder() + _T("Data"))) {
		DataFolder     = WinUtils::GetAppFolder() + _T("Data\\");
		SettingsFolder = IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(DataFolder));
		return;
	}

	SettingsFolder =  IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(GetApplicationDataPath() + _T("\\Image Uploader\\")));
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
				return;
			}
		}
	}

	if (FileExists(GetCommonApplicationDataPath() + SETTINGS_FILE_NAME)) {
		DataFolder = GetCommonApplicationDataPath() + _T("Image Uploader\\");
	}
	else 
		#endif
	
	{
		DataFolder = GetApplicationDataPath() + _T("Image Uploader\\");
	}
}
#endif

CSettings::CSettings()
#if !defined(IU_SHELLEXT) && !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	: ServerID(UploadProfile.ServerID), QuickServerID(UploadProfile.QuickServerID)
#endif
{
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	FindDataFolder();
	if (!IsDirectory(DataFolder))
	{
		CreateDirectory(DataFolder, 0);
	}
	if (!IsDirectory(IuCoreUtils::Utf8ToWstring(SettingsFolder).c_str()))
	{
		CreateDirectory(IuCoreUtils::Utf8ToWstring(SettingsFolder).c_str(), 0);
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
	UploadBufferSize = 65536;
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
	ServerID = 0;
	CodeLang = 0;
	ConfirmOnExit = 1;
	ExplorerContextMenu = false;
	ExplorerVideoContextMenu = false;
	ExplorerContextMenu_changed = false;
	ThumbsPerLine = 4;
	SendToContextMenu_changed = false;
	SendToContextMenu = 0;
	QuickServerID = 0;
	QuickUpload = 0;
	ParseSubDirs = 1;
	
	ShowUploadErrorDialog = true;

	ImageEditorPath = _T("mspaint.exe \"%1\"");
	AutoCopyToClipboard = false;
	AutoShowLog = true;
	

	StringToFont(_T("Tahoma,7,b,204"), &ThumbSettings.ThumbFont);
	StringToFont(_T("Tahoma,8,,204"), &VideoSettings.Font);

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
	ScreenshotSettings.RemoveCorners = true;
	ScreenshotSettings.AddShadow = true;
	ScreenshotSettings.RemoveBackground = false;


	TrayIconSettings.LeftClickCommand = 0; // without action
	TrayIconSettings.LeftDoubleClickCommand = 12; 

	TrayIconSettings.RightClickCommand = 1; // context menu
	TrayIconSettings.MiddleClickCommand = 7; // region screenshot
	TrayIconSettings.DontLaunchCopy = FALSE;
	TrayIconSettings.ShortenLinks = FALSE;
	TrayIconSettings.TrayScreenshotAction = 0;

	ImageReuploaderSettings.PasteHtmlOnCtrlV = true;
	Hotkeys_changed = false;
#endif
	
	/* binding settings */
	SettingsNode& general = mgr_["General"];
		general.n_bind(LastUpdateTime);
#if !defined(IU_SHELLEXT) && !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
		general.n_bind(Language);
		general.n_bind(ExplorerContextMenu);
		general.n_bind(ExplorerVideoContextMenu);
		general.n_bind(ExplorerCascadedMenu);
#endif
		#if !defined(IU_SHELLEXT) && !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	


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
	upload.n_bind(UrlShorteningServer);
	upload.n_bind(QuickUpload);
	upload.n_bind(QuickServerName);
	upload.n_bind(CodeLang);
	upload.n_bind(ThumbsPerLine);
	upload.n_bind(UseDirectLinks);
	upload.n_bind(UseTxtTemplate);
	upload.n_bind(CodeType);
	upload.n_bind(ShowUploadErrorDialog);

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
	if ( !IuCoreUtils::FileExists( fileName_)  ) {
		return true;
	}
	ZSimpleXml xml;
	xml.LoadFromFile( fileName_ );
	mgr_.loadFromXmlNode( xml.getRoot("ImageUploader").GetChild("Settings") );

	ZSimpleXmlNode settingsNode = xml.getRoot( "ImageUploader" ).GetChild( "Settings" );

#if !defined(IU_CLI) && !defined( IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
	std::string temp;
	if ( !settingsNode["Image"]["Format"].IsNull() ) {
		// for compatibility with old version configuration file
		LoadConvertProfile( "Old profile", settingsNode );
	}


	LoadConvertProfiles( settingsNode.GetChild("Image").GetChild("Profiles") );
#endif
	LoadAccounts( xml.getRoot( "ImageUploader" ).GetChild( "Settings" ).GetChild( "ServersParams" ) );

#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
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
		ZSimpleXml xml;
		mgr_.saveToXmlNode(xml.getRoot("ImageUploader").GetChild("Settings"));
#if !defined(IU_SERVERLISTTOOL) && !defined  (IU_CLI) && !defined(IU_SHELLEXT)
		SaveConvertProfiles(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("Image").GetChild("Profiles"));
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
		return ServersSettings[IuCoreUtils::WstringToUtf8((LPCTSTR)name)];
	}

#endif
	ServerSettingsStruct& CSettings::ServerByUtf8Name(std::string name)
	{
		return ServersSettings[name];
	}

	bool CSettings::LoadAccounts(ZSimpleXmlNode root)
	{
		std::vector<ZSimpleXmlNode> servers;
		root.GetChilds("Server", servers);

		for (size_t i = 0; i < servers.size(); i++)
		{
			std::string server_name = servers[i].Attribute("Name");
			std::vector<std::string> attribs;
			servers[i].GetAttributes(attribs);


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
						ServersSettings[server_name].params[attribName] = value;
				}
			}
#if !defined  (IU_CLI) && !defined(IU_SHELLEXT)
			ServersSettings[server_name].authData.DoAuth = servers[i].AttributeBool("Auth");

			std::string encodedLogin = servers[i].Attribute("Login");
			CEncodedPassword login;
			login.fromEncodedData(encodedLogin.c_str());
			ServersSettings[server_name].authData.Login = WCstringToUtf8(login);

			std::string encodedPass = servers[i].Attribute("Password");
			CEncodedPassword pass;
			pass.fromEncodedData(encodedPass.c_str());
			ServersSettings[server_name].authData.Password = WCstringToUtf8(pass);
#endif
		}
		return true;
	}

	bool CSettings::SaveAccounts(ZSimpleXmlNode root)
	{
		std::map <std::string, ServerSettingsStruct>::iterator it;
		for (it = ServersSettings.begin(); it != ServersSettings.end(); ++it)
		{
			ZSimpleXmlNode serverNode = root.CreateChild("Server");
			serverNode.SetAttribute("Name", it->first);

			std::map <std::string, std::string>::iterator param;
			for (param = it->second.params.begin(); param != it->second.params.end(); ++param)
			{
				serverNode.SetAttribute("_" + param->first, param->second);
			}
#if !defined  (IU_CLI) && !defined(IU_SHELLEXT)
			serverNode.SetAttributeBool("Auth", it->second.authData.DoAuth);

			CEncodedPassword login(Utf8ToWCstring(it->second.authData.Login));
			serverNode.SetAttribute("Login", WCstringToUtf8(login.toEncodedData()));

			CUploadEngineData* ued = _EngineList->byName(Utf8ToWCstring(it->first));
			if ( !ued || ued->NeedPassword ) { 
				CEncodedPassword pass(Utf8ToWCstring(it->second.authData.Password));
				serverNode.SetAttribute("Password", WCstringToUtf8(pass.toEncodedData()));
			}
#endif
		}
		return true;
	}

#if !defined  (IU_CLI) && !defined(IU_SHELLEXT) && !defined(IU_SERVERLISTTOOL)
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
	Reg.DeleteKey( "Software\\Zenden.ws\\Image Uploader" );
	Reg.SetRootKey( HKEY_LOCAL_MACHINE );
	Reg.DeleteKey( "Software\\Zenden.ws\\Image Uploader" );

	CString ShortcutName = GetSendToPath() + _T("\\Image Uploader.lnk");
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
	return FileExists( appFolder + "avcodec-53.dll") 
		 && FileExists( appFolder + "avformat-53.dll")
		 && FileExists( appFolder + "avutil-51.dll")
		 && FileExists( appFolder + "swscale-2.dll");
}

CString CSettings::prepareVideoDialogFilters() {
	CString result;
	std::vector<std::string>& extensions = VideoUtils::Instance().videoFilesExtensions;
	for ( int i = 0; i < extensions.size(); i++ ) {
		result += CString("*.") + CString(extensions[i].c_str()) + _T(";");
	}
	return result;
}
#endif
