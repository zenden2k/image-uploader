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

#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#pragma once

#include "atlheaders.h"
#include <atlframe.h>
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
// CInputDialog

class CInputDialog :
	public CDialogImpl<CInputDialog>
{
	public:
		CInputDialog(const CString& title, const CString& descr, const CString& defaultValue = _T(""), const CString& image = _T(""));
		~CInputDialog();
		enum { IDD = IDD_INPUTDIALOG };

		BEGIN_MSG_MAP(CInputDialog)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		END_MSG_MAP()

	public:
		CString getValue() const;

	protected:
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		CString title_;
		CString description_;
		CString value_;
		CString image_;
		CMyImage imgControl;
};

#endif // INPUTDIALOG_H