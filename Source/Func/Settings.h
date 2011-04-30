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

#ifndef IU_SETTINGS_H
#define IU_SETTINGS_H

#pragma once

struct UploadProfileStruct;

#include <atlcoll.h>
#include "langclass.h"
#include <map>
#include <string>
#include "../Core/ImageConverter.h"
#include "pluginloader.h"
#include "../Core/SettingsManager.h"
#include "Common.h"
#include "../Gui/Dialogs/HotkeySettings.h"
#define TRAY_SCREENSHOT_UPLOAD 0
#define TRAY_SCREENSHOT_CLIPBOARD 1
#define TRAY_SCREENSHOT_SHOWWIZARD 2
#define TRAY_SCREENSHOT_ADDTOWIZARD 3


/*struct ImageSettingsStruct: public ImageConvertingParams
{
	BOOL GenThumb;
	int ServerID, QuickServerID;
};*/

struct UploadProfileStruct
{
	BOOL GenThumb;
   bool KeepAsIs;
	int ServerID, QuickServerID;
};

struct FullUploadProfile
{
   UploadProfileStruct upload_profile;
   ImageConvertingParams convert_profile;
};
/*struct LogoSettingsStruct
{
	LOGFONT Font;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;
	CString FileName;
	TCHAR FontName[256];
	CString Text;
	COLORREF TextColor,StrokeColor;
};*/



struct TrayIconSettingsStruct
{
	int LeftDoubleClickCommand;
	int LeftClickCommand;
	int RightClickCommand;
	int MiddleClickCommand;
	int TrayScreenshotAction;
	BOOL DontLaunchCopy;
};
struct ThumbSettingsStruct: public ThumbCreatingParams
{
	/*LOGFONT ThumbFont;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;*/
	//int TextColor;
	//CString Text;
	//TCHAR FileName[256];
	TCHAR FontName[256];
	//COLORREF FrameColor,ThumbColor1,ThumbColor2/*TextBackground ,*/,ThumbTextColor;
	/*int ThumbAlpha;
	BOOL TextOverThumb;
	int ThumbWidth;*/
	BOOL UseServerThumbs;
	//BOOL UseThumbTemplate;
	/*BOOL DrawFrame;
	BOOL ThumbAddImageSize;
	BOOL ThumbAddBorder;*/	
	
	bool CreateThumbs;
	//CString thumbFileName;

};

struct VideoSettingsStruct
{
	int Columns;
	int TileWidth;
	int GapWidth;
	int GapHeight;
	int NumOfFrames;
	int JPEGQuality;
	BOOL UseAviInfo;
	BOOL ShowMediaInfo;
	LOGFONT Font;
	COLORREF TextColor;
};

class CEncodedPassword
{
	public:
		CEncodedPassword()
		{
		}
		CEncodedPassword(CString d) 
		{ 
			 data_ = d; 
		}
		CString toEncodedData() const
		{
			CString res;
			EncodeString(data_, res);
			return res;
		}
		void fromPlainText(CString data)
		{
			data = data_;
		}
		void fromEncodedData(CString data)
		{
			DecodeString(data, data_);
		}
		operator CString&()
		{
			return data_;
		}
		operator const TCHAR*()
		{
			return data_;
		}
		CEncodedPassword& operator=(const CString& text)
		{
			data_ = text;
			return *this;
		}
	private:
		CString data_;
};
struct ConnectionSettingsStruct
{
	BOOL UseProxy;
	CString ServerAddress;
	int ProxyPort;
	BOOL NeedsAuth;
	CString ProxyUser;
	CEncodedPassword ProxyPassword;
	int ProxyType;
};

struct ScreenshotSettingsStruct
{
	int Format;
	int Quality, Delay;
	int WindowHidingDelay;
	bool ShowForeground;
	bool CopyToClipboard;
	COLORREF brushColor;
	CString FilenameTemplate;
	CString Folder;
	bool RemoveCorners;
	bool AddShadow;
	bool RemoveBackground;
};

struct HistorySettingsStruct
{
	bool EnableDownloading;
};

class CSettings
{
	public:
		bool ExplorerContextMenu;
		bool ExplorerContextMenu_changed;
		bool ExplorerVideoContextMenu;
		bool UseDirectLinks;
		// Поля данных
		CString DataFolder;
		CString m_SettingsDir;
		CString Language;
		bool ExplorerCascadedMenu;
		#ifndef IU_SHELLEXT
#ifndef IU_SERVERLISTTOOL
		CHotkeyList Hotkeys;
#endif
		bool Hotkeys_changed;
		bool ShowTrayIcon;
		bool ShowTrayIcon_changed;
		bool AutoStartup;
		bool AutoStartup_changed;
		int ThumbsPerLine;
		TCHAR m_szLang[64];
		//ImageSettingsStruct ImageSettings;
		TrayIconSettingsStruct TrayIconSettings;
//		LogoSettingsStruct LogoSettings;
		ThumbSettingsStruct ThumbSettings;
		VideoSettingsStruct VideoSettings;
		ConnectionSettingsStruct ConnectionSettings;
		ScreenshotSettingsStruct ScreenshotSettings;
		HistorySettingsStruct HistorySettings;
		bool ConfirmOnExit;
		bool ShowUploadErrorDialog;
		int FileRetryLimit;
		int ActionRetryLimit;
		bool UseTxtTemplate;
		bool AutoShowLog;
		bool AutoCopyToClipboard;
		bool WatchClipboard;
		int CodeLang;
		int CodeType;
		//BOOL OldStyleMenu;
		bool ParseSubDirs;
		bool UseProxyServer;
		int LastUpdateTime;
    UploadProfileStruct UploadProfile;
		
		bool SendToContextMenu;
		bool SendToContextMenu_changed;
		bool QuickUpload;
		std::map <CString, ServerSettingsStruct> ServersSettings;
		CString ImageEditorPath;
		CString VideoFolder,ImagesFolder;
		bool SaveSettings();
      std::map<CString, ImageConvertingParams> ConvertProfiles;
      CString CurrentConvertProfileName;
		
#endif IU_SHELLEXT
	private:
		TCHAR m_Directory[MAX_PATH];
		
	public:
		CSettings();
		~CSettings();
		void Uninstall();
		void FindDataFolder();
		bool LoadSettings(LPCTSTR szDir=NULL, bool LoadFromRegistry  = true);
	int UploadBufferSize;
	void EnableAutostartup(bool enable);
	int ServerID, QuickServerID;
	void ApplyRegSettingsRightNow();
	bool LoadAccounts(ZSimpleXmlNode root);
	bool SaveAccounts(ZSimpleXmlNode root);
   bool LoadConvertProfiles(ZSimpleXmlNode root);
   bool LoadConvertProfile(const CString& name, ZSimpleXmlNode profileNode);
	bool SaveConvertProfiles(ZSimpleXmlNode root);
   void BindConvertProfile(SettingsNode& mgr,  ImageConvertingParams &params);
	#ifndef IU_SHELLEXT
	ServerSettingsStruct& ServerByName(CString name);
	ServerSettingsStruct& ServerByUtf8Name(std::string name);
#endif
	int FileServerID;
	CString ServerName, QuickServerName,FileServerName;
	SettingsManager mgr_;
	CString SettingsFolder;
};

extern CSettings Settings;

#endif