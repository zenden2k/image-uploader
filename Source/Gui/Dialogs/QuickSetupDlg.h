/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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

#ifndef QuickSetupDlg_H
#define QuickSetupDlg_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"

// CQuickSetupDlg


class CQuickSetupDlg : public CDialogImpl<CQuickSetupDlg>	
{
	public:
		CQuickSetupDlg();
		~CQuickSetupDlg();
	
		enum { IDD = IDD_QUICKSETUPDLG };
	protected:
		BEGIN_MSG_MAP(CQuickSetupDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_DOAUTHCHECKBOX, BN_CLICKED, OnClickedDoAuthCheckbox)
			COMMAND_HANDLER(IDC_SERVERCOMBOBOX, CBN_SELCHANGE, OnServerComboSelChange)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedDoAuthCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnServerComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	private:
		CMyImage LogoImage;
		CFont NewFont;
		CIcon hIcon;
		CIcon hIconSmall;
		void doAuthCheckboxChanged();
		CComboBoxEx serverComboBox_;
		CImageList comboBoxImageList_;
		void showAuthorizationControls(bool show);
		void serverChanged();
};


#endif // QuickSetupDlg_H