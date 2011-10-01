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

#pragma once

#include <atlcoll.h>
#include "langclass.h"
#include "common/myxml.h"
#include <map>

struct ImageSettingsStruct
{
	int NewWidth,NewHeight;
	BOOL KeepAsIs;
	BOOL AddLogo;
	BOOL AddText;
	BOOL GenThumb;
	
	int Format;
	int Quality;
	BOOL SaveProportions;
	int ServerID, QuickServerID;
};

struct LogoSettingsStruct
{
	LOGFONT Font;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;
	CString FileName;
	TCHAR FontName[256];
	CString Text;
	COLORREF TextColor,StrokeColor;
};

struct ThumbSettingsStruct
{
	
	LOGFONT ThumbFont;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;
	//int TextColor;
	CString Text;
	TCHAR FileName[256];
	TCHAR FontName[256];
	COLORREF FrameColor,ThumbColor1,ThumbColor2/*TextBackground ,*/,ThumbTextColor;
	int ThumbAlpha;
	BOOL TextOverThumb;
		int ThumbWidth;
BOOL UseServerThumbs;
	BOOL UseThumbTemplate;
	BOOL DrawFrame;
	BOOL ThumbAddImageSize;
	BOOL ThumbAddBorder;	BOOL CreateThumbs;

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
};

struct ConnectionSettingsStruct
{
	BOOL UseProxy;
	CString ServerAddress;
	int ProxyPort;
	BOOL NeedsAuth;
	CString ProxyUser;
	CString ProxyPassword;
	int ProxyType;
};

struct ScreenshotSettingsStruct
{
	//BOOL EntireScr;
	int Format;
	int Quality, Delay;
};


class CSettings
{
	public:
		// ���� ������
		int ThumbsPerLine;
		TCHAR m_szLang[64];
		ImageSettingsStruct ImageSettings;
		LogoSettingsStruct LogoSettings;
		ThumbSettingsStruct ThumbSettings;
		VideoSettingsStruct VideoSettings;
		ConnectionSettingsStruct ConnectionSettings;
		ScreenshotSettingsStruct ScreenshotSettings;
		bool ConfirmOnExit;
		bool ShowUploadErrorDialog;
		int FileRetryLimit;
		int ActionRetryLimit;
		bool UseTxtTemplate;
		bool AutoShowLog;
		bool AutoCopyToClipboard;
		int CodeLang;
		int CodeType;
		//BOOL OldStyleMenu;
		bool ParseSubDirs;
		bool UseProxyServer;
		CString Language;
		bool ExplorerImagesContextMenu;
		bool ExplorerImagesContextMenu_changed;
		bool ExplorerVideoContextMenu;
		bool ExplorerVideoContextMenu_changed;
		bool SendToContextMenu;
		bool SendToContextMenu_changed;
		bool QuickUpload;
		std::map<CString,LoginInfo> AuthParams ;
		CString ImageEditorPath;
		CString VideoFolder,ImagesFolder;
	private:
		TCHAR m_Directory[MAX_PATH];
		
	public:
		CSettings();

		bool LoadSettings();
		bool MacroLoadSettings(CMyXml &XML);
		bool MacroSaveSettings(CMyXml &XML);
int UploadBufferSize;
		bool SaveSettings();
int ServerID,
			QuickServerID;
void ApplyRegSettingsRightNow();
int FileServerID;
CString ServerName, QuickServerName,FileServerName;
	/**	void PutString(SettingIndex Index, LPCTSTR Name, LPCTSTR Value); 

		LPTSTR GetString(LPCTSTR Name);
		bool SetDirectory(LPCTSTR Directory);
		bool LoadLanguage(LPTSTR Lang);
		bool LoadList();*/

	//	CAtlArray<SettingsListItem> SettingsList;
		

	
};
extern CSettings Settings;