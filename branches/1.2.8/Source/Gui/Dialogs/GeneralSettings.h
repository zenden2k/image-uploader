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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "Gui/Dialogs/settingspage.h"

class CGeneralSettings : public CDialogImpl<CGeneralSettings>, 
	                      public CSettingsPage	
{
	public:
		enum { IDD = IDD_GENERALSETTINGS };

		CGeneralSettings();
		virtual ~CGeneralSettings();
		virtual bool Apply();

	protected:
		BEGIN_MSG_MAP(CGeneralSettings)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
			COMMAND_HANDLER(IDC_BROWSEBUTTON, BN_CLICKED, OnBnClickedBrowse)
			COMMAND_HANDLER(IDC_SHELLINTEGRATION, BN_CLICKED, OnShellIntegrationCheckboxChanged)
			COMMAND_HANDLER(IDC_STARTUPLOADINGFROMSHELL, BN_CLICKED, OnClickedQuickUpload)	
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnShellIntegrationCheckboxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		void ShellIntegrationChanged();
		LRESULT OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedQuickUpload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		int GetNextLngFile(LPTSTR szBuffer, int nLength);
		
		HANDLE findfile;
		CMyImage img;
		WIN32_FIND_DATA wfd;
};

#endif // GENERALSETTINGS_H

