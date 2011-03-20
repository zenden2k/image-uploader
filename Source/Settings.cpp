/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#include "settings.h"
#include "myutils.h"
#include "LogWindow.h"
#include <Shlobj.h>
#include "Common\CmdLine.h"
#include "3rdpart/Registry.h"
#include "Core/SettingsManager.h"

/* CString support for  SettingsManager */
			
inline std::string myToString(const CString& value)
{
	return WCstringToUtf8(value);
}

inline void myFromString(const std::string& text, CString& value)
{
	value= Utf8ToWCstring(text);
}
		
/* LOGFONT serialization support */				
inline std::string myToString(const LOGFONT& value)
{
	CString res;
	FontToString(&value, res);
	return WCstringToUtf8(res);
}

inline void myFromString(const std::string& text, LOGFONT& value)
{
	CString wide_text = Utf8ToWCstring(text);
	LOGFONT font;
	StringToFont(wide_text, &font);
	value= font;
}

inline std::string myToString(const CEncodedPassword& value)
{
	return WCstringToUtf8(value.toEncodedData());
}

inline void myFromString(const std::string& text, CEncodedPassword& value)
{
	value.fromEncodedData(Utf8ToWCstring(text));
}

inline std::string myToString(const CHotkeyList& value)
{
	return WCstringToUtf8(value.toString());
}

inline void myFromString(const std::string& text, CHotkeyList& value)
{
	value.DeSerialize(Utf8ToWCstring(text));
}

#define SETTINGS_FILE_NAME _T("settings.xml")

CSettings Settings;

#ifndef IU_SERVERLISTTOOL
	#include "FloatingWindow.h"
#endif

#define ASSERT

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}


void ApplyRegistrySettings()
{
	SHELLEXECUTEINFO TempInfo = {0};

	TCHAR buf[MAX_PATH];
	GetModuleFileName(0,buf,MAX_PATH-1);
	CString s=GetAppFolder();

	CString Command = CString(buf);
	TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask = 0;
	TempInfo.hwnd = NULL;
	TempInfo.lpVerb = _T("runas");
	TempInfo.lpFile = Command;
	TempInfo.lpParameters = _T(" /integration");
	TempInfo.lpDirectory = s;
	TempInfo.nShow = SW_NORMAL;

	::ShellExecuteEx(&TempInfo);
}

void RegisterShellExtension(bool Register)
{
	if(!FileExists(GetAppFolder()+_T("ExplorerIntegration.dll"))) return;
	SHELLEXECUTEINFO TempInfo = {0};
	CString s=GetAppFolder();
	TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask = 0;
	TempInfo.hwnd = NULL;
	if(IsVista())
		TempInfo.lpVerb = _T("runas");
	else 
		TempInfo.lpVerb = _T("open");
	TempInfo.lpFile = _T("regsvr32");
	TempInfo.lpParameters =CString((Register?_T(""):_T("/u ")))+ _T("/s \"")+GetAppFolder()+_T("ExplorerIntegration.dll\"");
	TempInfo.lpDirectory = s;
	TempInfo.nShow = SW_NORMAL;
	::ShellExecuteEx(&TempInfo);
}

CSettings::CSettings()
#ifndef IU_SHELLEXT
: ServerID(ImageSettings.ServerID),QuickServerID(ImageSettings.QuickServerID)
#endif
{
	if(IsDirectory(GetAppFolder() + _T("Data")))
	{	
		DataFolder = GetAppFolder() + _T("Data\\");
	}
	else 
	{
		if(FileExists(GetCommonApplicationDataPath()+ SETTINGS_FILE_NAME))
		{
			DataFolder = GetCommonApplicationDataPath() + _T("Image Uploader\\");
		}
		else
		{
			DataFolder = GetApplicationDataPath() + _T("Image Uploader\\");
		}
	}

	if(!IsDirectory(DataFolder))
	{
		CreateDirectory(DataFolder,0);
	}

	CString copyFrom = GetAppFolder()+SETTINGS_FILE_NAME;
	CString copyTo = DataFolder+SETTINGS_FILE_NAME;
	if(FileExists(copyFrom) && !FileExists(copyTo))
	{
		MoveFile(copyFrom, copyTo);
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
	FileRetryLimit = 3;
	ShowUploadErrorDialog = true;
	ActionRetryLimit = 2;	
	ImageEditorPath = _T("mspaint.exe \"%1\"");
	AutoCopyToClipboard = false;
	AutoShowLog = true;
	UploadBufferSize = 65536;
	ImageSettings.KeepAsIs = false;
	ImageSettings.NewWidth = 0;
	ImageSettings.NewHeight = 0;
	ImageSettings.AddLogo  = false;
	ImageSettings.AddText = false;
	ImageSettings.Format = 1;
	ImageSettings.Quality = 85;
	ImageSettings.SaveProportions = true;

	ImageSettings.LogoPosition = 0;
	ImageSettings.LogoBlend = 0;
	ImageSettings.Text = APPNAME;
	ImageSettings.TextPosition = 5;
	ImageSettings.TextColor = 0xffffffff;
	
	StringToFont(_T("Tahoma,8,,204"), &ImageSettings.Font);
	StringToFont(_T("Tahoma,7,b,204"), &ThumbSettings.ThumbFont);
	StringToFont(_T("Tahoma,8,,204"), &VideoSettings.Font);

	ThumbSettings.CreateThumbs = true;
	ThumbSettings.ThumbWidth = 180;
	ThumbSettings.DrawFrame = true;
	ThumbSettings.ThumbAddImageSize  = true;
	ThumbSettings.FrameColor = RGB( 0, 74, 111) ;
	ImageSettings.StrokeColor = RGB( 0, 0, 0);
	ThumbSettings.ThumbColor1 =  RGB( 13, 86, 125);
	ThumbSettings.ThumbColor2 = RGB( 6, 174, 255);
	ThumbSettings.UseServerThumbs = false;
	ThumbSettings.UseThumbTemplate = true;
	ThumbSettings.ThumbTextColor = RGB( 255, 255, 255);
	ThumbSettings.ThumbAlpha = 120;
	ThumbSettings.Text = _T("%width%x%height% (%size%)");
	ThumbSettings.ThumbFormat = tfSameAsImageFormat;
	ThumbSettings.thumbFileName = "default";
	VideoSettings.Columns = 3;
	VideoSettings.TileWidth =  200;
	VideoSettings.GapWidth = 5;
	VideoSettings.GapHeight = 7;
	VideoSettings.NumOfFrames = 8;
	VideoSettings.JPEGQuality =  100;
	VideoSettings.UseAviInfo = TRUE;
	VideoSettings.ShowMediaInfo = TRUE;
	VideoSettings.TextColor = RGB(0,0,0);

	ConnectionSettings.UseProxy =  FALSE;
	ConnectionSettings.ProxyPort= 0;
	ConnectionSettings.NeedsAuth = false;
	ConnectionSettings.ProxyType = 0;
	
	ScreenshotSettings.Format =  1;
	ScreenshotSettings.Quality = 85;
	ScreenshotSettings.WindowHidingDelay = 450;
	ScreenshotSettings.Delay = 1;
	ScreenshotSettings.brushColor = RGB(255,0,0);
	ScreenshotSettings.ShowForeground = false;
	ScreenshotSettings.FilenameTemplate = _T("screenshot %y-%m-%d %c");
	ScreenshotSettings.CopyToClipboard = false;
	if(!IsVista())
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
		general.n_bind(ShowTrayIcon);
		general.n_bind(AutoCopyToClipboard);
		general.n_bind(AutoShowLog);
		general.n_bind(ImagesFolder);
		general.n_bind(VideoFolder);
		general.n_bind(WatchClipboard);
		general.n_bind(Hotkeys);
		 
	SettingsNode& screenshot = mgr_["Screenshot"];
		screenshot.nm_bind(ScreenshotSettings,Delay);
		screenshot.nm_bind(ScreenshotSettings,Format);
		screenshot.nm_bind(ScreenshotSettings,Quality);
		screenshot.nm_bind(ScreenshotSettings,ShowForeground);
		screenshot.nm_bind(ScreenshotSettings,FilenameTemplate);
		screenshot.nm_bind(ScreenshotSettings,Folder);
		screenshot.nm_bind(ScreenshotSettings,CopyToClipboard);
		screenshot.nm_bind(ScreenshotSettings,brushColor);
	
	SettingsNode& image = mgr_["Image"];
		image.nm_bind(ImageSettings, Quality);
		image.nm_bind(ImageSettings, Format);
		image.nm_bind(ImageSettings, KeepAsIs);
		image.nm_bind(ImageSettings, NewWidth);
		image.nm_bind(ImageSettings, NewHeight);
		image.nm_bind(ImageSettings, AddLogo);
		image.nm_bind(ImageSettings, AddText);

	image["Logo"].bind(ImageSettings.LogoFileName);
	image["Logo"]["@LogoPosition"].bind(ImageSettings.LogoPosition);
	image["Logo"]["@LogoBlend"].bind(ImageSettings.LogoBlend);
			
	image["Text"].bind(ImageSettings.Text);
	image["Text"]["@TextPosition"].bind(ImageSettings.TextPosition);		
	image["Text"]["@TextColor"].bind(ImageSettings.TextColor);	
	image["Text"]["@Font"].bind(ImageSettings.Font);


	SettingsNode& thumbnails = mgr_["Thumbnails"];
		thumbnails.nm_bind(ThumbSettings, CreateThumbs);

	
		thumbnails.nm_bind(ThumbSettings, CreateThumbs);
		thumbnails.nm_bind(ThumbSettings, ThumbWidth);
		thumbnails.nm_bind(ThumbSettings, FrameColor);
		thumbnails.nm_bind(ImageSettings, StrokeColor);
		thumbnails.nm_bind(ThumbSettings, ThumbColor1);
		thumbnails.nm_bind(ThumbSettings, ThumbColor2);
		thumbnails.nm_bind(ThumbSettings, UseServerThumbs);
		thumbnails.nm_bind(ThumbSettings, UseThumbTemplate);
		thumbnails.nm_bind(ThumbSettings, ThumbAddImageSize);
		thumbnails.nm_bind(ThumbSettings, DrawFrame);
		
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

	SettingsNode& tray = mgr_["TrayIcon"];
		tray.nm_bind(TrayIconSettings, LeftDoubleClickCommand);
		tray.nm_bind(TrayIconSettings, LeftClickCommand);
		tray.nm_bind(TrayIconSettings, RightClickCommand);
		tray.nm_bind(TrayIconSettings, MiddleClickCommand);
		tray.nm_bind(TrayIconSettings, DontLaunchCopy);
		tray.nm_bind(TrayIconSettings, TrayScreenshotAction);
	
	SettingsNode& upload = mgr_["Uploading"];
		upload.n_bind(ServerName);
		upload.n_bind(FileServerName);
		upload.n_bind(QuickUpload);
		upload.n_bind(QuickServerName);
		upload.n_bind(CodeLang);
		upload.n_bind(ThumbsPerLine);
		upload.n_bind(UploadBufferSize);
		upload.n_bind(UseDirectLinks);
		upload.n_bind(UseTxtTemplate);
		upload.n_bind(CodeType);
		upload.n_bind(FileRetryLimit);
		upload.n_bind(ShowUploadErrorDialog);
		upload.n_bind(ActionRetryLimit);
	//	upload.n_bind(Proxy);	
		SettingsNode& proxy = upload["Proxy"];
		proxy["@UseProxy"].bind(ConnectionSettings.UseProxy);
		proxy["@NeedsAuth"].bind(ConnectionSettings.NeedsAuth);
		proxy.nm_bind(ConnectionSettings,ServerAddress);
		proxy.nm_bind(ConnectionSettings, ProxyPort);
		proxy.nm_bind(ConnectionSettings, ProxyType);
		proxy.nm_bind(ConnectionSettings, ProxyUser);
		proxy.nm_bind(ConnectionSettings, ProxyPassword);
}
#endif

bool CSettings::LoadSettings(LPCTSTR szDir)
{
	
	CString FileName= szDir? CString(szDir):IU_GetDataFolder()+_T("Settings.xml");
	if(!FileExists(FileName)) return true;
	
	/*if(!MyXML.Load(FileName))
	{
		MessageBox(0, MyXML.GetError(),0,0);
	}*/
	 //MacroLoadSettings(MyXML);
	 ZSimpleXml xml;
	 
	 xml.LoadFromFile(WCstringToUtf8(FileName));
	 mgr_.loadFromXmlNode(xml.getRoot("ImageUploader"));
	 LoadAccounts(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("ServersParams"));
	
	
	 return true;
}

#define MY_CLSID _T("{535E39BD-5883-454C-AFFC-C54B66B18206}")

bool RegisterClsId()
{
	TCHAR Buffer[MAX_PATH+1]=_T("CLSID\\");
	HKEY Key=0;
	
	lstrcat(Buffer, MY_CLSID);
	RegCreateKeyEx(HKEY_CLASSES_ROOT, Buffer, 0, 0, REG_OPTION_NON_VOLATILE,KEY_WRITE, 0, &Key, NULL);
  
	if(!Key)  return false;

	HKEY TempKey=0;
	RegCreateKeyEx(Key, _T("LocalServer32"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE		, 0, &TempKey, NULL);

	GetModuleFileName(0, Buffer, MAX_PATH);
	
	RegSetValueEx(TempKey,	0,	0,	REG_SZ, (LPBYTE) Buffer, (lstrlen(Buffer)+1)*sizeof(TCHAR));
	
	RegCloseKey(TempKey);

	RegCreateKeyEx(Key, _T("ProgID"), 0, 0, REG_OPTION_NON_VOLATILE,KEY_WRITE	, 0, &TempKey, NULL);
	lstrcpy(Buffer, _T("ImageUploader.ContextMenuHandler.1"));
	RegSetValueEx(TempKey,	0,	0,	REG_SZ, (LPBYTE) Buffer, (lstrlen(Buffer)+1)*sizeof(TCHAR));
	RegCloseKey(TempKey);

	RegCloseKey(Key);
 
	Key = 0;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, KEY_WRITE		,  &Key)==ERROR_SUCCESS)
	{
		lstrcpy(Buffer, _T("ImageUploader ContextMenuHandler"));
		RegSetValueEx(Key,MY_CLSID	,	0,	REG_SZ, (LPBYTE) Buffer, (lstrlen(Buffer)+1)*sizeof(TCHAR));
		RegCloseKey(Key);
	}
	return true;
}

bool UnRegisterClsId() // Deleting CLSID record from registry
{
	TCHAR Buffer[MAX_PATH+1] = _T("CLSID\\");
	lstrcat(Buffer, MY_CLSID);
	return SHDeleteKey(HKEY_CLASSES_ROOT,Buffer)==ERROR_SUCCESS;
}

int AddToExplorerContextMenu(LPCTSTR Extension, LPCTSTR Title, LPCTSTR Command,bool DropTarget) 
{
	HKEY ExtKey = NULL;
	TCHAR Buffer[MAX_PATH];

	Buffer[0] = _T('.');
	lstrcpy(Buffer+1, Extension); //Формируем строку вида ".ext"
	RegCreateKeyEx(HKEY_CLASSES_ROOT, Buffer, 0, 0, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, 0, &ExtKey, NULL);

	TCHAR ClassName[MAX_PATH]=_T("\0");
	DWORD BufSize = sizeof(ClassName)/sizeof(TCHAR);
	DWORD Type = REG_SZ;
   RegQueryValueEx(ExtKey,	0, 0, &Type, (LPBYTE)&ClassName, &BufSize); // Retrieving classname for extension
	RegCloseKey(ExtKey);

	if(Title == 0)
	{
		// Deleting
		wsprintf(Buffer, _T("%s\\shell\\iuploader"), (LPCTSTR)ClassName);
		SHDeleteKey(HKEY_CLASSES_ROOT, Buffer);
		return 0;
	}

	wsprintf(Buffer, _T("%s\\shell\\iuploader\\command"), (LPCTSTR)ClassName);

	if(!lstrlen(Buffer)) return false;
	wsprintf(Buffer, _T("%s\\shell\\iuploader"), (LPCTSTR)ClassName);
	DWORD res  = RegCreateKeyEx(HKEY_CLASSES_ROOT, Buffer, 0, 0, REG_OPTION_NON_VOLATILE,  KEY_WRITE	, 0, &ExtKey, NULL);
	
	if(res != ERROR_SUCCESS)
	{
		WriteLog(logWarning, TR("Настройки"),CString(TR("Не могу создать запись в реестре для расширения "))+Extension+_T("\r\n")+DisplayError(res));
		return 0;
	}

	RegSetValueEx(ExtKey, 0, 0, REG_SZ,	(LPBYTE)Title,	(lstrlen(Title)+1)*sizeof(TCHAR));

	HKEY CommandKey;
	
	if(RegCreateKeyEx(ExtKey, _T("command"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE	, 0, &CommandKey, NULL)!=ERROR_SUCCESS)
	{
	
	}
	HKEY DropTargetKey;
	if(DropTarget)
	{
		if(RegCreateKeyEx(ExtKey, _T("DropTarget"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE	, 0, &DropTargetKey, NULL)!=ERROR_SUCCESS)
		{		
			return 0;
		}
	}

	RegCloseKey(ExtKey);

	RegSetValueEx(CommandKey, 0, 0,	REG_SZ, (LPBYTE)Command, (lstrlen(Command)+1)*sizeof(TCHAR));
	RegCloseKey(CommandKey);

	if(DropTarget)
	{
		RegSetValueEx(DropTargetKey, _T("Clsid"), 0, REG_SZ,	(LPBYTE)MY_CLSID, (lstrlen(MY_CLSID)+1)*sizeof(TCHAR));
	}

	RegCloseKey(DropTargetKey);
	return 1; // That's means ALL OK! :)
}

bool CSettings::SaveSettings()
{	
	ZSimpleXml xml;
	mgr_.saveToXmlNode(xml.getRoot("ImageUploader"));
	SaveAccounts(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("ServersParams"));
	xml.SaveToFile(WCstringToUtf8(IU_GetDataFolder()+SETTINGS_FILE_NAME));

	#ifndef IU_SERVERLISTTOOL
	
	/*CMyXml MyXml;
	MyXml.SetDoc(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"));

	MacroSaveSettings(MyXml);
	MyXml.Save(IU_GetDataFolder()+SETTINGS_FILE_NAME);
*/
	CRegistry Reg;
	Reg.SetRootKey(HKEY_CURRENT_USER);
	if (Reg.SetKey("Software\\Image Uploader", TRUE))
	{
		Reg.WriteBool("ExplorerCascadedMenu", ExplorerCascadedMenu);
		Reg.WriteBool("ExplorerContextMenu", ExplorerContextMenu);
		Reg.WriteBool("ExplorerVideoContextMenu", ExplorerVideoContextMenu);
		Reg.WriteString("Language", Language);
	}

	if(SendToContextMenu_changed || ExplorerContextMenu_changed || ShowTrayIcon_changed) 
	{
		BOOL b;
		if(!Settings.ShowTrayIcon_changed)
			RegisterShellExtension(ExplorerContextMenu);

		if(IsVista() && IsElevated(&b)!=S_OK)
			ApplyRegistrySettings();
		else 
		{
			ApplyRegSettingsRightNow();
		}
	}
	
	ExplorerContextMenu_changed = false;
	SendToContextMenu_changed = false;

	if(ShowTrayIcon_changed)
	{
		ShowTrayIcon_changed = false;
		if(ShowTrayIcon)
		{
			if(!IsRunningFloatingWnd())
			{
				CmdLine.AddParam(_T("/tray"));
				floatWnd.CreateTrayIcon();
			}
		}
		else
		{
			HWND TrayWnd = FindWindow(0, _T("ImageUploader_TrayWnd"));
			if(TrayWnd)
				::SendMessage(TrayWnd, WM_CLOSETRAYWND,0, 0);
		}
	}
	else if(ShowTrayIcon)
	{
		HWND TrayWnd = FindWindow(0,_T("ImageUploader_TrayWnd"));
			if(TrayWnd)
				SendMessage(TrayWnd, WM_RELOADSETTINGS,  (floatWnd.m_hWnd)?1:0, (Settings.Hotkeys_changed)?0:1);
	}

	Settings.Hotkeys_changed  = false;
	#endif
	return true;
}

// Next code is to be deleted in next releases
void CSettings::ApplyRegSettingsRightNow() 
{
	// Applying Startup settings
	if(ShowTrayIcon)
	{
		HKEY hKey;
		CString StartupCommand = _T("\"")+CmdLine.ModuleName()+_T("\" /tray");
		LONG lRet,lRetOpen;
		lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),0,KEY_WRITE,&hKey );
		if (!lRet)
			lRetOpen = RegSetValueEx( hKey, _T("ImageUploader"), NULL,REG_SZ, (BYTE *)(LPCTSTR)StartupCommand,(StartupCommand.GetLength()+1)*sizeof(TCHAR));
      RegCloseKey( hKey );
	}
	else //deleting from Startup
	{
		HKEY hKey;
		 LONG lRet;
		lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),0,KEY_WRITE,&hKey );
		RegDeleteValue(hKey,_T("ImageUploader"));
	}

	//MessageBox(0,_T("ApplyRegSettingsRightNow()"),0,0);
	//if(SendToContextMenu_changed)
	{
		CString ShortcutName = GetSendToPath() + _T("\\Image Uploader.lnk");

		if(SendToContextMenu )
		{
			if(FileExists(ShortcutName)) DeleteFile(ShortcutName);

			CreateShortCut(ShortcutName, CmdLine.ModuleName(), GetAppFolder(), _T(" /upload"), 0, SW_SHOW, CmdLine.ModuleName(), 0) ;
		}
		else 
		{
			DeleteFile(ShortcutName);
		}
	}

	//if(ExplorerImagesContextMenu_changed || ExplorerVideoContextMenu_changed)
	{
		TCHAR szFileName[MAX_PATH+8]=_T("\"");

		GetModuleFileName(0, szFileName+1, MAX_PATH);
		lstrcat(szFileName, _T("\" \"%1\""));

		LPTSTR szList = _T("jpg\0jpeg\0png\0bmp\0gif");
		int Res;

		UnRegisterClsId();

		//if(ExplorerContextMenu_changed)
		{
			while ((*szList)!=0)
			{
				Res = AddToExplorerContextMenu(szList, (/*ExplorerImagesContextMenu?TR("Загрузить изображения"):*/0), szFileName,true);
				szList += lstrlen(szList)+1;
			}
		}

		//if( ExplorerVideoContextMenu_changed)
		{
			szList = VIDEO_FORMATS;
			while ((*szList)!=0)
			{
				Res = AddToExplorerContextMenu(szList, (/*ExplorerVideoContextMenu?TR("Открыть в Image Uploader"):*/0), szFileName,false);
				szList += lstrlen(szList)+1;
			}
		}
	}

}

ServerSettingsStruct& CSettings::ServerByName(CString name)
{
	return ServersSettings[name];
}

ServerSettingsStruct&  CSettings::ServerByUtf8Name(std::string name)
{
	return ServerByName(Utf8ToWCstring(name));
}

bool CSettings::LoadAccounts(ZSimpleXmlNode root)
{
	std::vector<ZSimpleXmlNode> servers;
	root.GetChilds("Server", servers);

	for(size_t i=0; i<servers.size(); i++)
	{
		std::string server_name = servers[i].Attribute("Name");
		std::vector<std::string> attribs;
		servers[i].GetAttributes(attribs);
		CString wideName = Utf8ToWCstring(server_name);

		for(size_t j=0; j<attribs.size(); j++)
		{
			std::string attribName = attribs[j];
		
			if(attribName.empty()) continue;
			if( attribName.substr(0,1) == "_")
			{
				std::string value = servers[i].Attribute(attribName);
				attribName = attribName.substr(1, attribName.size()-1);
				if(!value.empty())
					ServersSettings[wideName].params[attribName]=value;
			}

		}
		ServersSettings[wideName].authData.DoAuth = servers[i].AttributeBool("Auth");
		

		std::string encodedLogin = servers[i].Attribute("Login");
		CEncodedPassword login;
		login.fromEncodedData(encodedLogin.c_str());
		ServersSettings[wideName].authData.Login = WCstringToUtf8(login);
	
		std::string encodedPass = servers[i].Attribute("Password");
		CEncodedPassword pass;
		pass.fromEncodedData(encodedPass.c_str());
		ServersSettings[wideName].authData.Password = WCstringToUtf8(pass);
	}
	return true;
}

bool CSettings::SaveAccounts(ZSimpleXmlNode root)
{
	std::map <CString, ServerSettingsStruct>::iterator it;
	for(it=ServersSettings.begin(); it!=ServersSettings.end(); it++)
	{
		ZSimpleXmlNode serverNode = root.CreateChild("Server");	
		serverNode.SetAttribute("Name", WCstringToUtf8(it->first));
			
		std::map <std::string, std::string>::iterator param;
		for(param=it->second.params.begin(); param!=it->second.params.end(); param++)
		{	
				serverNode.SetAttribute("_"+param->first, param->second);
		}

		serverNode.SetAttributeBool("Auth",it->second.authData.DoAuth);
		
		CEncodedPassword login(Utf8ToWCstring(it->second.authData.Login));
		serverNode.SetAttribute("Login",WCstringToUtf8(login.toEncodedData()));

		CEncodedPassword pass(Utf8ToWCstring(it->second.authData.Password));
		serverNode.SetAttribute("Password",WCstringToUtf8(pass.toEncodedData()));
	
			
	}
	return true;
}
