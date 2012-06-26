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
#ifndef STATUSDLG_H
#define STATUSDLG_H

#pragma once
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "3rdpart/thread.h"

// CStatusDlg

class CStatusDlg :
	public CDialogImpl<CStatusDlg>
{
	public:
		CStatusDlg();
		~CStatusDlg();
		enum { IDD = IDD_STATUSDLG };
		CString m_Title, m_Text;
		bool m_bNeedStop;
		CAutoCriticalSection CriticalSection, Section2;
		BEGIN_MSG_MAP(CStatusDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		void SetInfo(const CString& Title, const CString& Text);
		void SetWindowTitle(const CString& WindowTitle);
		bool NeedStop();
		void Hide();
};

#endif // STATUSDLG_H
