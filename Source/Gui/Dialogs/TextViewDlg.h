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

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Func/LangClass.h"

// CTextViewDlg
using namespace WTL;
class CTextViewDlg : 
	public CDialogImpl<CTextViewDlg>,
	public CDialogResize<CTextViewDlg>	
{
	public:
		CTextViewDlg(const CString &text, const CString &title, const CString &info, const CString &question , const CString &okCaption=TR("OK"),const CString &cancelCaption=TR("Отмена"));
		~CTextViewDlg();
		enum { IDD = IDD_TEXTVIEWDLG };

	protected:
		BEGIN_MSG_MAP(CTextViewDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			  COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			  COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			CHAIN_MSG_MAP(CDialogResize<CTextViewDlg>)
		 END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CTextViewDlg)
				DLGRESIZE_CONTROL(IDC_QUESTIONLABEL, DLSZ_MOVE_Y|DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_TEXTEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X| DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		END_DLGRESIZE_MAP()

		 // Handler prototypes:
		 //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		 //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		 //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		CString m_okCaption,m_cancelCaption, m_text, m_title, m_info, m_question;
};


