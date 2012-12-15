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
#include "Core/ImageConverter.h"
#include "Core/SettingsManager.h"
#include "atlheaders.h"
#include "Func/langclass.h"
#include "Func/pluginloader.h"
#include "Gui/Dialogs/HotkeySettings.h"



//New 
struct ImageUploadParams {
	ImageUploadParams() {
		UseServerThumbs = false;
		CreateThumbs = true;
		ProcessImages = false;
		UseDefaultThumbSettings = true;
		UseDefaultSettings = true;
		Size = 150;
		ThumbResizeMode = ThumbCreatingParams::trByWidth;
	}
	void bind(SettingsNode& n);
	
	bool UseDefaultSettings;
	bool UseServerThumbs;
	bool CreateThumbs;
	bool ProcessImages;
	CString ImageProfileName;
	bool UseDefaultThumbSettings;
	unsigned int Size;
	ThumbCreatingParams::ThumbResizeEnum ThumbResizeMode;

};

class ServerProfile {

	public:
		ImageUploadParams imageUploadParams;
		ServerProfile();

		ServerProfile(CString newServerName);
		ServerSettingsStruct serverSettings();
		CUploadEngineData* uploadEngineData() const;

		void setProfileName(CString newProfileName);
		CString profileName() const;

		void setServerName(CString newProfileName);
		CString serverName() const;
		

	protected:
		CString serverName_;
		CString profileName_;
		
		friend class CSettings;
};

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
	bool UseAviInfo;
	bool ShowMediaInfo;
	LOGFONT Font;
	COLORREF TextColor;
	CString Engine;
};

class CEncodedPassword
{
	public:
		CEncodedPassword();
		CEncodedPassword(CString d);
		CString toEncodedData() const;
		void fromPlainText(CString data);
		void fromEncodedData(CString data);
		operator CString&();
		operator const TCHAR*();
		CEncodedPassword& operator=(const CString& text);
	private:
		CString data_;
};
struct ConnectionSettingsStruct
{
	bool UseProxy;
	CString ServerAddress;
	int ProxyPort;
	bool NeedsAuth;
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
		TrayIconSettingsStruct TrayIconSettings;
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
		bool ParseSubDirs;
		bool UseProxyServer;
		int LastUpdateTime;
    UploadProfileStruct UploadProfile;
		
		bool SendToContextMenu;
		bool SendToContextMenu_changed;
		bool QuickUpload;
		std::map <CString, std::map<CString,ServerSettingsStruct> > ServersSettings;
		CString ImageEditorPath;
		CString VideoFolder,ImagesFolder;
		bool SaveSettings();
      std::map<CString, ImageConvertingParams> ConvertProfiles;
      CString CurrentConvertProfileName;
		CString getShellExtensionFileName() const;
		enum { DefaultUploadBufferSize = 65536 };
		enum TrayScreenshotAction { TRAY_SCREENSHOT_UPLOAD = 0, TRAY_SCREENSHOT_CLIPBOARD, TRAY_SCREENSHOT_SHOWWIZARD, TRAY_SCREENSHOT_ADDTOWIZARD };
		static COLORREF DefaultLinkColor;
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
	int ServerID();//deprecated
	int QuickServerID();//deprecated
	ServerProfile imageServer, fileServer, quickServer;
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
	int FileServerID();
	CString ServerName();
	CString QuickServerName();
	CString FileServerName();
	SettingsManager mgr_;
	CString SettingsFolder;
	static const TCHAR VideoEngineDirectshow[];
	static const TCHAR VideoEngineFFmpeg[];
	static const TCHAR VideoEngineAuto[];
	static bool IsFFmpegAvailable();

	CString prepareVideoDialogFilters();
	static bool IsUrlHandlerRegistered();
	static bool RegisterURLHandler();
	static CString URLHandlerName();
};

extern CSettings Settings;

#endif