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
#include "LogWindow.h"
#include "atlheaders.h"
#include "Func/Settings.h"
#include "TextViewDlg.h"
#include <Func/WinUtils.h>
#include <Func/Myutils.h>

// CLogWindow
CLogWindow::CLogWindow()
{
}

CLogWindow::~CLogWindow()
{
	if (m_hWnd)
	{
		Detach();
		// DestroyWindow();
		m_hWnd = NULL;
	}
}

LRESULT CLogWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	DlgResize_Init();
	MsgList.SubclassWindow(GetDlgItem(IDC_MSGLIST));
	// TODO
	TRC(IDCANCEL, "Скрыть");
	SetWindowText(TR("Лог ошибок"));
	/*WriteLog(logError, "Log Windows", "Test log message", _T("Info"));
	WriteLog(logError, "Log Windows2", "Test log message2\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla\nBlablabla", _T("Info2"));
	*/
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
	if (MsgType == logError && Settings.AutoShowLog)
	{
		Show();
	}
}

void CLogWindow::Show()
{
	if (!IsWindowVisible())
		ShowWindow(SW_SHOW);
	SetForegroundWindow(m_hWnd);
}

LRESULT CLogWindow::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HWND hwnd = (HWND) wParam;
	POINT ClientPoint, ScreenPoint;
	if (hwnd != GetDlgItem(IDC_FILELIST))
		return 0;

	if (lParam == -1)
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
	BOOL bOutside;
	int index = MsgList.ItemFromPoint(ClientPoint, bOutside);
	if ( !bOutside ) {
		MsgList.SetCurSel( index );
	}

	CMenu FolderMenu;
	FolderMenu.CreatePopupMenu();
	FolderMenu.AppendMenu(MF_STRING, IDC_CLEARLIST, TR("Очистить список"));
	FolderMenu.AppendMenu(MF_STRING, IDC_COPYTEXTTOCLIPBOARD, TR("Копировать"));
	FolderMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	return 0;
}

LRESULT CLogWindow::OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CLogWndMsg* msg = (CLogWndMsg*) wParam;
	WriteLog(msg->MsgType, msg->Sender, msg->Msg, msg->Info);
	return 0;
}

LRESULT CLogWindow::OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MsgList.Clear();
	return 0;
}

LRESULT CLogWindow::OnCopyToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
	int messageIndex = MsgList.GetCurSel();
	LogListBoxItem* item = MsgList.getItemFromIndex( messageIndex );
	CString text;
	text.Format(_T("[%s] %s\r\n%s\r\n\r\n%s"), (LPCTSTR)item->Time, (LPCTSTR)item->Info, (LPCTSTR)item->strTitle, (LPCTSTR)item->strText);
	WinUtils::CopyTextToClipboard(text);
	return 0;
}

void WriteLog(LogMsgType MsgType, LPCWSTR Sender, LPCWSTR Msg, LPCWSTR Info)
{
	if (!LogWindow.m_hWnd)
		return;
	CLogWindow::CLogWndMsg msg;
	msg.Msg = Msg;
	msg.Info = Info;
	msg.Sender = Sender;
	msg.MsgType = MsgType;
	LogWindow.SendMessage(MYWM_WRITELOG, (WPARAM)&msg);
}

CLogWindow LogWindow;

namespace DefaultErrorHandling {
void ErrorMessage(ErrorInfo errorInfo)
{
	LogMsgType type = errorInfo.messageType == (ErrorInfo::mtWarning) ? logWarning : logError;
	CString errorMsg;

	CString infoText;
	if (!errorInfo.FileName.empty())
		infoText += TR("Файл: ") + Utf8ToWCstring(errorInfo.FileName) + _T("\n");

	if (!errorInfo.ServerName.empty())
	{
		CString serverName = Utf8ToWCstring(errorInfo.ServerName);
		if (!errorInfo.sender.empty())
			serverName += _T("(") + Utf8ToWCstring(errorInfo.sender) + _T(")");
		infoText += TR("Сервер: ") + serverName +  _T("\n");
	}

	if (!errorInfo.Url.empty())
		infoText += _T("URL: ") + Utf8ToWCstring(errorInfo.Url) + _T("\n");

	if (errorInfo.ActionIndex != -1)
		infoText += _T("Действие:") + CString(_T(" #")) + WinUtils::IntToStr(errorInfo.ActionIndex);

	if (infoText.Right(1) == _T("\n"))
		infoText.Delete(infoText.GetLength() - 1);
	if (!errorInfo.error.empty())
	{
		errorMsg += Utf8ToWCstring(errorInfo.error);
	}
	else
	{
		if (errorInfo.errorType == etRepeating)
		{
			errorMsg.Format(TR("Загрузка на сервер не удалась. Повторяю (%d)..."), errorInfo.RetryIndex );
		}
		else if (errorInfo.errorType == etRetriesLimitReached)
		{
			errorMsg = TR("Загрузка не сервер удалась! (лимит попыток исчерпан)");
		}
	}

	CString sender = TR("Модуль загрузки");
	if (!errorMsg.IsEmpty())
		WriteLog(type, sender, errorMsg, infoText);
}

void DebugMessage(const std::string& msg, bool isResponseBody)
{
	if (!isResponseBody)
		MessageBox(0, Utf8ToWCstring(msg.c_str()), _T("Uploader"), MB_ICONINFORMATION);
	else
	{
		CTextViewDlg TextViewDlg(IuCoreUtils::Utf8ToWstring(msg).c_str(), CString(_T("Server reponse")), CString(_T("Server reponse:")),
		                         _T("Save to file?"));

		if (TextViewDlg.DoModal(GetActiveWindow()) == IDOK)
		{
			CFileDialog fd(false, 0, 0, 4 | 2, _T("*.*\0*.*\0\0"), GetActiveWindow());
			lstrcpy(fd.m_szFileName, _T("file.html"));
			if (fd.DoModal() == IDOK)
			{
				FILE* f = _tfopen(fd.m_szFileName, _T("wb"));
				if (f)
				{
					// WORD BOM = 0xFEFF;
					// fwrite(&BOM, sizeof(BOM),1,f);
					fwrite(msg.c_str(), msg.size(), sizeof(char), f);
					fclose(f);
				}
			}
		}
	}
}
}
