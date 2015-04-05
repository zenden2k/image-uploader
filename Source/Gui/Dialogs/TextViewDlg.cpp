/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
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

#include "TextViewDlg.h"

// CTextViewDlg
CTextViewDlg::CTextViewDlg(const CString &text, const CString &title, const CString &info, const CString &question , const CString &okCaption,const CString &cancelCaption)
{
	m_text = text;
	m_okCaption = okCaption;
	m_cancelCaption = cancelCaption;
	m_question = question;
	m_info = info;
	m_title = title;
}

CTextViewDlg::~CTextViewDlg()
{
}

LRESULT CTextViewDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DlgResize_Init();
	CenterWindow(GetParent());
	SetDlgItemText(IDOK, m_okCaption);
	SetDlgItemText(IDCANCEL, m_cancelCaption);
	SetDlgItemText(IDC_TEXTEDIT, m_text);
	SetDlgItemText(IDC_QUESTIONLABEL, m_question);
	SetDlgItemText(IDC_TITLETEXT,m_info);
	SetWindowText(m_title);
	return 1;  // Let the system set the focus
}

LRESULT CTextViewDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CTextViewDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CTextViewDlg::OnClickedSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDOK);
	return 0;
}
