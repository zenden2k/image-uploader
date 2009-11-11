/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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

#include "stdafx.h"

#include "settings.h"
#include "myutils.h"
#include "Common\MyXml.h"

CSettings Settings;
#define ASSERT
#ifndef IU_SHELLEXT
BOOL IsVista()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

	if (	::GetVersionEx( &osver ) && 
		osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
		(osver.dwMajorVersion >= 6 ) )
		return TRUE;

	return FALSE;
}

#if  WINVER	< 0x0600

typedef struct _TOKEN_ELEVATION {
    DWORD TokenIsElevated;
} TOKEN_ELEVATION, *PTOKEN_ELEVATION;

#define TokenElevation 20

#endif

HRESULT 
IsElevated( __out_opt BOOL * pbElevated ) //= NULL )
{
	ASSERT( IsVista() );

	HRESULT hResult = E_FAIL; // assume an error occured
	HANDLE hToken	= NULL;

	if ( !::OpenProcessToken( 
		::GetCurrentProcess(), 
		TOKEN_QUERY, 
		&hToken ) )
	{
		ASSERT( FALSE );
		return hResult;
	}

	TOKEN_ELEVATION te = { 0 };
	DWORD dwReturnLength = 0;

	if ( !::GetTokenInformation(
		hToken,
		(TOKEN_INFORMATION_CLASS) TokenElevation,
		&te,
		sizeof( te ),
		&dwReturnLength ) )
	{
		ASSERT( FALSE );
	}
	else
	{
		ASSERT( dwReturnLength == sizeof( te ) );

		hResult = te.TokenIsElevated ? S_OK : S_FALSE; 

		if ( pbElevated)
			*pbElevated = (te.TokenIsElevated != 0);
	}

	::CloseHandle( hToken );

	return hResult;
}

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}

void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");
void EncodeString(LPCTSTR szSource,CString &Result,LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");




UploadEngine* GetEngineByName(LPCTSTR Name)
{
	for(int i=0; i<EnginesList.GetCount(); i++)
	{
		if(!lstrcmp(EnginesList[i].Name, Name))  return &EnginesList[i];
	}
	return 0;
}

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
	SHELLEXECUTEINFO TempInfo = {0};

	TCHAR buf[MAX_PATH];
	GetModuleFileName(0,buf,MAX_PATH-1);
	CString s=GetAppFolder();

	CString Command = CString(buf);
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
#endif
CSettings::CSettings()
#ifndef IU_SHELLEXT
: ServerID(ImageSettings.ServerID),QuickServerID(ImageSettings.QuickServerID)
#endif
{
	ExplorerCascadedMenu = true;
	#ifndef IU_SHELLEXT
	// Default values of settings
	*m_Directory = 0;
	UseTxtTemplate = false;
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
	UploadBufferSize = 16384;
	ImageSettings.KeepAsIs = false;
	ImageSettings.NewWidth = 0;
	ImageSettings.NewHeight = 0;
	ImageSettings.AddLogo  = false;
	ImageSettings.AddText = false;
	ImageSettings.Format = 1;
	ImageSettings.Quality = 85;
	ImageSettings.SaveProportions = true;

	LogoSettings.LogoPosition = 0;
	LogoSettings.LogoBlend = 0;
	LogoSettings.Text = APPNAME;
	LogoSettings.TextPosition = 5;
	LogoSettings.TextColor = 0xffffffff;
	
	StringToFont(_T("Tahoma,8,,204"), &LogoSettings.Font);
	StringToFont(_T("Tahoma,7,b,204"), &ThumbSettings.ThumbFont);

	ThumbSettings.CreateThumbs = true;
	ThumbSettings.ThumbWidth = 180;
	ThumbSettings.DrawFrame = true;
	ThumbSettings.ThumbAddImageSize  = true;
	ThumbSettings.FrameColor = RGB( 0, 74, 111) ;
	LogoSettings.StrokeColor = RGB( 0, 0, 0);
	ThumbSettings.ThumbColor1 =  RGB( 13, 86, 125);
	ThumbSettings.ThumbColor2 = RGB( 6, 174, 255);
	ThumbSettings.UseServerThumbs = false;
	ThumbSettings.UseThumbTemplate = false;
	ThumbSettings.ThumbTextColor = RGB( 255, 255, 255);
	ThumbSettings.ThumbAlpha = 120;
	ThumbSettings.Text = _T("%width%x%height% (%size%)");

	VideoSettings.Columns = 3;
	VideoSettings.TileWidth =  200;
	VideoSettings.GapWidth = 5;
	VideoSettings.GapHeight = 7;
	VideoSettings.NumOfFrames = 8;
	VideoSettings.JPEGQuality =  100;
	VideoSettings.UseAviInfo = TRUE;

	ConnectionSettings.UseProxy =  FALSE;
	ConnectionSettings.ProxyPort= 0;
	ConnectionSettings.NeedsAuth = false;
	ConnectionSettings.ProxyType = 0;
	
	ScreenshotSettings.Format =  1;
	ScreenshotSettings.Quality = 85;
	ScreenshotSettings.Delay = 3;
	#endif
}

#define SETTINGS_READ
#include "SettingsSaver.h" // Generating a function which reads settings

#ifndef IU_SHELLEXT
#undef SETTINGS_READ
#include "SettingsSaver.h" // Generating a function which saves settings
#endif
bool CSettings::LoadSettings(LPCTSTR szDir)
{
	CMyXml MyXML;
	CString FileName= (szDir? CString(szDir):GetAppFolder())+_T("Settings.xml");
	if(!FileExists(FileName)) return true;
	
	if(!MyXML.Load(FileName))
	{
		MessageBox(0, MyXML.GetError(),0,0);
	}
	return MacroLoadSettings(MyXML);
}
#ifndef IU_SHELLEXT
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
	TCHAR Buffer[MAX_PATH+1]=_T("CLSID\\");
	lstrcat(Buffer, MY_CLSID);
	return SHDeleteKey(HKEY_CLASSES_ROOT,Buffer)==ERROR_SUCCESS;
}

int AddToExplorerContextMenu(LPCTSTR Extension, LPCTSTR Title, LPCTSTR Command,bool DropTarget) 
{
	HKEY ExtKey = NULL;
	TCHAR Buffer[MAX_PATH];

	Buffer[0]=_T('.');
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
		wsprintf(Buffer, _T("%s\\shell\\iuploader"), ClassName);
		SHDeleteKey(HKEY_CLASSES_ROOT, Buffer);
		return 0;
	}

	wsprintf(Buffer, _T("%s\\shell\\iuploader\\command"), ClassName);

	if(!lstrlen(Buffer)) return false;
	wsprintf(Buffer, _T("%s\\shell\\iuploader"), ClassName);
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
		//MessageBox(0, _T("Could not create registry command key"), 0,0);
		//return 0;
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
		RegSetValueEx(
DropTargetKey, _T("Clsid"), 0, REG_SZ,	(LPBYTE)MY_CLSID, (lstrlen(MY_CLSID)+1)*sizeof(TCHAR));
	}

	RegCloseKey(DropTargetKey);
	return 1; // That's means ALL OK! :)
}

// Function that gets path to SendTo folder
CString GetSendToPath() 
{
	CString result;
	LPITEMIDLIST pidl;
	TCHAR        szSendtoPath [MAX_PATH];
	HANDLE       hFile;
	LPMALLOC     pMalloc;

	if(SUCCEEDED( SHGetSpecialFolderLocation ( NULL, CSIDL_SENDTO, &pidl )))
	{
		if(SHGetPathFromIDList(pidl, szSendtoPath))
		{
			result = szSendtoPath;
		}

		if(SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free ( pidl );
			pMalloc->Release();
		}
	}
	return result;
}

void write_simple_doc3()  // ????
{  
	
}  

bool CSettings::SaveSettings()
{
	// Converting server id to server name
	if(ServerID >= 0 && EnginesList.GetCount())
		Settings.ServerName = EnginesList[ServerID].Name;

	if(QuickServerID>=0 && EnginesList.GetCount())
	Settings.QuickServerName = EnginesList[QuickServerID].Name;
	if(FileServerID>=0 && EnginesList.GetCount())
		Settings.FileServerName = EnginesList[FileServerID].Name;
	
	CMyXml MyXml;

	MacroSaveSettings(MyXml);
	MyXml.Save(GetAppFolder() + _T("Settings.xml"));

	if(SendToContextMenu_changed || ExplorerContextMenu_changed) 
	{
		BOOL b;

		RegisterShellExtension(ExplorerContextMenu);

		if(IsVista() && IsElevated(&b)!=S_OK)
		ApplyRegistrySettings();
		else 
		{
			ApplyRegSettingsRightNow();

	
	}}
	ExplorerContextMenu_changed = false;
	SendToContextMenu_changed = false;

	return true;
}

void CSettings::ApplyRegSettingsRightNow()
{
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

		/*if(ExplorerContextMenu || ExplorerVideoContextMenu)
			RegisterClsId();
		else */
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
#endif