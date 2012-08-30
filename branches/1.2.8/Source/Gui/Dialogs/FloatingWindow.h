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

#ifndef FLOATINGWINDOW_H
#define FLOATINGWINDOW_H

// This file was generated by WTL subclass control wizard 
// FloatingWindow.h : Declaration of the FloatingWindow

#pragma once
#include "atlheaders.h"
#include <atlcrack.h>
#include "Common/trayicon.h"
#include "TraySettings.h"
#include "Core/Upload/FileQueueUploader.h"
#include <Gui/Dialogs/HotkeySettings.h>
// FloatingWindow

#define IDM_UPLOADFILES 20001
#define IDM_IMPORTVIDEO IDM_UPLOADFILES+1
#define IDM_SCREENSHOT IDM_UPLOADFILES+2
#define IDM_SCREENSHOTDLG IDM_UPLOADFILES+3
#define IDM_REGIONSCREENSHOT IDM_UPLOADFILES+4
#define IDM_FULLSCREENSHOT IDM_UPLOADFILES+5
#define IDM_WINDOWSCREENSHOT IDM_UPLOADFILES+6
#define IDM_WINDOWHANDLESCREENSHOT IDM_UPLOADFILES+7
#define IDM_FREEFORMSCREENSHOT IDM_UPLOADFILES+8
#define IDM_ADDFOLDERS IDM_UPLOADFILES+9
#define IDM_SHOWAPPWINDOW IDM_UPLOADFILES+10
#define IDM_SETTINGS IDM_UPLOADFILES+11
#define IDM_EXIT IDM_UPLOADFILES+12
#define IDM_CONTEXTMENU IDM_UPLOADFILES+13
#define IDM_PASTEFROMCLIPBOARD (IDM_UPLOADFILES+14)
#define IDM_MEDIAINFO (IDM_UPLOADFILES+15)
#define IDM_UPLOADIMAGES (IDM_UPLOADFILES+16)
#define IDM_SCREENTSHOTACTION_UPLOAD (IDM_UPLOADFILES+17)
#define IDM_SCREENTSHOTACTION_TOCLIPBOARD (IDM_UPLOADFILES+18)
#define IDM_SCREENTSHOTACTION_SHOWWIZARD (IDM_UPLOADFILES+19)
#define IDM_SCREENTSHOTACTION_ADDTOWIZARD (IDM_UPLOADFILES+20)
#define IDM_PASTEFROMWEB (IDM_UPLOADFILES+21)
#define IDM_STOPUPLOAD (IDM_UPLOADFILES+22)

#define WM_CLOSETRAYWND (WM_USER+2)
#define WM_RELOADSETTINGS (WM_USER+3)



class CFloatingWindow :
	public CWindowImpl<CFloatingWindow>, 
	public CTrayIconImpl<CFloatingWindow>, 
	public CFileQueueUploader::Callback
{
public:
	HANDLE hMutex;
	HWND m_ActiveWindow;
		HMENU m_hTrayIconMenu;
		UINT WM_TASKBARCREATED;
		bool EnableClicks;
		HWND m_PrevActiveWindow;
		CHotkeyList m_hotkeys;
		HICON m_hIconSmall;
		bool m_bStopCapturingWindows;
		bool m_bIsUploading;
		CFileQueueUploader::FileListItem m_LastUploadedItem;
	CFloatingWindow();
	~CFloatingWindow();
	DECLARE_WND_CLASS(_T("CFloatingWindow"))
	
    BEGIN_MSG_MAP(CFloatingWindow)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_CREATE(OnCreate)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
		COMMAND_ID_HANDLER_EX(IDM_SETTINGS, OnMenuSettings)
		COMMAND_ID_HANDLER_EX(IDM_IMPORTVIDEO, OnImportvideo)
		COMMAND_ID_HANDLER_EX(IDM_UPLOADFILES, OnUploadFiles)
		COMMAND_ID_HANDLER_EX(IDM_UPLOADIMAGES, OnUploadImages)
		COMMAND_ID_HANDLER_EX(IDM_SCREENSHOTDLG, OnScreenshotDlg)
		COMMAND_ID_HANDLER_EX(IDM_REGIONSCREENSHOT, OnRegionScreenshot)
		COMMAND_ID_HANDLER_EX(IDM_FULLSCREENSHOT, OnFullScreenshot)
		COMMAND_ID_HANDLER_EX(IDM_WINDOWHANDLESCREENSHOT, OnWindowHandleScreenshot)
		COMMAND_ID_HANDLER_EX(IDM_FREEFORMSCREENSHOT, OnFreeformScreenshot)
		COMMAND_ID_HANDLER_EX(IDM_WINDOWSCREENSHOT, OnWindowScreenshot)
		COMMAND_ID_HANDLER_EX(IDM_ADDFOLDERS, OnAddFolder)
		COMMAND_ID_HANDLER_EX(IDM_SHOWAPPWINDOW, OnShowAppWindow)
		COMMAND_ID_HANDLER_EX(IDM_PASTEFROMWEB, OnPasteFromWeb)
		COMMAND_ID_HANDLER_EX(IDM_CONTEXTMENU, OnContextMenu)
		COMMAND_ID_HANDLER_EX(IDM_PASTEFROMCLIPBOARD, OnPaste)
		COMMAND_ID_HANDLER_EX(IDM_STOPUPLOAD, OnStopUpload)
		COMMAND_ID_HANDLER_EX(IDM_MEDIAINFO, OnMediaInfo)
		COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_UPLOAD, OnScreenshotActionChanged)
		COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_TOCLIPBOARD, OnScreenshotActionChanged)
		COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_SHOWWIZARD, OnScreenshotActionChanged)
		COMMAND_ID_HANDLER_EX(IDM_SCREENTSHOTACTION_ADDTOWIZARD, OnScreenshotActionChanged)
		MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
		MESSAGE_HANDLER_EX(WM_CLOSETRAYWND, OnCloseTray)
		MESSAGE_HANDLER_EX(WM_RELOADSETTINGS, OnReloadSettings)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_HOTKEY(OnHotKey)
		MESSAGE_HANDLER_EX(WM_TASKBARCREATED, OnTaskbarCreated)

		//CHAIN_MSG_MAP(CTrayIconImpl<CFloatingWindow>)
    END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	 LRESULT OnClose(void);
	 LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	 LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	 LRESULT OnMenuSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnCloseTray(UINT uMsg, WPARAM wParam, LPARAM lParam);
	 LRESULT OnReloadSettings(UINT uMsg, WPARAM wParam, LPARAM lParam);
	 LRESULT OnImportvideo(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnUploadFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnUploadImages(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnScreenshotDlg(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnRegionScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnFullScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnWindowScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnFreeformScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnWindowHandleScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnAddFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnShowAppWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnContextMenu(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnMediaInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnScreenshotActionChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnStopUpload(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnTimer(UINT id);
	 void CreateTrayIcon();
	 void RegisterHotkeys();
	 void UnRegisterHotkeys();
	 LRESULT OnHotKey(int HotKeyID, UINT flags, UINT vk);
	 LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	  LRESULT OnPasteFromWeb(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	 LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam);
	 void ShowBaloonTip(const CString& text, const CString& title);
	 void UploadScreenshot(const CString& realName, const CString &displayName);
	 CString fileName, realFileName;
	 CFileQueueUploader * m_FileQueueUploader;
	 bool OnQueueFinished(CFileQueueUploader*);
	 bool m_bFromHotkey;
	 bool OnFileFinished(bool ok, CFileQueueUploader::FileListItem& result);
	 bool OnConfigureNetworkManager(CFileQueueUploader*,NetworkManager* nm);
	 std::string source_file_name_;
	 std::string server_name_;
};
extern CFloatingWindow floatWnd;
void CreateFloatWindow();
BOOL IsRunningFloatingWnd();

#endif // FLOATINGWINDOW_H

