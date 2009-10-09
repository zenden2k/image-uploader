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

#include "resource.h"       // main symbols
#include "Uploader.h"
// CSizeExceed

class CSizeExceed : 
	public CDialogImpl<CSizeExceed>	
{
	public:
		CSizeExceed(LPTSTR szFileName, ImageSettingsStruct &iss);
		~CSizeExceed();
		enum { IDD = IDD_SIZEEXCEED };
		
		 BEGIN_MSG_MAP(CSizeExceed)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			  COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			  COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			  COMMAND_HANDLER(IDC_FORALL, BN_CLICKED, OnBnClickedForall)
			  COMMAND_HANDLER(IDC_KEEPASIS, BN_CLICKED, OnBnClickedKeepasis)
		 END_MSG_MAP()
		 // Handler prototypes:
		 //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		 //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		 //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBnClickedKeepasis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBnClickedForall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		void Translate();
	private:
		CMyImage img;
		CString m_szFileName;
		ImageSettingsStruct &m_ImageSettings;
		void DisplayParams(void);
		void GetParams();
	};


