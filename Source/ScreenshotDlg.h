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
//#include "maindlg.h"
#include "resource.h"
#include "regionselect.h"
//#include "wizarddlg.h"
#include "hyperlinkcontrol.h"
#define IDC_VIEWSETTINGS WM_USER + 220
#define IDC_SCRACTIVEWINDOW WM_USER + 221

// CScreenshotDlg
class CScreenshotDlg : 
	public CDialogImpl<CScreenshotDlg>	, 
	public CRegionSelectCallback,
	public CWinDataExchange<CScreenshotDlg>
{
	public:
		CRegionSelectCallback *m_pCallBack;
		TCHAR FileName[256];
		CWindow *MainDlg;
		HBRUSH WhiteBr;
		bool m_bExpanded;
		bool m_bEntireScreen;
		int m_Action;
		//CWizardDlg *WizardDlg;
		int nFullWindowHeight;
		CScreenshotDlg();
		~CScreenshotDlg();
		enum { IDD = IDD_SCREENSHOTDLG };

		BEGIN_MSG_MAP(CScreenshotDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MSG_WM_CTLCOLORDLG(OnCtlColorMsgDlg)
			MSG_WM_CTLCOLORBTN(OnCtlColorMsgDlg)
			MSG_WM_CTLCOLORSTATIC(OnCtlColorMsgDlg)
			COMMAND_HANDLER(IDC_SCREENSHOT, BN_CLICKED, OnClickedOK)
			
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnEnter)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_ID_HANDLER(IDC_VIEWSETTINGS,  OnSettingsClick)
			COMMAND_ID_HANDLER(IDC_SCRACTIVEWINDOW,  OnClickedOK)
			COMMAND_HANDLER(IDC_REGIONSELECT, BN_CLICKED, OnBnClickedRegionselect)
		
		END_MSG_MAP()
		
	BEGIN_DDX_MAP(CScreenshotDlg)
		DDX_CONTROL(IDC_COMMANDBOX/*IDC_LINKSCONTROL*/,CommandBox)
    END_DDX_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		
		// Handlers:
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSettingsClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
			
		LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);
		int ScreenshotError();
		CHyperLinkControl CommandBox;
		void OnScreenshotFinished(int Result);
		void OnScreenshotSaving(LPTSTR szFileName, Bitmap* Bm);
		void ExpandDialog();
		void SaveSettings();
		void Execute(HWND Parent, CRegionSelectCallback *RegionSelectCallback, bool FullScreen = true);
};