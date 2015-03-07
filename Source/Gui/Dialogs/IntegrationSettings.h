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

#ifndef IntegrationSettings_H
#define IntegrationSettings_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "Gui/Dialogs/settingspage.h"
#include <Func/Settings.h>
class CIntegrationSettings : public CDialogImpl<CIntegrationSettings>, 
	                      public CSettingsPage	
{
	public:
		enum { IDD = IDD_INTEGRATIONSETTINGS };

		CIntegrationSettings();
		virtual ~CIntegrationSettings();
		virtual bool Apply();

		struct ListItemData {
			ServerProfile serverProfile;
			CString name;
		};

	protected:
		BEGIN_MSG_MAP(CIntegrationSettings)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			COMMAND_HANDLER(IDC_SHELLINTEGRATION, BN_CLICKED, OnShellIntegrationCheckboxChanged)
			COMMAND_HANDLER(IDC_STARTUPLOADINGFROMSHELL, BN_CLICKED, OnClickedQuickUpload)	
			COMMAND_HANDLER(IDC_ADDITEM, BN_CLICKED, OnBnClickedAdditem)
			COMMAND_HANDLER(IDC_DELETEITEM, BN_CLICKED, OnBnClickedDeleteitem)
			COMMAND_HANDLER(IDC_DOWNBUTTON, BN_CLICKED, OnBnClickedDownbutton)
			COMMAND_HANDLER(IDC_UPBUTTON, BN_CLICKED, OnBnClickedUpbutton)
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnShellIntegrationCheckboxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		void ShellIntegrationChanged();
		LRESULT OnClickedQuickUpload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		ServerProfilesMap serverProfiles_;
		CListBox menuItemsListBox_;
		bool menuItemsChanged_;
public:
	LRESULT OnBnClickedAdditem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDeleteitem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDownbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedUpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif // IntegrationSettings_H

