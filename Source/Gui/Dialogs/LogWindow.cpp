/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "Func/WinUtils.h"
#include "Core/i18n/Translator.h"
#include "Core/Settings/BasicSettings.h"

// CLogWindow
CLogWindow::CLogWindow(): mainThreadId_(GetCurrentThreadId()), logger_(nullptr)
{
}

CLogWindow::~CLogWindow()
{
}

void CLogWindow::setLogger(DefaultLogger* logger) {
    if (logger_ != logger) {
        logger_ = logger;
        logger->addListener(this);
    }
}

void CLogWindow::setFileNameFilter(CString fileName) {
    fileNameFilter_ = fileName;
}

LRESULT CLogWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow();
    DlgResize_Init();
    MsgList.SubclassWindow(GetDlgItem(IDC_MSGLIST));

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

void CLogWindow::WriteLogImpl(const DefaultLogger::LogEntry& entry)
{
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
    MsgList.AddString(entry.MsgType, entry.Sender.c_str(), entry.Msg.c_str(), entry.Info.c_str(), entry.Time.c_str());
    if (entry.MsgType == ILogger::logError && Settings.AutoShowLog) {
        Show();
    }
}

void CLogWindow::Show()
{
    if (!IsWindowVisible()) {
        ShowWindow(SW_SHOW);
    }
    SetForegroundWindow(m_hWnd);
    ::SetActiveWindow(m_hWnd);
    //BringWindowToTop();
    //SetWindowPos(HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
}

LRESULT CLogWindow::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    HWND hwnd = reinterpret_cast<HWND>(wParam);
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
        ScreenPoint.x = GET_X_LPARAM(lParam);
        ScreenPoint.y = GET_Y_LPARAM(lParam);
        ClientPoint = ScreenPoint;
        ::ScreenToClient(hwnd, &ClientPoint);
    }

    BOOL bOutside;
    UINT index = MsgList.ItemFromPoint(ClientPoint, bOutside);

    if ( !bOutside ) {
        MsgList.SetSel( index, TRUE );
    }

    CMenu FolderMenu;
    FolderMenu.CreatePopupMenu();
    if ( index != 0xffff && !bOutside ) {
        FolderMenu.AppendMenu(MF_STRING, IDC_COPYTEXTTOCLIPBOARD, TR("Copy") + CString(_T("\tCtrl+C")));
    }
    FolderMenu.AppendMenu(MF_STRING, IDC_SELECTALLITEMS, TR("Select all") + CString(_T("\tCtrl+A")));
    FolderMenu.AppendMenu(MF_STRING, IDC_CLEARLIST, TR("Clear list"));

    FolderMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
    return 0;
}

LRESULT CLogWindow::OnCopyToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/) {
    const int maxItems = 1000;
    int selectedItems[maxItems];
    memset(selectedItems, 0, sizeof(selectedItems));
    int selectedItemsCount = MsgList.GetSelItems(maxItems, selectedItems);

    if (selectedItemsCount >= 0) { 
        CString result;
        for (int i = 0; i < selectedItemsCount; i++) {
            int itemIndex = selectedItems[i];
            LogListBoxItem* item = MsgList.getItemFromIndex(itemIndex);
            CString text;
            text.Format(_T("[%s] %s\r\n%s\r\n\r\n%s"), static_cast<LPCTSTR>(item->Time), static_cast<LPCTSTR>(item->Info),
                static_cast<LPCTSTR>(item->strTitle), static_cast<LPCTSTR>(item->strText));
            result += text;
            result += "\r\n\r\n";
        }
        WinUtils::CopyTextToClipboard(result);
    }
    
    return 0;
}

LRESULT CLogWindow::OnSelectAllItems(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    MsgList.SelectAll();
    return 0;
}

void CLogWindow::TranslateUI()
{
    if (ServiceLocator::instance()->translator()->isRTL()) {
        SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYOUTRTL | WS_EX_RTLREADING); 
        MsgList.SetWindowLong(GWL_EXSTYLE, MsgList.GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYOUTRTL | WS_EX_RTLREADING);
    }
    TRC(IDCANCEL, "Hide");
    TRC(IDC_CLEARLOGBUTTON, "Clear");
    CString windowTitle = TR("Error log");
    if (!fileNameFilter_.IsEmpty()) {
        windowTitle += _T(": ") + WinUtils::myExtractFileName(fileNameFilter_);
    }
    SetWindowText(windowTitle);
}

LRESULT CLogWindow::OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    std::lock_guard<std::mutex> lk(queueMutex_);
    for (const auto& item : queuedItems_) {
        WriteLogImpl(item);
    }
    queuedItems_.clear();
    
    return 0;
}

LRESULT CLogWindow::OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MsgList.Clear();
    return 0;
}

void CLogWindow::WriteLog(const DefaultLogger::LogEntry& entry)
{
    if (!m_hWnd) {
        return;
    }

    if (!fileNameFilter_.IsEmpty()) {
        if (CString(entry.FileName.c_str()) != fileNameFilter_) {
            return;
        }
    }

    if (GetCurrentThreadId() == mainThreadId_) {
        // Call directly
        WriteLogImpl(entry);
    } else {
        {
            std::lock_guard<std::mutex> lk(queueMutex_);
            queuedItems_.push_back(entry);
        }
        PostMessage(MYWM_WRITELOG);
    }
}

LRESULT CLogWindow::OnBnClickedClearLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    MsgList.Clear();
    if (fileNameFilter_.IsEmpty()) {
        logger_->clear();
    }
    return 0;
}

void CLogWindow::onItemAdded(int index, const DefaultLogger::LogEntry& entry) {
    WriteLog(entry);
}

void CLogWindow::reloadList() {
    MsgList.Clear();
    std::lock_guard<std::mutex> lk(logger_->getEntryMutex());
    for (const auto& entry : *logger_) {
        WriteLog(entry);
    }
}