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
#include "Core/Settings.h"
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

void CLogWindow::WriteLogImpl(LogMsgType MsgType, const CString& Sender, const CString& Msg, const CString& Info)
{
    MsgList.AddString(MsgType, Sender, Msg, Info);
    if (MsgType == logError && Settings.AutoShowLog) {
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
    if (hwnd != GetDlgItem(IDC_MSGLIST))
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
        FolderMenu.AppendMenu(MF_STRING, IDC_COPYTEXTTOCLIPBOARD, TR("Copy"));
    }
    FolderMenu.AppendMenu(MF_STRING, IDC_CLEARLIST, TR("Clear list"));

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
    TRC(IDCANCEL, "Hide");
    TRC(IDC_CLEARLOGBUTTON, "Clear");
    SetWindowText(TR("Error log"));
}

LRESULT CLogWindow::OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CLogWndMsg* msg = (CLogWndMsg*) wParam;
    WriteLogImpl(msg->MsgType, msg->Sender, msg->Msg, msg->Info);
    delete msg;
    return 0;
}

LRESULT CLogWindow::OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MsgList.Clear();
    return 0;
}

void CLogWindow::WriteLog(LogMsgType MsgType, const CString&  Sender, const CString&  Msg, const CString&  Info)
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


LRESULT CLogWindow::OnBnClickedClearLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MsgList.Clear();
    return 0;
}
