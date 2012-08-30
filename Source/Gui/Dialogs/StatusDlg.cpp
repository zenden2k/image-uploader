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

#include "atlheaders.h"
#include "StatusDlg.h"
#include "Func/Common.h"
#include "Func/MyUtils.h"
#include "Func/LangClass.h"
#include "Gui/GuiTools.h"
// CStatusDlg
CStatusDlg::CStatusDlg()
{
		
}

CStatusDlg::~CStatusDlg()
{
}

LRESULT CStatusDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	m_bNeedStop = false;
	SetTimer(1, 500);
	TRC(IDCANCEL, "Остановить");
	GuiTools::MakeLabelBold(GetDlgItem(IDC_TITLE));
	return 1;  // Let the system set the focus
}

LRESULT CStatusDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Section2.Lock();
	m_bNeedStop = true;
	Section2.Unlock();
	KillTimer(1);
	
	return 0;
}

void CStatusDlg::SetInfo(const CString& Title, const CString& Text)
{
	CriticalSection.Lock();
	m_Title=Title;
	m_Text = Text;
	CriticalSection.Unlock();
}

LRESULT CStatusDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(!IsWindowVisible() && !m_bNeedStop)
		ShowWindow(SW_SHOW);
	SetDlgItemText(IDC_TITLE, m_Title);
	SetDlgItemText(IDC_TEXT, m_Text);
	return 0;
}

void CStatusDlg::SetWindowTitle(const CString& WindowTitle)
{
	CriticalSection.Lock();
	SetWindowText(WindowTitle);
	CriticalSection.Unlock();
}

bool CStatusDlg::NeedStop()
{
	Section2.Lock();
	bool res = m_bNeedStop;
	Section2.Unlock();
	return res;
}

void CStatusDlg::Hide()
{
	KillTimer(1);
	ShowWindow(SW_HIDE);
	m_bNeedStop = false;
}