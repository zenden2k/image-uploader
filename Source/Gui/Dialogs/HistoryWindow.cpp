/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "HistoryWindow.h"

#include "Func/LangClass.h"
#include "Core/Settings.h"
#include "Core/HistoryManager.h"
#include "Core/ServiceLocator.h"
#include "ResultsWindow.h"
#include "Core/3rdpart/pcreplusplus.h"
#include "Func/WinUtils.h"
#include "ClearHistoryDlg.h"
#include "Core/Utils/DesktopUtils.h"

// CHistoryWindow
CHistoryWindow::CHistoryWindow(CWizardDlg* wizardDlg)
{
    m_historyReader = 0;
    delayed_closing_ = false;
    wizardDlg_ = wizardDlg;
}

CHistoryWindow::~CHistoryWindow()
{
    if(m_hWnd) 
    {
        Detach();
        m_hWnd = NULL;
    }
    delete m_historyReader;
}

LRESULT CHistoryWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow();
    DlgResize_Init();
    m_treeView.SubclassWindow(GetDlgItem(IDC_HISTORYTREE));
    m_treeView.onThreadsFinished.bind(this, &CHistoryWindow::threadsFinished);
    m_treeView.onThreadsStarted.bind(this, &CHistoryWindow::threadsStarted);
    m_treeView.onItemDblClick.bind(this, &CHistoryWindow::onItemDblClick);
    TRC(IDCANCEL, "Close");
    TRC(IDC_SESSIONSCOUNTDESCR, "Session total:");
    TRC(IDC_FILESCOUNTDESCR, "Files total:");
    TRC(IDC_UPLOADTRAFFICDESCR, "Total size:");
    SetWindowText(TR("Upload History"));
    TRC(IDC_TIMEPERIODLABEL, "Time Period:");
    TRC(IDC_DOWNLOADTHUMBS, "Retrieve thumbnails from the Internet");
    TRC(IDC_CLEARHISTORYBTN, "Clear History...");

    HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
    if (hWnd)
    {
        m_wndAnimation.SubclassWindow(hWnd);
        if (m_wndAnimation.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF),_T("GIF")))
            m_wndAnimation.Draw();
        m_wndAnimation.ShowWindow(SW_HIDE);
    }
    
    LoadMonthList();
    
    SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_SETCHECK, static_cast<WPARAM>(Settings.HistorySettings.EnableDownloading));
    SelectedMonthChanged();
    m_treeView.SetFocus();
    return 1;  // Let the system set the focus
}

BOOL CHistoryWindow::PreTranslateMessage(MSG* pMsg)
{
    return CWindow::IsDialogMessage(pMsg);
}

LRESULT CHistoryWindow::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    delayed_closing_ = true;
    if(!m_treeView.isRunning())
    {
        Settings.HistorySettings.EnableDownloading = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
        EndDialog(0);
    }
    else
    {
        ::EnableWindow(GetDlgItem(IDCANCEL), false);
        ::EnableWindow(GetDlgItem(IDC_MONTHCOMBO), true);
        m_treeView.EnableWindow(true);
        m_treeView.abortLoadingThreads();
    }
    return 0;
}

void CHistoryWindow::Show()
{
    if(!IsWindowVisible())
            ShowWindow(SW_SHOW);
    SetForegroundWindow(m_hWnd);
}

void CHistoryWindow::LoadMonthList() {
    SendDlgItemMessage(IDC_MONTHCOMBO, CB_RESETCONTENT, 0, 0);
    std::vector<CString> files;
    historyFolder = U2W(Settings.SettingsFolder) + CString(_T("\\History\\"));
    WinUtils::GetFolderFileList(files, historyFolder, _T("history*.xml"));
    pcrepp::Pcre regExp("history_(\\d+)_(\\d+)", "imcu");

    for (size_t i = 0; i<files.size(); i++) {
        m_HistoryFiles.push_back(files[i]);

        CString monthLabel = Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt(WCstringToUtf8(files[i])));

        size_t pos = 0;

        if (regExp.search(WCstringToUtf8(monthLabel), pos)) {
            std::string yearStr = regExp[1];
            std::string monthStr = regExp[2];
            int year = atoi(yearStr.c_str());
            int month = atoi(monthStr.c_str());
            monthLabel.Format(_T("%d/%02d"), year, month);
        } else {
            monthLabel.Replace(_T("history_"), _T(""));
            monthLabel.Replace(_T("_"), _T("/"));
        }

        int newItemIndex = SendDlgItemMessage(IDC_MONTHCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)monthLabel);
        if (newItemIndex >= 0) {
            SendDlgItemMessage(IDC_MONTHCOMBO, CB_SETITEMDATA, newItemIndex, i);
        }
    }
    int selectedIndex = files.size() - 1;
    SendDlgItemMessage(IDC_MONTHCOMBO, CB_SETCURSEL, selectedIndex, 0);
}

LRESULT CHistoryWindow::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwnd = reinterpret_cast<HWND>(wParam);  
    POINT ClientPoint, ScreenPoint;
    if(hwnd != GetDlgItem(IDC_HISTORYTREE)) return 0;

    if(lParam == -1) 
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
    bool isSessionItem  = false;
    TreeItem* item = m_treeView.selectedItem();
    if(!item) return 0;
    
    isSessionItem = item->level()==0;
    
    HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
    CMenu menu;
    menu.CreatePopupMenu();
    if(!isSessionItem)
    {
        menu.AppendMenu(MF_STRING, ID_OPENINBROWSER, TR("Open in Web Browser"));
        menu.AppendMenu(MF_STRING, ID_COPYTOCLIPBOARD, TR("Copy URL"));
    }
    menu.AppendMenu(MF_STRING, ID_VIEWBBCODE, TR("View BBCode/HTML codes"));
    if(!isSessionItem)
    {
        std::string fileName  = historyItem->localFilePath;
        if(!fileName.empty() && IuCoreUtils::DirectoryExists(IuCoreUtils::ExtractFilePath(fileName)))
        {
            menu.AppendMenu(MF_STRING, ID_OPENFOLDER, TR("Open in folder"));
        }
        if (!historyItem->editUrl.empty())
        {
            menu.AppendMenu(MF_STRING, ID_EDITFILEONSERVER, TR("Edit file on server"));
        }
        if (!historyItem->deleteUrl.empty())
        {
            menu.AppendMenu(MF_STRING, ID_DELETEFILEONSERVER, TR("Delete file from server"));
        }
    }
    menu.SetMenuDefaultItem(0, true);
    menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
    return 0;
}

void CHistoryWindow::FillList(CHistoryReader * mgr)
{
    bool enabledDownload = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
    m_treeView.setDownloadingEnabled(enabledDownload);
    int nSessionsCount = mgr->getSessionCount();

    m_treeView.SetRedraw(false);
    TreeItem* res = 0;
    int nFilesCount = 0;
    int64_t totalFileSize = 0;
    for(int i=0; i<nSessionsCount; i++)
    {
        CHistorySession* ses = mgr->getSession(i);
        std::string serverName = ses->serverName();
        if(serverName.empty()) serverName = "n/a";

        std::string label = IuCoreUtils::timeStampToString(ses->timeStamp())+ "\r\n Server: "+ serverName+ " Files: " + IuCoreUtils::toString(ses->entriesCount()); 
        res = m_treeView.addEntry(ses, Utf8ToWCstring(label));
        int nCount = ses->entriesCount();
        for(int j=0; j<nCount; j++)
        {
            nFilesCount++;
            if ( ses->entry(j).uploadFileSize < 1000000000 ) {
                totalFileSize += ses->entry(j).uploadFileSize;
            }
            m_treeView.addSubEntry(res, ses->entry(j),nCount<4);
        }
        //m_treeView.ExpandItem(res);
    }
    if(res)
    {
        m_treeView.SetCurSel(m_treeView.GetCount()-1);
        m_treeView.SetCurSel(-1);
    }
    m_treeView.SetRedraw(true);

    SetDlgItemInt(IDC_FILESCOUNTLABEL, nFilesCount, false);
    SetDlgItemInt(IDC_SESSIONSCOUNTLABEL, nSessionsCount, false);
    SetDlgItemText(IDC_UPLOADTRAFFICLABEL, Utf8ToWCstring(IuCoreUtils::fileSizeToString(totalFileSize)));
}

LRESULT CHistoryWindow::OnHistoryTreeCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    bHandled = true;
    if(m_treeView.m_hWnd == 0) return 0;
    return 0;
}

LRESULT CHistoryWindow::OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if(!item) return 0;
    OpenInBrowser(item);
    return 0;
}

LRESULT CHistoryWindow::OnCopyToClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if(!item) return 0;
    HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
    std::string url = historyItem->directUrl.length()?historyItem->directUrl:historyItem->viewUrl;
    WinUtils::CopyTextToClipboard(Utf8ToWCstring(url));
    return 0;
}

CUrlListItem fromHistoryItem(const HistoryItem& historyItem)
{
    CUrlListItem it;
    it.ImageUrl = Utf8ToWstring(historyItem.directUrl).c_str();
    it.ImageUrlShortened = Utf8ToWstring(historyItem.directUrlShortened).c_str();
    it.ThumbUrl =  Utf8ToWstring(historyItem.thumbUrl).c_str();
    it.DownloadUrl = Utf8ToWstring(historyItem.viewUrl).c_str();
    it.DownloadUrlShortened = Utf8ToWstring(historyItem.viewUrlShortened).c_str();
    it.FileName = Utf8ToWstring(historyItem.displayName).c_str();
    return it;
}

LRESULT CHistoryWindow::OnViewBBCode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if(!item) return 0;
    std::vector<CUrlListItem> items;

    if(item->level()==0)
    {
        CHistorySession* ses = reinterpret_cast<CHistorySession*>(item->userData());
        for(int i=0; i<ses->entriesCount(); i++)
        {
            CUrlListItem it  =fromHistoryItem(ses->entry(i));
            items.push_back(it);
        }
    }
    else
    {
        HistoryItem* hit = reinterpret_cast<HistoryItem*>(item->userData());
        CUrlListItem it  = fromHistoryItem(*hit);
        items.push_back(it);
    }
    CResultsWindow rp(wizardDlg_, items, false);
    rp.DoModal(m_hWnd);
    return 0;
}

LRESULT CHistoryWindow::OnOpenFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if(!item) return 0;
    HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
    std::string fileName  = historyItem->localFilePath;
    if(fileName.empty()) return 0;
    std::string directory = IuCoreUtils::ExtractFilePath(fileName);
    if(IuCoreUtils::FileExists(fileName))
    {
        ShellExecuteW(NULL, NULL, L"explorer.exe", CString(_T("/select, ")) + Utf8ToWCstring(fileName), NULL, SW_SHOWNORMAL);
    }
    else if(IuCoreUtils::DirectoryExists(directory))
    {
        ShellExecute(NULL, _T("open"), Utf8ToWCstring(directory), NULL, NULL, SW_SHOWNORMAL);
    }
    return 0;
}

LRESULT CHistoryWindow::OnEditFileOnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if (!item) return 0;
    HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
    ShellExecute(0, _T("open"), U2W(historyItem->editUrl), NULL, NULL, SW_SHOWNORMAL);
    return 0;
}

LRESULT CHistoryWindow::OnDeleteFileOnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if (!item) return 0;
    HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
    DesktopUtils::ShellOpenUrl(historyItem->deleteUrl);
    return 0;
}

LRESULT CHistoryWindow::OnMonthChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SelectedMonthChanged();
    return 0;
}
        
void CHistoryWindow::LoadHistoryFile(CString fileName)
{
    m_delayedFileName = fileName;
    if(!m_treeView.isRunning())
    {   
        m_treeView.ResetContent();

        delete m_historyReader;
        m_historyReader = new CHistoryReader();
        m_historyReader->loadFromFile(WCstringToUtf8(historyFolder + fileName));
        FillList(m_historyReader);
        m_delayedFileName.Empty();
    }
    else
    {
        ::EnableWindow(GetDlgItem(IDC_MONTHCOMBO), false);
        ::EnableWindow(GetDlgItem(IDCANCEL), false);
        ::EnableWindow(GetDlgItem(IDC_CLEARHISTORYBTN), false);
        m_treeView.EnableWindow(false);
        m_treeView.abortLoadingThreads();
    }
}

void CHistoryWindow::SelectedMonthChanged() {
    int nIndex = SendDlgItemMessage(IDC_MONTHCOMBO, CB_GETCURSEL);
    if (nIndex == -1) {
        m_treeView.ResetContent();
        return;
    }
    int historyFileIndex = static_cast<int>(SendDlgItemMessage(IDC_MONTHCOMBO, CB_GETITEMDATA, nIndex));
    LoadHistoryFile(m_HistoryFiles[historyFileIndex]);
    m_treeView.SetFocus();
}

void CHistoryWindow::OpenInBrowser(TreeItem* item) {
    HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
    std::string url = historyItem->directUrl.length() ? historyItem->directUrl : historyItem->viewUrl;
    ShellExecute(NULL, _T("open"), Utf8ToWCstring(url), NULL, NULL, SW_SHOWNORMAL);
}

void CHistoryWindow::threadsFinished()
{
    m_wndAnimation.ShowWindow(SW_HIDE);
    
    if(!m_delayedFileName.IsEmpty()) {
        SendMessage(WM_MY_OPENHISTORYFILE, reinterpret_cast<WPARAM>(static_cast<LPCTSTR>(m_delayedFileName)));
        //LoadHistoryFile(m_delayedFileName);
        
    }
    else if(delayed_closing_)
    {
        EndDialog(0);
        return;
    }
    ::EnableWindow(GetDlgItem(IDC_MONTHCOMBO), true);
    m_treeView.EnableWindow(true);
    ::EnableWindow(GetDlgItem(IDCANCEL), true);
    ::EnableWindow(GetDlgItem(IDC_CLEARHISTORYBTN), true);
}

void CHistoryWindow::threadsStarted()
{
    m_wndAnimation.ShowWindow(SW_SHOW);
}

LRESULT CHistoryWindow::OnDownloadThumbsCheckboxChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    Settings.HistorySettings.EnableDownloading = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
    m_treeView.setDownloadingEnabled(Settings.HistorySettings.EnableDownloading);
    return 0;
}
        
LRESULT CHistoryWindow::OnWmOpenHistoryFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPCTSTR fileName = reinterpret_cast<TCHAR*>(wParam);
    //m_treeView.setDownloadingEnabled(false);
    LoadHistoryFile(fileName);
    return 0;
}

LRESULT CHistoryWindow::OnBnClickedClearHistoryBtn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CClearHistoryDlg dlg;
    if (dlg.DoModal(m_hWnd) == IDOK) {
        if (ServiceLocator::instance()->historyManager()->clearHistory(dlg.GetValue())) {
            SetDlgItemInt(IDC_FILESCOUNTLABEL, 0, false);
            SetDlgItemInt(IDC_SESSIONSCOUNTLABEL, 0, false);
            SetDlgItemText(IDC_UPLOADTRAFFICLABEL, L"0");
            LoadMonthList();
            SelectedMonthChanged();
            MessageBox(TR("History has been cleared succesfully."), APPNAME, MB_ICONINFORMATION);
        }
    }
    return 0;
}

void CHistoryWindow::onItemDblClick(TreeItem* item) {
    bool isSessionItem = item->level() == 0;
    if (!isSessionItem) {
        OpenInBrowser(item);
    }
}