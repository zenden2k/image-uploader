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

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include <atlcrack.h>
#include "SettingsPage.h"

class CConvertPresetDlg;
class CSettingsPage;

const int ConvertPageCount = 2;

class CConvertPresetDlg :
	public CDialogImpl<CConvertPresetDlg>  // , public CUpdateUI<CConvertPresetDlg>
	//	public CMessageFilter, public CIdleHandler
{
	public:
		CConvertPresetDlg(int Page);
		~CConvertPresetDlg();
		enum { IDD = IDD_CONVERTPRESETDLG };

		BEGIN_MSG_MAP(CConvertPresetDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			NOTIFY_HANDLER(IDC_TABCONTROL, TCN_SELCHANGE, OnTabChanged)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER_EX(IDC_APPLY, BN_CLICKED, OnApplyBnClicked)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		void CloseDialog(int nVal);
		int CurPage;
		int PrevPage, NextPage;
		bool CreatePage(int PageID);
		CSettingsPage* Pages[ConvertPageCount];
		int PageToShow;
		bool ShowPage(int idPage);
		LRESULT OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
};
