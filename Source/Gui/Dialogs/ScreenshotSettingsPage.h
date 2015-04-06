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
#ifndef SCREENSHOTSETTINGSPAGE_H
#define SCREENSHOTSETTINGSPAGE_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include <atlcrack.h>
#include "SettingsPage.h"

class CScreenshotSettingsPagePage :	public CDialogImpl<CScreenshotSettingsPagePage>, 
	                                 public CSettingsPage	
{
	public:
		CScreenshotSettingsPagePage();
		~CScreenshotSettingsPagePage();
		enum { IDD = IDD_SCREENSHOTSETTINGSPAGE};

	protected:
		BEGIN_MSG_MAP(CScreenshotSettingsPagePage)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_SCREENSHOTSFOLDERSELECT, BN_CLICKED, OnScreenshotsFolderSelect)
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		bool Apply();
		LRESULT OnScreenshotsFolderSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


#endif // SCREENSHOTSETTINGSPAGE_H