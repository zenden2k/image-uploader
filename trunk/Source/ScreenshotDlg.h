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

#pragma once

#include "resource.h"
#include "regionselect.h"
#include "hyperlinkcontrol.h"
#include "Core/ScreenCapture.h"

#define IDC_FULLSCREEN WM_USER + 219
#define IDC_VIEWSETTINGS WM_USER + 220
#define IDC_SCRACTIVEWINDOW WM_USER + 221
#define IDC_FREEFORMREGION WM_USER + 222
#define IDC_HWNDSREGION WM_USER + 223
#include <atlctrlx.h> 
#include "3rdpart/wtlaero.h"
// CScreenshotDlg
class CScreenshotDlg : 
	public /*aero::*/CDialogImpl<CScreenshotDlg>	, 
	public CWinDataExchange<CScreenshotDlg>
{
	public:
		CScreenshotDlg();
		~CScreenshotDlg();
		CaptureMode captureMode() const;
		enum { IDD = IDD_SCREENSHOTDLG };
	
	protected:
		BEGIN_MSG_MAP(CScreenshotDlg)
//			CHAIN_MSG_MAP(aero::CDialogImpl<CScreenshotDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MSG_WM_CTLCOLORDLG(OnCtlColorMsgDlg)
			MSG_WM_CTLCOLORBTN(OnCtlColorMsgDlg)
			MSG_WM_CTLCOLORSTATIC(OnCtlColorMsgDlg)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_ID_HANDLER(IDC_VIEWSETTINGS,  OnSettingsClick)
			COMMAND_HANDLER(IDC_FULLSCREEN, BN_CLICKED, OnClickedFullscreenCapture)
			COMMAND_ID_HANDLER(IDC_SCRACTIVEWINDOW,  OnClickedActiveWindowCapture)
			COMMAND_HANDLER(IDC_REGIONSELECT, BN_CLICKED, OnBnClickedRegionselect)
			COMMAND_HANDLER(IDC_FREEFORMREGION, BN_CLICKED, OnBnClickedFreeFormRegion)
			COMMAND_HANDLER(IDC_HWNDSREGION, BN_CLICKED, OnBnClickedWindowHandlesRegion)
		END_MSG_MAP()
	
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		
		// Handlers:
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnSettingsClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnEnter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnCtlColorMsgDlg(HDC hdc, HWND hwndChild);

		LRESULT OnBnClickedFreeFormRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBnClickedWindowHandlesRegion(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedFullscreenCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedActiveWindowCapture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBnClickedRegionselect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
		CBrush m_WhiteBr;
		//bool m_bExpanded;
		//int nFullWindowHeight;
		CHyperLinkControl CommandBox;
		CaptureMode m_CaptureMode;
		void ExpandDialog() const;
};