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

#include "ServerFolderSelect.h"

#include "Core/Upload/UploadEngineManager.h"
#include "NewFolderDlg.h"
#include "Core/Settings.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Func/WinUtils.h"
#include "Core/CoreFunctions.h"

CServerFolderSelect::CServerFolderSelect(ServerProfile& serverProfile, UploadEngineManager * uploadEngineManager) :serverProfile_(serverProfile)
{
    m_UploadEngine = serverProfile_.uploadEngineData();
    uploadEngineManager_ = uploadEngineManager;
    runningScript_ = nullptr;
}

CServerFolderSelect::~CServerFolderSelect()
{
}

LRESULT CServerFolderSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_FolderTree = GetDlgItem(IDC_FOLDERTREE);
    CenterWindow(GetParent());

    DlgResize_Init();

    HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
    if (hWnd)
    {
        m_wndAnimation.SubclassWindow(hWnd);
        if (m_wndAnimation.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF), _T("GIF")))
            m_wndAnimation.Draw();
        m_wndAnimation.ShowWindow(SW_HIDE);
    }

    // Internalization
    TRC(IDCANCEL, "Cancel");
    TRC(IDOK, "OK");
    TRC(IDC_NEWFOLDERBUTTON, "Create folder");
    SetWindowText(TR("Folder list"));

    HBITMAP hBitmap;

    // Get color depth (minimum requirement is 32-bits for alpha blended images).
    //int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP), BITSPIXEL);

    hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_SERVERTOOLBARBMP2));
    m_PlaceSelectorImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 6);
    m_PlaceSelectorImageList.Add(hBitmap, RGB(255, 0, 255));

    m_FolderTree.SetImageList(m_PlaceSelectorImageList);
    m_FolderMap[L""] = 0;

    CoreFunctions::ConfigureProxy(&m_NetworkClient);
   
    m_FolderOperationType = foGetFolders;
    CAdvancedUploadEngine *uploadScript=nullptr;
    uploadScript = dynamic_cast<CAdvancedUploadEngine*>(uploadEngineManager_->getUploadEngine(serverProfile_));

    if (!uploadScript)
    {
        SetDlgItemText(IDC_FOLDERLISTLABEL, TR("An error occured while loading script."));
        return 0;
    }
    CString title;
    title.Format(TR("Folder list on server %s for account '%s':"), (LPCTSTR)Utf8ToWCstring(m_UploadEngine->Name),
                 (LPCTSTR)Utf8ToWCstring(serverProfile_.serverSettings().authData.Login));
    SetDlgItemText(IDC_FOLDERLISTLABEL, title);
    
    if (uploadScript)
    {
        uploadScript->setNetworkClient(&m_NetworkClient);
        uploadScript->getAccessTypeList(m_accessTypeList);
        CreateLoadingThread();
    }
    return 1;  // Let the system set the focus
}

LRESULT CServerFolderSelect::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HTREEITEM item = m_FolderTree.GetSelectedItem();

    if (!item)
        return 0;
    int nIndex = m_FolderTree.GetItemData(item);

    if (nIndex < 0 || nIndex > m_FolderList.GetCount() - 1)
        return 0;

    if (!::IsWindowEnabled(GetDlgItem(IDOK)))
        return 0;

    m_SelectedFolder = m_FolderList[nIndex];
    EndDialog(wID);

    return 0;
}

LRESULT CServerFolderSelect::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    {
        std::lock_guard<std::mutex> guard(runningScriptMutex_);
        if (runningScript_) {
            runningScript_->stop();
            stopSignal = true;
        } else {
            EndDialog(wID);
        }
    }
    return 0;
}

DWORD CServerFolderSelect::Run()
{
    CAdvancedUploadEngine * script = dynamic_cast<CAdvancedUploadEngine*>(uploadEngineManager_->getUploadEngine(serverProfile_));

    if (!script)
        return 0;

    runningScriptMutex_.lock();
    runningScript_ = script;
    runningScriptMutex_.unlock();

    script->setNetworkClient(&m_NetworkClient);
    m_NetworkClient.setProgressCallback(NetworkClient::ProgressCallback(this, &CServerFolderSelect::progressCallback));


    if (m_FolderOperationType == foGetFolders)
    {
        m_FolderList.Clear();
        m_FolderMap.clear();
       
        NetworkClient networkClient;
        CoreFunctions::ConfigureProxy(&networkClient);
        script->setNetworkClient(&networkClient);
        networkClient.setProgressCallback(NetworkClient::ProgressCallback(this, &CServerFolderSelect::progressCallback));

        int retCode = script->getFolderList(m_FolderList);
        if (retCode < 1)
        {
            if (retCode == -1)
                TRC(IDC_FOLDERLISTLABEL, "This server doesn't support folders listing.");
            else
                TRC(IDC_FOLDERLISTLABEL, "Failed to load folder list.");
        
            OnLoadFinished();
            return 0;
        }

        m_FolderTree.DeleteAllItems();
        BuildFolderTree(m_FolderList.m_folderItems, "");
        m_FolderTree.SelectItem(m_FolderMap[Utf8ToWstring(m_SelectedFolder.id)]);
    }

    else if (m_FolderOperationType == foCreateFolder)
    {
        script->createFolder(CFolderItem(), m_newFolder);
        m_FolderOperationType = foGetFolders;
        Run();
        m_FolderTree.SelectItem(m_FolderMap[Utf8ToWstring(m_newFolder.id)]);
    }

    else if (m_FolderOperationType == foModifyFolder) // Modifying an existing folder
    {
        script->modifyFolder(m_newFolder);
        m_FolderOperationType = foGetFolders;
        Run();
        m_FolderTree.SelectItem(m_FolderMap[Utf8ToWstring(m_newFolder.id)]);
    }

    OnLoadFinished();
    return 0;
}

void CServerFolderSelect::OnLoadFinished()
{
    {
        std::lock_guard<std::mutex> guard(runningScriptMutex_);
        runningScript_ = nullptr;
        stopSignal = false;
        uploadEngineManager_->clearThreadData();
    }
    BlockWindow(false);
}

void CServerFolderSelect::NewFolder(const CString& parentFolderId)
{
    CFolderItem newFolder;
    CNewFolderDlg dlg(newFolder, true, m_accessTypeList);
    if (dlg.DoModal(m_hWnd) == IDOK)
    {
        m_newFolder = newFolder;
        m_newFolder.parentid = WCstringToUtf8(parentFolderId);
        m_FolderOperationType = foCreateFolder;
        if (!IsRunning())
        {
            CreateLoadingThread();
        }
    }
}

int CServerFolderSelect::progressCallback(INetworkClient* userData, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (stopSignal) {
        return -1; // abort operation
    }
    return 0;
}

LRESULT CServerFolderSelect::OnBnClickedNewfolderbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    NewFolder("");
    return 0;
}

LRESULT CServerFolderSelect::OnFolderlistLbnDblclk(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    SendDlgItemMessage(IDOK, BM_CLICK );
    return 0;
}

LRESULT CServerFolderSelect::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CMenu sub;
    MENUITEMINFO mi;
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_TYPE | MIIM_ID;
    mi.fType = MFT_STRING;
    sub.CreatePopupMenu();

    POINT ClientPoint, ScreenPoint;
    HWND hwnd = reinterpret_cast<HWND>(wParam);
    if (hwnd != m_FolderTree.m_hWnd)
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

    UINT flags;
    HTREEITEM selectedItem = m_FolderTree.HitTest(ClientPoint, &flags);
    if (!selectedItem)
        return 0;

    m_FolderTree.SelectItem(selectedItem);

    mi.wID = ID_EDITFOLDER;
    mi.dwTypeData = TR_CONST("Edit");
    sub.InsertMenuItem(1, true, &mi);

    mi.wID = ID_OPENINBROWSER;
    mi.dwTypeData = TR_CONST("Open in Web Browser");
    sub.InsertMenuItem(0, true, &mi);

    mi.wID = ID_CREATENESTEDFOLDER;
    mi.dwTypeData = TR_CONST("Create nested folder");
    sub.InsertMenuItem(2, true, &mi);

    sub.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
    return 0;
}

LRESULT CServerFolderSelect::OnEditFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HTREEITEM item = m_FolderTree.GetSelectedItem();

    if (!item)
        return 0;
    int nIndex = m_FolderTree.GetItemData(item);

    CFolderItem& folder = m_FolderList[nIndex];

    CNewFolderDlg dlg(folder, false, m_accessTypeList);
    if (dlg.DoModal() == IDOK)
    {
        m_FolderOperationType = foModifyFolder;  // Editing an existing folder
        m_newFolder = folder;
        if (!IsRunning())
        {
            CreateLoadingThread();
        }
    }
    return 0;
}

void CServerFolderSelect::BlockWindow(bool Block)
{
    m_wndAnimation.ShowWindow(Block ? SW_SHOW : SW_HIDE);
    ::EnableWindow(GetDlgItem(IDOK), !Block);
    //::EnableWindow(GetDlgItem(IDCANCEL), !Block);
    ::EnableWindow(GetDlgItem(IDC_NEWFOLDERBUTTON), !Block);
    m_FolderTree.EnableWindow(!Block);
}

LRESULT CServerFolderSelect::OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HTREEITEM item = m_FolderTree.GetSelectedItem();

    if (!item)
        return 0;
    int nIndex = m_FolderTree.GetItemData(item);

    CFolderItem& folder = m_FolderList[nIndex];

    CString str = folder.viewUrl.c_str();
    if (!str.IsEmpty())
    {
        ShellExecute(0, _T("open"), str, _T(""), 0, SW_SHOWNORMAL);
    }
    return 0;
}

void CServerFolderSelect::CreateLoadingThread()
{
    BlockWindow(true);
    Start();
}

LRESULT CServerFolderSelect::OnCreateNestedFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HTREEITEM item = m_FolderTree.GetSelectedItem();

    if (!item)
        return 0;
    int nIndex = m_FolderTree.GetItemData(item);

    CFolderItem& folder = m_FolderList[nIndex];
    NewFolder(folder.id.c_str());
    return 0;
}

void CServerFolderSelect::BuildFolderTree(std::vector<CFolderItem>& list, const CString& parentFolderId)
{
    for (size_t i = 0; i < list.size(); i++)
    {
        CFolderItem& cur = list[i];
        if (cur.parentid.c_str() == parentFolderId)
        {
            CString title = Utf8ToWCstring(cur.title);
            if (cur.itemCount != -1)
                title += _T(" (") + WinUtils::IntToStr(cur.itemCount) + _T(")");
            HTREEITEM res = m_FolderTree.InsertItem(title, 1, 1, m_FolderMap[Utf8ToWstring(cur.parentid)], TVI_SORT  );
            m_FolderTree.SetItemData(res, i);

            m_FolderMap[Utf8ToWstring(cur.id)] = res;
            if (cur.id != "")
                BuildFolderTree(list, cur.id.c_str());
            if (parentFolderId.IsEmpty())
                m_FolderTree.Expand(res);
        }
    }
}
