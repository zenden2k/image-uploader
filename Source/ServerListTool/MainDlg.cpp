// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "MainDlg.h"

#include "Core/Settings.h"
#include "Gui/Dialogs/LogWindow.h"
#include "3rdpart/GdiPlusH.h"
#include "Core/Utils/CoreUtils.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Core/Utils/CryptoUtils.h"

#include "Core/Upload/UploadManager.h"

#include "ServersChecker.h"

namespace ServersListTool
{
CMainDlg::CMainDlg(UploadEngineManager* uploadEngineManager, UploadManager* uploadManager, CMyEngineList* engineList) :
model_(engineList), m_ListView(&model_)
{
    uploadEngineManager_ = uploadEngineManager;
    uploadManager_ = uploadManager;
    engineList_ = engineList;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

    contextMenuItemId = -1;
    CenterWindow(); // center the dialog on the screen
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
    DoDataExchange(FALSE);

    // set icons
    icon_ = reinterpret_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
    SetIcon(icon_, TRUE);
    iconSmall_ = reinterpret_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
        IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    SetIcon(iconSmall_, FALSE);

    m_ListView.Init();

    withAccountsRadioButton_.SetCheck(BST_CHECKED);
    checkImageServersCheckBox_.SetCheck(BST_CHECKED);
    m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

    CString testFileName = WinUtils::GetAppFolder() + "testfile.jpg";
    CString testURL = "https://github.com/zenden2k/image-uploader/issues";
    if (xml.LoadFromFile(WCstringToUtf8((WinUtils::GetAppFolder() + "servertool.xml")))) {
        SimpleXmlNode root = xml.getRoot("ServerListTool");
        std::string name = root.Attribute("FileName");
        if (!name.empty()) {
            testFileName = Utf8ToWstring(name.c_str()).c_str();
        }
        std::string url = root.Attribute("URL");
        if (!url.empty()) {
            testURL = Utf8ToWstring(url.c_str()).c_str();
        }
    }
    Settings.MaxThreads = 10;

    SetDlgItemText(IDC_TOOLFILEEDIT, testFileName);
    SetDlgItemText(IDC_TESTURLEDIT, testURL);
    Settings.LoadSettings("", "");
    Settings.ConnectionSettings.UseProxy = ConnectionSettingsStruct::kSystemProxy;
    //iuPluginManager.setScriptsDirectory(WstrToUtf8((LPCTSTR)(WinUtils::GetAppFolder() + "Data/Scripts/")));

    serversChecker_ = std::make_unique<ServersChecker>(&model_, uploadManager_);
    serversChecker_->setOnFinishedCallback(std::bind(&CMainDlg::processFinished, this));
    m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 6);
    m_ListView.SetImageList(m_ImageList, LVSIL_NORMAL);
    /*for (int i = 0; i < engineList_->count(); i++) {
        m_skipMap[i] = false;
        m_ListView.AddItem(i, 0, WinUtils::IntToStr(i + 1), i);
        CUploadEngineData* ued = engineList_->byIndex(i);
        CString name = Utf8ToWstring(ued->Name).c_str();
        if (ued->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            name += _T("  [URL Shortener]");
        }
        m_ListView.SetItemText(i, 1, name);
    }
    */

    return TRUE;
}

LRESULT  CMainDlg::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
    HWND hwnd = reinterpret_cast<HWND>(wParam);
    POINT ClientPoint, ScreenPoint;
    if (hwnd != GetDlgItem(IDC_TOOLSERVERLIST)) return 0;

    if (lParam == -1) {
        ClientPoint.x = 0;
        ClientPoint.y = 0;
        ScreenPoint = ClientPoint;
        ::ClientToScreen(hwnd, &ScreenPoint);
    } else {
        ScreenPoint.x = GET_X_LPARAM(lParam);
        ScreenPoint.y = GET_Y_LPARAM(lParam);
        ClientPoint = ScreenPoint;
        ::ScreenToClient(hwnd, &ClientPoint);
    }
    LV_HITTESTINFO hti;
    memset(&hti, 0, sizeof(hti));
    hti.pt = ClientPoint;
    m_ListView.HitTest(&hti);

    if (hti.iItem >= 0) {
        ServerData* sd = model_.getDataByIndex(hti.iItem);

        if (sd) {

            CMenu menu;
            menu.CreatePopupMenu();
            menu.AppendMenu(MF_STRING, ID_COPYDIRECTURL, _T("Copy direct url"));
            menu.EnableMenuItem(ID_COPYDIRECTURL, sd->directUrl.empty() ? MF_DISABLED : MF_ENABLED);

            menu.AppendMenu(MF_STRING, ID_COPYTHUMBURL, _T("Copy thumb url"));
            menu.EnableMenuItem(ID_COPYTHUMBURL, sd->thumbUrl.empty() ? MF_DISABLED : MF_ENABLED);

            menu.AppendMenu(MF_STRING, ID_COPYVIEWURL, _T("Copy view url"));
            menu.EnableMenuItem(ID_COPYVIEWURL, sd->viewurl.empty() ? MF_DISABLED : MF_ENABLED);

            contextMenuItemId = hti.iItem;
            menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
        }
        
    }

    return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CSimpleDialog<IDD_ABOUTBOX, TRUE> dlg;
    dlg.DoModal();
    return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
    if (!WinUtils::FileExists(fileName)) {
        MessageBox(CString(_T("Test file not found.")) + _T("\r\n") + fileName, APPNAME, MB_ICONERROR);
        return 0;
    }
    /*m_ListView.DeleteAllItems();
    for (int i = 0; i < engineList_->count(); i++) {
        ServerData sd;
        m_CheckedServersMap[i] = sd;
        m_ListView.AddItem(i, 0, WinUtils::IntToStr(i + 1));
        CUploadEngineData* ued = engineList_->byIndex(i);
        CString name = Utf8ToWstring(ued->Name).c_str();
        if (ued->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            name += _T("  [URL Shortener]");
        }
        m_ListView.SetItemText(i, 1, name);
    }*/

    m_srcFileHash = U2W(IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(W2U(fileName)));
    CString report = _T("Source file: ") + GetFileInfo(fileName, &m_sourceFileInfo);
    SetDlgItemText(IDC_TOOLSOURCEFILE, report);
    ::EnableWindow(GetDlgItem(IDOK), false);
    GuiTools::ShowDialogItem(m_hWnd, IDC_STOPBUTTON, true);
    m_NeedStop = false;

    bool useAccounts = withAccountsRadioButton_.GetCheck() == BST_CHECKED || alwaysWithAccountsRadioButton_.GetCheck() == BST_CHECKED;
    bool onlyAccs = alwaysWithAccountsRadioButton_.GetCheck() == BST_CHECKED;

    bool CheckImageServers = checkImageServersCheckBox_.GetCheck() == BST_CHECKED;
    bool CheckFileServers = checkFileServersCheckBox_.GetCheck() == BST_CHECKED;
    bool CheckURLShorteners = checkUrlShortenersCheckBox_.GetCheck() == BST_CHECKED;

    serversChecker_->setCheckFileServers(CheckFileServers);
    serversChecker_->setCheckImageServers(CheckImageServers);
    serversChecker_->setCheckUrlShorteners(CheckURLShorteners);
    serversChecker_->setOnlyAccs(onlyAccs);
    serversChecker_->setUseAccounts(useAccounts);
    model_.resetData();

    //CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
    if (!WinUtils::FileExists(fileName)) {
        LOG(ERROR) << "File not found " << fileName;
        processFinished();
        return -1;
    }
    CString url = GuiTools::GetWindowText(GetDlgItem(IDC_TESTURLEDIT));
    if (CheckURLShorteners && url.IsEmpty()) {
        LOG(ERROR) << "URL should not be empty!";
        processFinished();
        return -1;
    }

    serversChecker_->start(W2U(fileName), W2U(url));
    return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (isRunning()) {
        stop();
    } else {
        SimpleXml savexml;
        CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
        SimpleXmlNode root = savexml.getRoot("ServerListTool");
        root.SetAttribute("FileName", W2U(fileName));
        root.SetAttribute("Time", static_cast<int>(GetTickCount()));
        savexml.SaveToFile(W2U(WinUtils::GetAppFolder() + "servertool.xml"));
        EndDialog(wID);
    }
    return 0;
}

int CMainDlg::Run() {
    return 0;
}


LRESULT CMainDlg::OnSkip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nIndex = -1;
    do {
        nIndex = m_ListView.GetNextItem(nIndex, LVNI_SELECTED);
        if (nIndex == -1) break;
        ServerData* sd = model_.getDataByIndex(nIndex);
        if (sd) {
            sd->skip = !sd->skip;
            model_.notifyRowChanged(nIndex);
        }

    } while (nIndex != -1);

    return 0;
}

/*LRESULT CMainDlg::OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pnmh);

    switch (lplvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
    {
        auto it = m_CheckedServersMap.find(lplvcd->nmcd.dwItemSpec);
        if (it != m_CheckedServersMap.end() && it->second.color) {
            lplvcd->clrTextBk = it->second.color;
            return CDRF_NEWFONT;
        }
    }
    break;
    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
        lplvcd->clrText = RGB(255, 0, 0);
        return CDRF_NEWFONT;
    }
    return 0;
}*/

LRESULT CMainDlg::OnBrowseButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CFileDialog fd(true, 0, 0, 4 | 2, 0, m_hWnd);

    if (fd.DoModal() != IDOK || !fd.m_szFileName) return 0;

    SetDlgItemText(IDC_TOOLFILEEDIT, fd.m_szFileName);
    return 0;
}

void CMainDlg::stop()
{
    m_NeedStop = true;
    serversChecker_->stop();
}

bool CMainDlg::isRunning()
{
    return serversChecker_->isRunning();
}

bool CMainDlg::OnNeedStop()
{
    return m_NeedStop;
}

void CMainDlg::processFinished() {
    ::EnableWindow(GetDlgItem(IDOK), true);
    GuiTools::ShowDialogItem(m_hWnd, IDC_STOPBUTTON, false);
}


LRESULT CMainDlg::OnErrorLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    LogWindow.ShowWindow(LogWindow.IsWindowVisible() ? SW_HIDE : SW_SHOW);
    return 0;
}

LRESULT CMainDlg::OnSkipAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    for (int i = 0; i < engineList_->count(); i++) {
        ServerData* sd = model_.getDataByIndex(i);
        if (sd) {
            sd->skip = !sd->skip;
        }
    }
    m_ListView.Invalidate();
    return 0;
}

LRESULT CMainDlg::OnCopyDirectUrl(WORD, WORD, HWND, BOOL&) {
    ServerData* sd = model_.getDataByIndex(contextMenuItemId);
    if (sd && !sd->directUrl.empty()) {
        WinUtils::CopyTextToClipboard(U2W(sd->directUrl));
    }

    return 0;
}

LRESULT CMainDlg::OnCopyThumbUrl(WORD, WORD, HWND, BOOL&) {
    ServerData* sd = model_.getDataByIndex(contextMenuItemId);
    if (sd && !sd->thumbUrl.empty()) {
        WinUtils::CopyTextToClipboard(U2W(sd->thumbUrl));
    }

    return 0;
}

LRESULT CMainDlg::OnCopyViewUrl(WORD, WORD, HWND, BOOL&) {
    ServerData* sd = model_.getDataByIndex(contextMenuItemId);
    if (sd && !sd->viewurl.empty()) {
        WinUtils::CopyTextToClipboard(U2W(sd->viewurl));
    }

    return 0;
}

LRESULT CMainDlg::OnBnClickedStopbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    serversChecker_->stop();

    return 0;
}
}