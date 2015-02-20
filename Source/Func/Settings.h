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

#include <map>
#include <string>
#include "Core/SettingsManager.h"
#include <Core/Upload/UploadEngine.h>
#ifndef IU_CLI
    #include "Func/Settings.h"
	#include "atlheaders.h"
	#include "Func/langclass.h"
	#include "Func/pluginloader.h"
	#include "Func/Common.h"
	#include "Gui/Dialogs/HotkeySettings.h"
    #include "Core/ImageConverter.h"

	#define TRAY_SCREENSHOT_UPLOAD 0
	#define TRAY_SCREENSHOT_CLIPBOARD 1
	#define TRAY_SCREENSHOT_SHOWWIZARD 2
	#define TRAY_SCREENSHOT_ADDTOWIZARD 3
#endif
struct UploadProfileStruct
{
	bool GenThumb;
   bool KeepAsIs;
	int ServerID, QuickServerID;
};

#ifndef IU_CLI


struct FullUploadProfile
{
   UploadProfileStruct upload_profile;
   ImageConvertingParams convert_profile;
};

struct TrayIconSettingsStruct
{
	int LeftDoubleClickCommand;
	int LeftClickCommand;
	int RightClickCommand;
	int MiddleClickCommand;
	int TrayScreenshotAction;
	BOOL DontLaunchCopy;
	BOOL ShortenLinks;
};
struct ThumbSettingsStruct: public ThumbCreatingParams
{
	TCHAR FontName[256];
	BOOL UseServerThumbs;
	bool CreateThumbs;
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
	CString Engine;
	CString SnapshotsFolder;
	CString SnapshotFileTemplate;
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

struct ImageReuploaderSettingsStruct {
	bool PasteHtmlOnCtrlV;
};
#endif

class CSettings
{
	public:

		int FileRetryLimit;
		int ActionRetryLimit;
				UploadProfileStruct UploadProfile;
		
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)

		// ���� ������
		bool ExplorerContextMenu;
		bool ExplorerContextMenu_changed;
		bool ExplorerVideoContextMenu;
		bool UseDirectLinks;
		CString DataFolder;
		CString m_SettingsDir;
		CString Language;
		bool ExplorerCascadedMenu;
		
void FindDataFolder();
#endif

#ifndef IU_CLI
ConnectionSettingsStruct ConnectionSettings;
#endif

std::map <std::string, ServerSettingsStruct> ServersSettings;
bool AutoShowLog;
#ifndef IU_SERVERLISTTOOL
#if !defined(IU_CLI)
		CHotkeyList Hotkeys;
		bool Hotkeys_changed;
		bool ShowTrayIcon;
		bool ShowTrayIcon_changed;
		bool AutoStartup;
		bool AutoStartup_changed;
		int ThumbsPerLine;
		TCHAR m_szLang[64];
		TrayIconSettingsStruct TrayIconSettings;
		ThumbSettingsStruct ThumbSettings;
		VideoSettingsStruct VideoSettings;

		ScreenshotSettingsStruct ScreenshotSettings;
		HistorySettingsStruct HistorySettings;
		ImageReuploaderSettingsStruct ImageReuploaderSettings;
		bool ConfirmOnExit;
		bool ShowUploadErrorDialog;

		bool UseTxtTemplate;
		
		bool AutoCopyToClipboard;
		bool WatchClipboard;
		int CodeLang;
		int CodeType;
		bool ParseSubDirs;
		bool UseProxyServer;
		

		static COLORREF DefaultLinkColor;
		bool SendToContextMenu;
		bool SendToContextMenu_changed;
		bool QuickUpload;

		CString ImageEditorPath;
		CString VideoFolder,ImagesFolder;

		std::map<CString, ImageConvertingParams> ConvertProfiles;
		CString CurrentConvertProfileName;
		CString getShellExtensionFileName() const;
#endif
		
#endif 

#if !defined(IU_CLI)
	private:
		TCHAR m_Directory[MAX_PATH];
#endif
		
	public:
		CSettings();
		~CSettings();

		
		bool LoadSettings(std::string szDir="", std::string fileName = "", bool LoadFromRegistry  = true);
		bool SaveSettings();
		bool LoadAccounts(ZSimpleXmlNode root);
		bool SaveAccounts(ZSimpleXmlNode root);
	int UploadBufferSize;
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
		void Uninstall();
	void EnableAutostartup(bool enable);
	int ServerID, QuickServerID;
	void ApplyRegSettingsRightNow();
	
   bool LoadConvertProfiles(ZSimpleXmlNode root);
   bool LoadConvertProfile(const CString& name, ZSimpleXmlNode profileNode);
	bool SaveConvertProfiles(ZSimpleXmlNode root);
   void BindConvertProfile(SettingsNode& mgr,  ImageConvertingParams &params);
#ifndef IU_SHELLEXT
   ServerSettingsStruct& ServerByName(CString name);
   
#endif
   
	int FileServerID;
	CString ServerName, QuickServerName,FileServerName, UrlShorteningServer;
	
	
	static const TCHAR VideoEngineDirectshow[];
	static const TCHAR VideoEngineFFmpeg[];
	static const TCHAR VideoEngineAuto[];
	static bool IsFFmpegAvailable();
	
	CString prepareVideoDialogFilters();
#endif
	ServerSettingsStruct& ServerByUtf8Name(std::string name);
	SettingsManager mgr_;
	std::string fileName_; 
	std::string SettingsFolder;
	unsigned int LastUpdateTime;
};

extern CSettings Settings;

#endif