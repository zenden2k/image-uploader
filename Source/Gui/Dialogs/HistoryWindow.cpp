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

#include "Core/i18n/Translator.h"
#include "Core/Settings/WtlGuiSettings.h"
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
    delayed_closing_ = false;
    wizardDlg_ = wizardDlg;
    delayedLoad_ = false;
}

CHistoryWindow::~CHistoryWindow() 
{
}

LRESULT CHistoryWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    CenterWindow();
    DlgResize_Init();
    dateFromPicker_ = GetDlgItem(IDC_DATEFROMPICKER);
    dateToPicker_ = GetDlgItem(IDC_DATETOPICKER);

    dateFilterCheckbox_ = GetDlgItem(IDC_DATEFROMCHECKBOX);
    dateFilterCheckbox_.SetCheck(BST_CHECKED);

    m_treeView.SubclassWindow(GetDlgItem(IDC_HISTORYTREE));
    m_treeView.onThreadsFinished.bind(this, &CHistoryWindow::threadsFinished);
    m_treeView.onThreadsStarted.bind(this, &CHistoryWindow::threadsStarted);
    m_treeView.onItemDblClick.bind(this, &CHistoryWindow::onItemDblClick);
    TRC(IDOK, "Apply");
    TRC(IDC_CLEARFILTERS, "Clear filters");
    TRC(IDC_FILTERSGROUPBOX, "Filters");
    TRC(IDC_DATEFROMCHECKBOX, "Date from:");
    TRC(IDC_DATETOLABEL, "to:");
    TRC(IDC_FILENAMELABEL, "Filename:");
    TRC(IDC_URLLABEL, "URL:");
    TRC(IDCANCEL, "Close");
    TRC(IDC_SESSIONSCOUNTDESCR, "Session total:");
    TRC(IDC_FILESCOUNTDESCR, "Files total:");
    TRC(IDC_UPLOADTRAFFICDESCR, "Total size:");
    SetWindowText(TR("Upload History"));
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

    SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_SETCHECK, static_cast<WPARAM>(Settings.HistorySettings.EnableDownloading));
    //SelectedMonthChanged();
    initSearchForm();
    dateFromCheckboxChanged();
    applyFilters();
    m_treeView.SetFocus();
    return 0; 
}

BOOL CHistoryWindow::PreTranslateMessage(MSG* pMsg)
{
    return CWindow::IsDialogMessage(pMsg);
}

LRESULT CHistoryWindow::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    delayed_closing_ = true;
    if(!m_treeView.isRunning())
    {
        Settings.HistorySettings.EnableDownloading = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
        EndDialog(0);
    }
    else
    {
        ::EnableWindow(GetDlgItem(IDCANCEL), false);
        ::EnableWindow(GetDlgItem(IDOK), false);
        ::EnableWindow(GetDlgItem(IDC_CLEARFILTERS), false);
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

LRESULT CHistoryWindow::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwnd = reinterpret_cast<HWND>(wParam);  
    POINT ClientPoint, ScreenPoint;
    if(hwnd != GetDlgItem(IDC_HISTORYTREE)) return 0;
    bool isSessionItem = false;
    TreeItem* item = m_treeView.selectedItem();
    if (!item) return 0;

    if(lParam == -1) 
    {
        ClientPoint.x = 0;
        ClientPoint.y = 0;
        int itemIndex = m_treeView.GetCurSel();
        if (itemIndex >= 0) {
            CRect rc; 
            if (m_treeView.GetItemRect(itemIndex, &rc) != LB_ERR) {
                ClientPoint = rc.CenterPoint();
            }
        }
        
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
   
    isSessionItem = item->level()==0;
    
    HistoryItem* historyItem = CHistoryTreeControl::getItemData(item);
    CMenu menu;
    menu.CreatePopupMenu();
    if(!isSessionItem)
    {
        menu.AppendMenu(MF_STRING, ID_OPENINBROWSER, TR("Open in Web Browser"));
        menu.SetMenuDefaultItem(ID_OPENINBROWSER, FALSE);
        menu.AppendMenu(MF_STRING, ID_COPYTOCLIPBOARD, TR("Copy URL"));
    }
    menu.AppendMenu(MF_STRING, ID_VIEWBBCODE, TR("View BBCode/HTML codes"));
    if(!isSessionItem)
    {
        if(!historyItem->localFilePath.empty() && 
            IuCoreUtils::DirectoryExists(IuCoreUtils::ExtractFilePath(historyItem->localFilePath)))
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
            m_treeView.addSubEntry(res, &ses->entry(j),nCount<4);
        }
        ses->setDeleteItems(false); // treeView will manage histroy items
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
    HistoryItem* historyItem = CHistoryTreeControl::getItemData(item);
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
        HistoryItem* hit = CHistoryTreeControl::getItemData(item);
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
    HistoryItem* historyItem = CHistoryTreeControl::getItemData(item);
    std::string fileName  = historyItem->localFilePath;
    if(fileName.empty()) return 0;
    std::string directory = IuCoreUtils::ExtractFilePath(fileName);
    if(IuCoreUtils::FileExists(fileName))
    {
        WinUtils::ShowFileInFolder(U2W(fileName), m_hWnd);
    }
    else if(IuCoreUtils::DirectoryExists(directory))
    {
        WinUtils::ShellOpenFileOrUrl(U2W(directory), m_hWnd);
    }
    return 0;
}

LRESULT CHistoryWindow::OnEditFileOnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if (!item) return 0;
    HistoryItem* historyItem = CHistoryTreeControl::getItemData(item);
    WinUtils::ShellOpenFileOrUrl(U2W(historyItem->editUrl), m_hWnd);
    return 0;
}

LRESULT CHistoryWindow::OnDeleteFileOnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    TreeItem* item = m_treeView.selectedItem();
    if (!item) return 0;
    if (LocalizedMessageBox(TR("Are you sure?"), APPNAME, MB_ICONQUESTION|MB_YESNO) == IDYES) {
        HistoryItem* historyItem = CHistoryTreeControl::getItemData(item);
        DesktopUtils::ShellOpenUrl(historyItem->deleteUrl);
    }
    
    return 0;
}
        
void CHistoryWindow::LoadHistory()
{
    //m_delayedFileName = fileName;
    if(!m_treeView.isRunning())
    {   
        m_treeView.ResetContent();

        m_historyReader.reset(new CHistoryReader(ServiceLocator::instance()->historyManager()));

        SYSTEMTIME dateFrom, dateTo;
        time_t timeFrom = 0, timeTo = 0;
        if (dateFilterCheckbox_.GetCheck() == BST_CHECKED) {
            if (dateFromPicker_.GetSystemTime(&dateFrom) == GDT_VALID) {
                dateFrom.wHour = 0;
                dateFrom.wMinute = 0;
                dateFrom.wSecond = 0;
                dateFrom.wMilliseconds = 0;
                timeFrom = WinUtils::SystemTimeToTime(dateFrom);
            }
            if (dateToPicker_.GetSystemTime(&dateTo) == GDT_VALID) {
                dateTo.wHour = 23;
                dateTo.wMinute = 59;
                dateTo.wSecond = 59;
                dateTo.wMilliseconds = 999;
                timeTo = WinUtils::SystemTimeToTime(dateTo);
            }
        }
        CString fileName = GuiTools::GetDlgItemText(m_hWnd, IDC_FILENAMEEDIT);
        CString url = GuiTools::GetDlgItemText(m_hWnd, IDC_URLEDIT);
        if (m_historyReader->loadFromDB(timeFrom, timeTo, W2U(fileName), W2U(url))) {
            FillList(m_historyReader.get());
        }

        delayedLoad_ = false;
    }
    else
    {
        delayedLoad_ = true;
        ::EnableWindow(GetDlgItem(IDOK), false);
        ::EnableWindow(GetDlgItem(IDCANCEL), false);
        ::EnableWindow(GetDlgItem(IDC_CLEARHISTORYBTN), false);
        ::EnableWindow(GetDlgItem(IDC_CLEARFILTERS), false);
        m_treeView.EnableWindow(false);
        m_treeView.abortLoadingThreads();
    }
}

void CHistoryWindow::OpenInBrowser(const TreeItem* item) {
    HistoryItem* historyItem = CHistoryTreeControl::getItemData(item);
    std::string url = historyItem->directUrl.length() ? historyItem->directUrl : historyItem->viewUrl;
    WinUtils::ShellOpenFileOrUrl(U2W(url), m_hWnd);
}

void CHistoryWindow::threadsFinished()
{
    m_wndAnimation.ShowWindow(SW_HIDE);
    
    if (delayedLoad_) {
        SendMessage(WM_MY_OPENHISTORYFILE);        
    }
    else if(delayed_closing_)
    {
        EndDialog(0);
        return;
    }
    m_treeView.EnableWindow(true);
    ::EnableWindow(GetDlgItem(IDCANCEL), true);
    ::EnableWindow(GetDlgItem(IDOK), true);
    ::EnableWindow(GetDlgItem(IDC_CLEARFILTERS), true);
    ::EnableWindow(GetDlgItem(IDC_CLEARHISTORYBTN), true);
}

void CHistoryWindow::threadsStarted()
{
    m_wndAnimation.ShowWindow(SW_SHOW);
}

LRESULT CHistoryWindow::OnDownloadThumbsCheckboxChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    Settings.HistorySettings.EnableDownloading = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
    m_treeView.setDownloadingEnabled(Settings.HistorySettings.EnableDownloading);
    return 0;
}
        
LRESULT CHistoryWindow::OnWmOpenHistoryFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{    
    LoadHistory();
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
            applyFilters();
            LocalizedMessageBox(TR("History has been cleared successfully."), APPNAME, MB_ICONINFORMATION);
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

void CHistoryWindow::applyFilters() {
    LoadHistory();
}

LRESULT CHistoryWindow::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    applyFilters();
    return 0;
}

LRESULT CHistoryWindow::OnDateFromCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    dateFromCheckboxChanged();
    return 0;
}

void CHistoryWindow::dateFromCheckboxChanged() {
    BOOL isChecked = dateFilterCheckbox_.GetCheck() == BST_CHECKED;
    dateFromPicker_.EnableWindow(isChecked);
    dateToPicker_.EnableWindow(isChecked);
}

LRESULT CHistoryWindow::OnClearFilters(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    dateFilterCheckbox_.SetCheck(BST_UNCHECKED);
    
    initSearchForm();
    LoadHistory();
    return 0;
}

void CHistoryWindow::initSearchForm() {
    SYSTEMTIME st, st2;

    GetLocalTime(&st);
    st2 = WinUtils::SystemTimeAdd(st, -30 * 3600 * 24);
    dateFromPicker_.SetSystemTime(GDT_VALID, &st2);
    dateToPicker_.SetSystemTime(GDT_VALID, &st);

    SetDlgItemText(IDC_FILENAMEEDIT, _T(""));
    SetDlgItemText(IDC_URLEDIT, _T(""));

    dateFromCheckboxChanged();
}