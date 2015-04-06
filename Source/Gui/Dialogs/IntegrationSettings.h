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

#ifndef IntegrationSettings_H
#define IntegrationSettings_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "Gui/Dialogs/settingspage.h"
#include <Func/Settings.h>
#include "Gui/Controls/IconButton.h"

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
			bool invalid;
			CString itemId;
			ListItemData() {
				invalid = false;
			}
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
			REFLECT_NOTIFICATIONS()
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
		CIconButton upButton_;
		CIconButton downButton_;
		CIconButton addItemButton_;
		CIconButton deleteItemButton_;
public:
	LRESULT OnBnClickedAdditem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDeleteitem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDownbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedUpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif // IntegrationSettings_H

