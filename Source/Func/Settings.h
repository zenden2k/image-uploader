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

#ifndef IU_SETTINGS_H
#define IU_SETTINGS_H

#include <map>
#include <string>
#include "Core/SettingsManager.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Upload/ServerProfile.h"

#if !defined(IU_IMAGEEDITOR) && !defined(IU_CLI)
    #include "Func/Settings.h"
	#include "atlheaders.h"
	#include "Func/langclass.h"
	//#include "Core/Upload/UploadEngineManager.h"
	#include "Func/Common.h"
	#include "Gui/Dialogs/HotkeySettings.h"
    #include "Core/Images/ImageConverter.h"
 
	#define TRAY_SCREENSHOT_UPLOAD 0
	#define TRAY_SCREENSHOT_CLIPBOARD 1
	#define TRAY_SCREENSHOT_SHOWWIZARD 2
	#define TRAY_SCREENSHOT_ADDTOWIZARD 3
	#define TRAY_SCREENSHOT_OPENINEDITOR 4
#endif
struct UploadProfileStruct
{
	bool GenThumb;
   bool KeepAsIs;
	int ServerID, QuickServerID;
};

typedef std::map <std::string, std::map <std::string, ServerSettingsStruct>> ServerSettingsMap;

#if !defined(IU_CLI) && !defined(IU_IMAGEEDITOR)


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
	bool OpenInEditor; // only from screenshot dlg
	bool UseOldRegionScreenshotMethod;
};


struct HistorySettingsStruct
{
	bool EnableDownloading;
};

struct ImageReuploaderSettingsStruct {
	bool PasteHtmlOnCtrlV;
};
#endif

#if !defined(IU_CLI)
struct ImageEditorSettingsStruct {
	Gdiplus::Color ForegroundColor;
	Gdiplus::Color BackgroundColor;
	int PenSize;
	int RoundingRadius;
	LOGFONT Font;
    bool AllowAltTab;
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
		
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL) && !defined(IU_IMAGEEDITOR)

		// Поля данных
		bool ExplorerContextMenu;
		bool ExplorerContextMenu_changed;
		bool ExplorerVideoContextMenu;
		bool DropVideoFilesToTheList;
		bool UseDirectLinks;
		CString DataFolder;
		CString m_SettingsDir;
		CString Language;
		bool ExplorerCascadedMenu;
		int MaxThreads;
		
void FindDataFolder();
#endif

#if !defined(IU_CLI) && !defined(IU_IMAGEEDITOR)
ConnectionSettingsStruct ConnectionSettings;
#endif
#if !defined(IU_CLI)
ImageEditorSettingsStruct ImageEditorSettings;
#endif

ServerSettingsMap ServersSettings;
bool AutoShowLog;
bool UseNewIcon;
#ifndef IU_SERVERLISTTOOL


#if !defined(IU_CLI) && !defined(IU_IMAGEEDITOR)
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
		typedef fastdelegate::FastDelegate1<CSettings*> ChangeCallback;
		void addChangeCallback(const ChangeCallback& callback);
		

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
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL) && !defined(IU_IMAGEEDITOR)
		void Uninstall();
	void EnableAutostartup(bool enable);
	// Deprecated
	protected:
	int getServerID();
	int getQuickServerID();
	int getFileServerID();
protected:
    public:
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

	CString getSettingsFileName() const;
		
		
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
	void notifyChange();
	SettingsManager mgr_;
	std::string fileName_; 
	std::string SettingsFolder;
	std::vector<ChangeCallback> changeCallbacks_;
	unsigned int LastUpdateTime;
};

extern CSettings Settings;

#endif
