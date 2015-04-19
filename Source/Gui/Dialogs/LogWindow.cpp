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
#include "LogWindow.h"
#include "atlheaders.h"
#include "Func/Settings.h"
#include "TextViewDlg.h"
#include "Func/WinUtils.h"

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

void CLogWindow::WriteLog(LogMsgType MsgType, const CString& Sender, const CString& Msg, const CString& Info)
{
	MsgList.AddString(MsgType, Sender, Msg, Info);
	if (MsgType == logError && Settings.AutoShowLog)
	{
		Show();
	}
}

void CLogWindow::Show()
{
	if (!IsWindowVisible()) {
		ShowWindow(SW_SHOW);
		SetForegroundWindow(m_hWnd);
		::SetActiveWindow(m_hWnd);
		BringWindowToTop();
	}
	SetWindowPos(HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
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
	UINT index = MsgList.ItemFromPoint(ClientPoint, bOutside);

	if ( !bOutside ) {
		MsgList.SetCurSel( index );
	}

	CMenu FolderMenu;
	FolderMenu.CreatePopupMenu();
	if ( index != 0xffff && !bOutside ) {
		FolderMenu.AppendMenu(MF_STRING, IDC_COPYTEXTTOCLIPBOARD, TR("Копировать"));
	}
	FolderMenu.AppendMenu(MF_STRING, IDC_CLEARLIST, TR("Очистить список"));

	FolderMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
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

void CLogWindow::TranslateUI()
{
	TRC(IDCANCEL, "Скрыть");
	SetWindowText(TR("Лог ошибок"));
}

LRESULT CLogWindow::OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CLogWndMsg* msg = (CLogWndMsg*) wParam;
	WriteLog(msg->MsgType, msg->Sender, msg->Msg, msg->Info);
	delete msg;
	return 0;
}

LRESULT CLogWindow::OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MsgList.Clear();
	return 0;
}

void WriteLog(LogMsgType MsgType,  const CString&  Sender,  const CString&  Msg,  const CString&  Info)
{
	if (!LogWindow.m_hWnd)
		return;
	CLogWindow::CLogWndMsg* msg = new CLogWindow::CLogWndMsg();
	msg->Msg = Msg;
	msg->Info = Info;
	msg->Sender = Sender;
	msg->MsgType = MsgType;
	LogWindow.PostMessage(MYWM_WRITELOG, (WPARAM)msg);
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

int responseFileIndex = 1;

void DebugMessage(const std::string& msg, bool isResponseBody)
{
	if (!isResponseBody)
		MessageBox(0, Utf8ToWCstring(msg.c_str()), _T("Uploader"), MB_ICONINFORMATION);
	else
	{
		CTextViewDlg TextViewDlg(Utf8ToWstring(msg).c_str(), CString(_T("Server reponse")), CString(_T("Server reponse:")),
		                         _T("Save to file?"));

		if (TextViewDlg.DoModal(GetActiveWindow()) == IDOK)
		{
			CFileDialog fd(false, 0, 0, 4 | 2, _T("*.html\0*.html\0\0"), GetActiveWindow());
			CString fileName;
			fileName.Format(_T("response_%02d.html"), responseFileIndex++);
			lstrcpy(fd.m_szFileName, fileName);
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
