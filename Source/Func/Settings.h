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

typedef std::map <std::string, std::map <std::string, ServerSettingsStruct>> ServerSettingsMap;

#ifndef IU_CLI





struct ImageUploadParams {
	ImageUploadParams() {
		UseServerThumbs = false;
		CreateThumbs = false;
		ProcessImages = false;
		UseDefaultThumbSettings = true;
		ImageProfileName = "Default";
		Thumb.Size = 180;
		Thumb.ResizeMode = ThumbCreatingParams::trByWidth;
		Thumb.AddImageSize = true;
		Thumb.Format = ThumbCreatingParams::tfPNG;
		Thumb.TemplateName = "default";
		Thumb.BackgroundColor = RGB( 255, 255, 255);
		Thumb.Quality = 85;
		Thumb.Text = _T("%width%x%height% (%size%)");
	}
	void bind(SettingsNode& n);


	bool UseServerThumbs;
	bool CreateThumbs;
	bool ProcessImages;
	bool ThumbAddImageSize;

	CString ImageProfileName;

	bool UseDefaultThumbSettings;
	ThumbCreatingParams getThumb(); 
	ThumbCreatingParams& getThumbRef(); 
	void setThumb(ThumbCreatingParams tcp); 
protected:
	ThumbCreatingParams Thumb;
};
class ServerProfile {

public:
	
	ServerProfile();

	ServerProfile(CString newServerName);
	ServerSettingsStruct& serverSettings();
	CUploadEngineData* uploadEngineData() const;

	void setProfileName(CString newProfileName);
	CString profileName() const;

	void setServerName(CString newProfileName);
	CString serverName() const;

	std::string folderTitle() const;
	void setFolderTitle(std::string newTitle);
	std::string folderId() const;
	void setFolderId(std::string newId);
	std::string folderUrl() const;
	void setFolderUrl(std::string newUrl);
	bool isNull();
	bool UseDefaultSettings;
	void clearFolderInfo();

	void bind(SettingsNode& n);

	ImageUploadParams getImageUploadParams();
	ImageUploadParams& getImageUploadParamsRef();
    void setImageUploadParams(ImageUploadParams iup);
	friend struct ImageUploadParams;

protected:
	CString serverName_;
	CString profileName_;
	ImageUploadParams imageUploadParams;
	std::string folderTitle_;
	std::string folderId_;
	std::string folderUrl_;

	friend class CSettings;
};

typedef std::map<CString, ServerProfile> ServerProfilesMap;
struct FullUploadProfile
{
	ServerProfile upload_profile;
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
protected:
		UploadProfileStruct UploadProfile;
public:
		
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)

		// Поля данных
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

ServerSettingsMap ServersSettings;
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
		bool RememberImageServer;
		bool RememberFileServer;
protected:
		ThumbSettingsStruct ThumbSettings;
public:
		VideoSettingsStruct VideoSettings;
		ServerProfile imageServer, fileServer, quickScreenshotServer,contextMenuServer,urlShorteningServer;

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

		bool IsPortable;

		CString ImageEditorPath;
		CString VideoFolder,ImagesFolder;

		std::map<CString, ImageConvertingParams> ConvertProfiles;
		ServerProfilesMap ServerProfiles;
protected:
		CString CurrentConvertProfileName;
public:
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
		bool LoadAccounts(SimpleXmlNode root);
		bool SaveAccounts(SimpleXmlNode root);

		bool LoadServerProfiles(SimpleXmlNode root);
		bool SaveServerProfiles(SimpleXmlNode root);
	int UploadBufferSize;
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
		void Uninstall();
	void EnableAutostartup(bool enable);
	// Deprecated
	protected:
	int getServerID();
	int getQuickServerID();
	int getFileServerID();
public:
	void setServerID(int id);
	void setQuickServerID(int id);
	void setFileServerID(int id);

	void ApplyRegSettingsRightNow();
	
   bool LoadConvertProfiles(SimpleXmlNode root);
   bool LoadConvertProfile(const CString& name, SimpleXmlNode profileNode);
	bool SaveConvertProfiles(SimpleXmlNode root);
   void BindConvertProfile(SettingsNode& mgr,  ImageConvertingParams &params);
#ifndef IU_SHELLEXT
   protected:
   ServerSettingsStruct& ServerByName(CString name);
   public:
   
#endif
   

	CString getServerName();
	CString getQuickServerName();
	CString getFileServerName();
		
		
	//CString	UrlShorteningServer;

	protected:

		CString ServerName;
		CString QuickServerName;
		CString FileServerName;
	public:
	static const TCHAR VideoEngineDirectshow[];
	static const TCHAR VideoEngineFFmpeg[];
	static const TCHAR VideoEngineAuto[];
	static bool IsFFmpegAvailable();
	
	CString prepareVideoDialogFilters();
#endif
	protected:
	ServerSettingsStruct& ServerByUtf8Name(std::string name);
	public:
	SettingsManager mgr_;
	std::string fileName_; 
	std::string SettingsFolder;
	unsigned int LastUpdateTime;
};

extern CSettings Settings;

#endif
