/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#pragma once
#include "atlheaders.h"
#include "welcomedlg.h"
#include "maindlg.h"
#include "VideoGrabberPage.h"
#include "uploadsettings.h"
#include "aboutdlg.h"
#include "resource.h"       // main symbols
#include <atlcrack.h>
#include "logosettings.h"
#include "settingspage.h"
#include "Gui/Controls/tablistbox.h"
// CSettingsDlg

const int SettingsPageCount = 10;

class CSettingsDlg : public CDialogImpl<CSettingsDlg>
{
	public:
		CSettingsDlg(int Page);
		~CSettingsDlg();
		enum { IDD = IDD_SETTINGSDLG };
		enum SettingsPage { spGeneral, spServers, spImages, spThumbnails, spScreenshot,
		spVideo, spUploading, spIntegration, spTrayIcon, spHotkeys};
	protected:
		BEGIN_MSG_MAP(CSettingsDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			NOTIFY_HANDLER(IDC_TABCONTROL, TCN_SELCHANGE, OnTabChanged)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER_EX(IDC_APPLY, BN_CLICKED, OnApplyBnClicked)
		COMMAND_HANDLER(IDC_SETTINGSPAGESLIST, LBN_SELCHANGE, OnSettingsPagesSelChanged)
			REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	void CloseDialog(int nVal);
	LRESULT OnSettingsPagesSelChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	int CurPage;
	CIcon hIcon;
	CIcon hIconSmall;
	int PrevPage,NextPage;
	bool CreatePage(int PageID);
	CSettingsPage* Pages[SettingsPageCount];
	int PageToShow;
	bool ShowPage(int idPage);
	CTabListBox m_SettingsPagesListBox;
	LRESULT OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
};

#endif // SETTINGSDLG_H