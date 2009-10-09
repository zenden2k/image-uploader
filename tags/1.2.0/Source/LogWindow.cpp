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

#include "stdafx.h"
#include "LogWindow.h"

// CLogWindow
CLogWindow::CLogWindow()
{
}

CLogWindow::~CLogWindow()
{
	if(m_hWnd) 
	{
		DestroyWindow();
		m_hWnd = NULL;
	}
}

LRESULT CLogWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	DlgResize_Init();
	MsgList.SubclassWindow(GetDlgItem(IDC_MSGLIST));
	
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this); 

	TRC(IDCANCEL, "Скрыть");
	SetWindowText(TR("Лог ошибок"));
	return 1;  // Let the system set the focus
}

BOOL CLogWindow::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CLogWindow::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShowWindow(SW_HIDE);
	return 0;
}

void CLogWindow::WriteLog(LogMsgType MsgType, CString Sender, CString Msg, LPCTSTR Info)
{
	MsgList.AddString(MsgType, Sender, _T("") + Msg, Info);
	if(MsgType == logError && Settings.AutoShowLog)
	{
		Show();
	}
}

void CLogWindow::Show()
{
	if(!IsWindowVisible())
			ShowWindow(SW_SHOW);
	SetForegroundWindow(m_hWnd);
}


LRESULT CLogWindow::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	MENUITEMINFO mi;
	HWND 	hwnd = (HWND) wParam;  
	POINT ClientPoint, ScreenPoint;
	if(hwnd != GetDlgItem(IDC_FILELIST)) return 0;

	if(lParam == -1) 
	{
		ClientPoint.x = 0;
		ClientPoint.y = 0;
		ScreenPoint = ClientPoint;
		::ClientToScreen(hwnd, &ScreenPoint);
	}
	else
	{
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}

	CMenu FolderMenu;
	FolderMenu.CreatePopupMenu();
	FolderMenu.AppendMenu(MF_STRING, IDC_CLEARLIST, TR("Очистить список"));
	FolderMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	
	return 0;
}
	
	
LRESULT CLogWindow::OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MsgList.Clear();
	return 0;
}
	
void WriteLog(LogMsgType MsgType, CString Sender, CString Msg,LPCTSTR Info)
{
	if(!LogWindow.m_hWnd) return;
	LogWindow.WriteLog(MsgType, Sender, Msg, Info);
}

CLogWindow LogWindow;
