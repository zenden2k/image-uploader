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

#include "MainDlg.h"

#include "atlheaders.h"
#include "Core/Settings.h"
#include "Func/CmdLine.h"
#include "Func/SystemUtils.h"
#include "Func/WinUtils.h"
#include "Func/Myutils.h"
#include "Gui/GuiTools.h"
#include "Core/Utils/DesktopUtils.h"
#include "ImageEditor/Gui/ImageEditorWindow.h"
#include "Func/ImageEditorConfigurationProvider.h"
#include "Gui/Components/NewStyleFolderDialog.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Gui/Dialogs/SearchByImageDlg.h"
#include "Func/IuCommonFunctions.h"
#include "3rdpart/ShellPidl.h"
#include <boost/format.hpp>
#include <shlobj.h>

CMainDlg::CMainDlg() {
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);

    listChanged_ = false;
    callbackLastCallType_ = false;
    SetTimer(kStatusTimer, 500);
    PageWnd = m_hWnd;
    TRC(IDC_ADDIMAGES, "Add Files");
    TRC(IDC_ADDVIDEO, "Import Video File");
    TRC(IDC_SCREENSHOT, "Screenshot");
    TRC(IDC_PROPERTIES, "Properties");
    TRC(IDC_DELETE, "Remove from list");
    
    ThumbsView.SubclassWindow(GetDlgItem(IDC_FILELIST));
    ThumbsView.Init(true);
    callbackLastCallTime_ = 0;
    ThumbsView.SetOnItemCountChanged([&](CThumbsView*, bool selected)
    {
        DWORD curTime = ::GetTickCount();

        listChanged_ = true;
        if (callbackLastCallType_ == selected && curTime - callbackLastCallTime_ < 150) {
            return;
        }
        callbackLastCallTime_ = curTime;
        callbackLastCallType_ = selected;
        UpdateStatusLabel();
    });
    UpdateStatusLabel();

    ACCEL Accels[1];
    Accels[0].fVirt = FCONTROL | FSHIFT | FVIRTKEY;
    Accels[0].key = VkKeyScan('c');;
    Accels[0].cmd = IDM_COPYFILEPATH;
    hotkeys_ = CreateAcceleratorTable(Accels, ARRAY_SIZE(Accels));

    WaitThreadStop.Create();
    WaitThreadStop.ResetEvent();
    return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DestroyAcceleratorTable(hotkeys_);
    WaitThreadStop.Close();
    return 0;
}

LRESULT CMainDlg::OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("importvideo"));
    return 0;
}

bool CMainDlg::CheckEditInteger(int Control)
{
    TCHAR Buffer[MAX_PATH];
    GetDlgItemText(Control, Buffer, sizeof(Buffer)/sizeof(TCHAR));
    if(lstrlen(Buffer) == 0) return false;
    int n = GetDlgItemInt(Control);
    if(n) SetDlgItemInt(Control, (n<0)?(-n):n);
    else SetDlgItemText(Control, _T(""));

    return false;
}

LRESULT CMainDlg::OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addimages"));
    return 0;
}

LRESULT CMainDlg::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    MENUITEMINFO mi;
    HWND hwnd = reinterpret_cast<HWND>(wParam);  
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

    CMenu menu;
    LV_HITTESTINFO hti;
    hti.pt = ClientPoint;
    ThumbsView.HitTest(&hti);

    if(hti.iItem<0) // 
    {
        CMenu FolderMenu;
        FolderMenu.LoadMenu(IDR_CONTEXTMENU2);
        CMenu sub = FolderMenu.GetSubMenu(0);
        bool IsClipboard=    WizardDlg->IsClipboardDataAvailable();
        sub.EnableMenuItem(IDC_PASTE, (IsClipboard) ? MF_ENABLED : MF_GRAYED);
        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_TYPE;
        mi.fType = MFT_STRING;
        mi.dwTypeData = const_cast<LPWSTR>(TR("Add Images"));
        sub.SetMenuItemInfo(IDC_ADDIMAGES, false, &mi);
        mi.dwTypeData = const_cast<LPWSTR>(TR("Add Files"));
        sub.SetMenuItemInfo(IDM_ADDFILES, false, &mi);
        mi.dwTypeData = const_cast<LPWSTR>(TR("Add folder"));
        sub.SetMenuItemInfo(IDM_ADDFOLDER, false, &mi);
        TCHAR buf[MAX_PATH];
        lstrcpy(buf, TR("Paste"));
        lstrcat(buf, _T("\t"));
        lstrcat(buf,Settings.Hotkeys.getByFunc("paste").localKey.toString());
        mi.dwTypeData = buf;
        sub.SetMenuItemInfo(IDC_PASTE, false, &mi);
        mi.dwTypeData = const_cast<LPWSTR>(TR("Remove all"));
        sub.SetMenuItemInfo(IDC_DELETEALL, false, &mi);
        sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
    }
    else
    {
        CString singleSelectedItem;
        bool isImage = false;
        if ( ThumbsView.GetSelectedCount() == 1 ) {
            singleSelectedItem = getSelectedFileName();
            isImage = IuCommonFunctions::IsImage(singleSelectedItem);
        }
        
        TCHAR buf[MAX_PATH];
        RECT r;
        GetClientRect(&r);
        menu.LoadMenu(IDR_CONTEXTMENU);

        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_TYPE;
        mi.fType = MFT_STRING;

        CMenu sub = menu.GetSubMenu(0);
        sub.SetMenuDefaultItem(0, true);

		CString fileName = FileList[hti.iItem].FileName;
		bool bIsImageFile = IuCommonFunctions::IsImage(fileName);
		bool isVideoFile = IsVideoFile(fileName);

        if(!bIsImageFile){
            sub.DeleteMenu(IDM_VIEW, MF_BYCOMMAND );
            sub.DeleteMenu(IDM_EDIT, MF_BYCOMMAND );
            sub.DeleteMenu(IDM_PRINT, MF_BYCOMMAND );
			sub.DeleteMenu(IDM_EDITINEXTERNALEDITOR, MF_BYCOMMAND);
            sub.DeleteMenu(IDM_SEARCHBYIMGITEM, MF_BYCOMMAND);
        }

		if (!isVideoFile){
			sub.DeleteMenu(IDM_EXTRACTFRAMES, MF_BYCOMMAND);
		}

        mi.dwTypeData = const_cast<LPWSTR>(TR("View"));
        sub.SetMenuItemInfo(IDM_VIEW, false, &mi);

        mi.dwTypeData = const_cast<LPWSTR>(TR("Show in default viewer"));
        sub.SetMenuItemInfo(IDM_OPENINDEFAULTVIEWER, false, &mi);

        if (bIsImageFile) {
            mi.dwTypeData = const_cast<LPWSTR>(TR("Print..."));
            sub.SetMenuItemInfo(IDM_PRINT, false, &mi);
        }
        mi.dwTypeData = const_cast<LPWSTR>(TR("Open in folder"));
        sub.SetMenuItemInfo(IDM_OPENINFOLDER, false, &mi);

        mi.dwTypeData = const_cast<LPWSTR>(TR("Save as..."));
        sub.SetMenuItemInfo(IDM_SAVEAS, false, &mi); 

        mi.dwTypeData = const_cast<LPWSTR>(TR("Extract frames"));
        sub.SetMenuItemInfo(IDM_EXTRACTFRAMES, false, &mi);
        

        CString menuItemTitle = ( isImage ?  TR("Copy image") : TR("Copy") ) + CString(_T("\tCtrl+C"));
        lstrcpy(buf, menuItemTitle);
        mi.dwTypeData  = buf;
        sub.SetMenuItemInfo(IDM_COPYFILETOCLIPBOARD, false, &mi);

        CString label = TR("Copy full path") + CString(_T("\tCtrl+Shift+C"));
        mi.dwTypeData = const_cast<LPWSTR>((LPCTSTR)label);
		sub.SetMenuItemInfo(IDM_COPYFILEPATH, false, &mi);

        mi.dwTypeData = const_cast<LPWSTR>(TR("Remove"));
        sub.SetMenuItemInfo(IDM_DELETE, false, &mi);
        mi.dwTypeData = const_cast<LPWSTR>(TR("Properties"));
        sub.SetMenuItemInfo(IDC_PROPERTIES, false, &mi);
        
        mi.dwTypeData = const_cast<LPWSTR>(TR("Edit"));
        sub.SetMenuItemInfo(IDM_EDIT, false, &mi);

        mi.dwTypeData = const_cast<LPWSTR>(TR("Edit"));
        sub.SetMenuItemInfo(IDM_EDIT, false, &mi);

        mi.dwTypeData = const_cast<LPWSTR>(TR("Open in external editor"));
        sub.SetMenuItemInfo(IDM_EDITINEXTERNALEDITOR, false, &mi);
        
        sub.EnableMenuItem(IDM_EDIT, bIsImageFile?MF_ENABLED    :MF_GRAYED    );

        if (isImage) {
            CMenu subMenu;
            subMenu.CreatePopupMenu();

            subMenu.AppendMenu(MFT_STRING, IDM_COPYFILEASDATAURI, TR_CONST("data:URI"));
            subMenu.AppendMenu(MFT_STRING, IDM_COPYFILEASDATAURIHTML, TR_CONST("data:URI (HTML)"));

            MENUITEMINFO miiNew = { 0 };
            miiNew.cbSize = sizeof(MENUITEMINFO);
            miiNew.fMask = MIIM_SUBMENU | MIIM_STRING;
            miiNew.hSubMenu = subMenu.Detach();   // Detach() to keep the pop-up menu alive
            miiNew.dwTypeData = TR_CONST("Copy &as...");
            sub.InsertMenuItem(IDM_DELETE, false, &miiNew);

            mi.dwTypeData = const_cast<LPWSTR>(TR("Search by image"));
            sub.SetMenuItemInfo(IDM_SEARCHBYIMGITEM, false, &mi);
        }

        sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
    }
    return 0;
}

bool CMainDlg::AddToFileList(LPCTSTR FileName, const CString& virtualFileName, bool ensureVisible, Gdiplus::Image *Img)
{
    CFileListItem fl; //internal list item
    
    if(!FileName) return FALSE;

    if(!WinUtils::FileExists(FileName)) return FALSE;
    fl.selected = false;

    fl.FileName = FileName;

    if(virtualFileName.IsEmpty())
    fl.VirtualFileName = myExtractFileName(FileName);
    else
    fl.VirtualFileName = virtualFileName;
    
    int FileSize = MyGetFileSize(FileName);
    if(FileSize<-1) FileSize = 0;

    FileList.Add(fl);

    CString Buf;
    if(IuCommonFunctions::IsImage(FileName))
    Buf = WinUtils::GetOnlyFileName(FileName );
    else Buf = myExtractFileName(FileName);
    if(FileName) 
        ThumbsView.AddImage(fl.FileName, fl.VirtualFileName, ensureVisible, Img);
        
    EnableNext(FileList.GetCount()>0);
    listChanged_ = true;
    return TRUE;
}

// Выбран пункт меню или нажата кнопка удаления файла из списка
LRESULT CMainDlg::OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ThumbsView.DeleteSelected();
    return 0;
}

LRESULT CMainDlg::OnEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nCurItem;

    if ((nCurItem=ThumbsView.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))<0)
        return FALSE;

    LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
    if(!FileName) return FALSE;
    using namespace ImageEditor;
    ImageEditorConfigurationProvider configProvider;
    ImageEditor::ImageEditorWindow imageEditor(FileName, &configProvider);
    imageEditor.showUploadButton(false);
    imageEditor.showAddToWizardButton(false);
    
    /*ImageEditorWindow::DialogResult dr = */imageEditor.DoModal(WizardDlg->m_hWnd, ImageEditorWindow::wdmAuto);
    
    ThumbsView.OutDateThumb(nCurItem);
    ThumbsView.UpdateOutdated();
    
    return 0;
}

LRESULT CMainDlg::OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("screenshotdlg"));
    return 0;
}

bool CMainDlg::OnShow()
{
    ShowPrev();
    ShowNext();
    EnablePrev();
    EnableExit();
    EnableNext(FileList.GetCount()>0);
    ThumbsView.SetFocus();
    ThumbsView.LoadThumbnails();
    return true;
}

LRESULT CMainDlg::OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
    NM_LISTVIEW * pnmv = reinterpret_cast <NM_LISTVIEW *>  (pNMHDR);  
    if(!pnmv) return 0;

    FileList.RemoveAt(pnmv->iItem);

    EnableNext(FileList.GetCount()>0);
    bHandled = false;
    //UpdateStatusLabel();
    return 0;
}

LRESULT CMainDlg::OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ThumbsView.DeleteSelected();
    UpdateStatusLabel();
    return 0;
}

bool CMainDlg::OnHide()
{
    ThumbsView.StopAndWait();
    if(IsRunning())
    {
        WaitThreadStop.SetEvent(); // Sending stop message destinated for child thread
        WaitForThread(3500);
    }
    return false;
}

BOOL CMainDlg::FileProp(){
    CComPtr<IShellFolder> desktop; // namespace root for parsing the path
    HRESULT hr = SHGetDesktopFolder(&desktop);
   
    if (!SUCCEEDED(hr)) {
        return false;
    }

    int nCurItem = -1;

    std::vector<LPITEMIDLIST> list;
    while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL | LVNI_SELECTED)) >= 0) {
        LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
        if (!FileName) {
            continue;
        }
       
        PIDLIST_RELATIVE newPIdL;
        hr = desktop->ParseDisplayName(m_hWnd, 0, const_cast<LPWSTR>(FileName), 0, &newPIdL, 0);
        if (SUCCEEDED(hr)) {
            list.push_back(newPIdL);
            /*
            CComPtr<IShellFolder> folder;
            if (SUCCEEDED(mycomputer->BindToObject(folderPidl, 0, IID_IShellFolder, (void**)&folder))) {
                //ILAppend()
                PIDLIST_RELATIVE newPIdL = NULL;
                ULONG dwAttributes = SFGAO_FILESYSTEM | SHCIDS_ALLFIELDS | SFGAO_HASPROPSHEET;
                ULONG eaten = 0;
                CString onlyFileName = WinUtils::myExtractFileName(FileName);
                hr = folder->ParseDisplayName(m_hWnd, 0, (LPWSTR)(LPCTSTR)onlyFileName, 0, &newPIdL,0);
                if (SUCCEEDED(hr)) {
                    

                }
            }*/
        }
    }

    if (!list.empty()) {
        CShellPidl::MultiFileProperties(m_hWnd, desktop, const_cast<LPCITEMIDLIST*>(&list[0]), list.size());

        for (auto& pidl : list) {
            SHFree(pidl);
        }
    }
    return TRUE;
}

LRESULT CMainDlg::OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    FileProp();
    return 0;
}

LRESULT CMainDlg::OnEditExternal(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nCurItem;

    if ((nCurItem=ThumbsView.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))<0)
            return FALSE;
    
    LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
    if(!FileName) return FALSE;
    // Edit this bullshit
    CString EditorCmd = Settings.ImageEditorPath;
    CString EditorCmdLine ;
    EditorCmd.Replace(_T("%1"), FileName);
    EditorCmdLine = WinUtils::ExpandEnvironmentStrings(EditorCmd);
    
    TCHAR FilePathBuffer[256];
    ExtractFilePath(FileName, FilePathBuffer);

    CCmdLine EditorLine(EditorCmdLine);

    SHELLEXECUTEINFO Sei;
    ZeroMemory(&Sei, sizeof(Sei));
    Sei.cbSize = sizeof(Sei);
    Sei.fMask  = SEE_MASK_NOCLOSEPROCESS;
    Sei.hwnd = m_hWnd;
    Sei.lpVerb = _T("open");

    Sei.lpFile = EditorLine.ModuleName();
    Sei.lpParameters = EditorLine.OnlyParams();
    Sei.nShow = SW_SHOW;

    if (!::ShellExecuteEx(&Sei)) {
        LOG(ERROR) << "Opening external editor failed." << std::endl << "Reason: " << WinUtils::ErrorCodeToString(GetLastError());
        return 0;
    }

    if(Sei.hProcess)
    {
        if(IsRunning())
        {
            WaitThreadStop.SetEvent();
            WaitForThread(9999);
        }
        ThumbsView.OutDateThumb(nCurItem);
        m_EditorProcess = Sei.hProcess;
        Start();
    }
    return 0;
}

LRESULT CMainDlg::OnImageView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ThumbsView.ViewSelectedImage();
    return 0;
}

LRESULT CMainDlg::OnMenuItemPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("paste"));
    return 0;
}

LRESULT CMainDlg::OnAddFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addfolder"));
    return 0;
}
    
LRESULT CMainDlg::OnBnClickedDeleteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ThumbsView.MyDeleteAllItems();
    UpdateStatusLabel();
    return 0;
}

DWORD CMainDlg::Run()
{
    HANDLE Events[2];
    Events[0] = m_EditorProcess;
    Events[1] = WaitThreadStop.m_hEvent;
    WaitForMultipleObjects(2, Events, FALSE, INFINITE);
    ThumbsView.UpdateOutdated();
    WaitThreadStop.ResetEvent();
    return 0;
}

// Executing Windows Explorer; file will be highlighted
LRESULT CMainDlg::OnOpenInFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nCurItem = -1;
    PIDLIST_ABSOLUTE folderPidl = nullptr;

    CComPtr<IShellFolder> desktop; // namespace root for parsing the path
    HRESULT hr = SHGetDesktopFolder(&desktop);

    if (!SUCCEEDED(hr)) {
        return 0;
    }

    std::vector<LPCITEMIDLIST> list;
    while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL | LVNI_SELECTED)) >= 0) {
        LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
        if (!FileName) {
            continue;
        }

        PIDLIST_RELATIVE newPIdL;
        hr = desktop->ParseDisplayName(m_hWnd, nullptr, const_cast<LPWSTR>(FileName), nullptr, &newPIdL, nullptr);
        if (SUCCEEDED(hr)) {
            LPCITEMIDLIST relativePidl = nullptr;
            CComPtr<IShellFolder> folder;

            hr = SHBindToParent(newPIdL, IID_IShellFolder, reinterpret_cast<void**>(&folder), &relativePidl);
            if (SUCCEEDED(hr)) {
                list.push_back(relativePidl);
                if (!folderPidl) {
                    CComQIPtr<IPersistFolder2> persistFolder(folder);
                    if (persistFolder) {
                        persistFolder->GetCurFolder(&folderPidl);
                    }
                }
            }
        }
    }
    if (!list.empty() && folderPidl != nullptr) {
        // Will be opened the parent folder of the first file.
        // Files laying in another folders will be ignored.
        hr = SHOpenFolderAndSelectItems(folderPidl, list.size(), &list[0], 0);
        if (!SUCCEEDED(hr)) {
            LOG(ERROR) << "Unable to open folder in shell, error code=" << hr;
        }
        for (auto& pidl : list) {
            SHFree(const_cast<LPITEMIDLIST>(pidl));
        }
    }
    
    return 0;
}

LRESULT CMainDlg::OnOpenInDefaultViewer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    CString fileName = getSelectedFileName();
    if (!fileName.IsEmpty()) {
        SHELLEXECUTEINFO TempInfo = { 0 };
        TCHAR filePath[1024];
        WinUtils::ExtractFilePath(fileName, filePath);
        TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
        TempInfo.hwnd = m_hWnd;
        TempInfo.lpVerb = _T("open");
        TempInfo.lpFile = fileName;
        TempInfo.lpDirectory = filePath;
        TempInfo.nShow = SW_NORMAL;

        if (!::ShellExecuteEx(&TempInfo)) {
            DWORD lastError = GetLastError();
            if (lastError != ERROR_CANCELLED) { // The operation was canceled by the user.
                LOG(ERROR) << "Opening default application failed." << std::endl << "Reason: " << WinUtils::ErrorCodeToString(lastError);
            }
                    
            return 0;
        }
    }
    return 0;
}

LRESULT CMainDlg::OnAddFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    WizardDlg->executeFunc(_T("addfiles"));
    return 0;
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) {
    if (hotkeys_ && TranslateAccelerator(m_hWnd, hotkeys_, pMsg)) {
        return TRUE;
    }
    return FALSE;
}

CString CMainDlg::getSelectedFileName() {
    int nCurItem;

    if ((nCurItem = ThumbsView.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))<0)
        return CString();

    LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
    if ( !FileName ) {
        return CString();
    }

    return FileName;
}

int CMainDlg::getSelectedFiles(std::vector<CString>& selectedFiles) {
    int nCurItem = -1;

    while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL | LVNI_SELECTED)) >= 0) {
        LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
        if (!FileName) {
            continue;
        }
        selectedFiles.push_back(FileName);

    }
    return selectedFiles.size();
}

LRESULT CMainDlg::OnCopyFileToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    int nCurItem = -1;

    std::vector<LPCTSTR> selectedFiles;
    while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL|LVNI_SELECTED)) >= 0 ) {
        LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
        if ( !FileName ) {
            continue;
        }
        selectedFiles.push_back( FileName );

    }
    if ( selectedFiles.empty() ) {
        return FALSE;
    }

    if ( selectedFiles.size() == 1) {
        SystemUtils::CopyFileAndImageToClipboard(selectedFiles[0]);
    } else {
        SystemUtils::CopyFilesToClipboard(selectedFiles);
    }

    return 0;
}

LRESULT CMainDlg::OnSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    int nCurItem = -1;

    std::deque<CString> selectedFiles;
    while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL|LVNI_SELECTED)) >= 0 ) {
        LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
        if ( !FileName ) {
            continue;
        }
        selectedFiles.push_back( FileName );
    }
    if ( selectedFiles.empty() ) {
        return FALSE;
    }

    if ( selectedFiles.size() == 1 ) {
        TCHAR Buf[MAX_PATH*4];
        CString FileName = selectedFiles[0];
        CString fileExt = WinUtils::GetFileExt(FileName);
        GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2,
            TR("Files")+CString(" *.")+fileExt, CString(_T("*."))+fileExt,
            TR("All files"),_T("*.*"));
        CFileDialog fd(false, fileExt, FileName,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,Buf,m_hWnd);
        if(fd.DoModal()!=IDOK || !fd.m_szFileName[0]) return 0;

        if (!CopyFile(FileName, fd.m_szFileName, false)) {
            MessageBox(TR("Cannot copy file: ")+WinUtils::GetLastErrorAsString(), APPNAME, MB_ICONERROR);
        }
    } else {
        CNewStyleFolderDialog dlg(m_hWnd, CString(), CString());
        if (dlg.DoModal(m_hWnd) == IDOK) {
            CString newPath = dlg.GetFolderPath();
            if (!newPath.IsEmpty()) {
                size_t fileCount = selectedFiles.size();
                for (size_t i = 0; i < fileCount; i++) {
                    if (!CopyFile(selectedFiles[i], newPath + _T("\\") + WinUtils::myExtractFileName(selectedFiles[i]), false)) {
                        LOG(ERROR) << TR("Cannot copy file ")<< selectedFiles[i] << "\r\n" << WinUtils::GetLastErrorAsString();
                    }
                }
            }
        }    
    }

    return 0;
}

LRESULT CMainDlg::OnCopyFileAsDataUri(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    CString fileName = getSelectedFileName();
    if (!fileName.IsEmpty()) {
        if (!CopyFileToClipboardInDataUriFormat(fileName, 0, 85, false)) {
            MessageBox(_T("Failed to copy file to clipboard"), APPNAME, MB_ICONERROR);
        }
    }
    return 0;
}

LRESULT CMainDlg::OnCopyFileAsDataUriHtml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    CString fileName = getSelectedFileName();
    if (!fileName.IsEmpty()) {
        if (!CopyFileToClipboardInDataUriFormat(fileName, 0, 85, true)) {
            MessageBox(_T("Failed to copy file to clipboard"), APPNAME, MB_ICONERROR);
        }
    }
    return 0;
}

LRESULT CMainDlg::OnCopyFilePath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nCurItem = -1;
    CString result; 
    while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL | LVNI_SELECTED)) >= 0) {
        LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
        if (!FileName) {
            continue;
        }
        if (!result.IsEmpty()) {
            result += _T("\r\n");
        }
        result += FileName;

    }

    if (!result.IsEmpty()) {
        if (!WinUtils::CopyTextToClipboard(result)) {
			MessageBox(_T("Failed to copy file to clipboard"), APPNAME, MB_ICONERROR);
		}
	}
	return 0;
}

LRESULT CMainDlg::OnSearchByImage(WORD, WORD, HWND, BOOL&) {
    CString fileName = getSelectedFileName();
    if (!fileName.IsEmpty()) {
        CSearchByImageDlg dlg(fileName);
        dlg.DoModal(m_hWnd);
    }
    return 0;
}

LRESULT CMainDlg::OnExtractFramesFromSelectedFile(WORD, WORD, HWND, BOOL&) {
	CString fileName = getSelectedFileName();
	if (!fileName.IsEmpty()) {
		WizardDlg->importVideoFile(fileName, 2);
	}
	return 0;
}

LRESULT CMainDlg::OnTimer(UINT, WPARAM wParam, LPARAM, BOOL&) {
    if (wParam == kStatusTimer && listChanged_) {
        UpdateStatusLabel();
    }
    return 0;
}

void CMainDlg::UpdateStatusLabel() {
    int selectedItemsCount = ThumbsView.GetSelectedCount();
    int totalCount = ThumbsView.GetItemCount();
    std::string statusText;
    if (selectedItemsCount) {
        statusText = str(boost::format("%1% files selected/%2% files total") % selectedItemsCount % totalCount);
    } else {
        statusText = str(boost::format("%d files") % totalCount);
    }
    SetDlgItemText(IDC_STATUSLABEL, U2W(statusText));
    listChanged_ = false;
}

void CMainDlg::ArgvQuote(const std::wstring& Argument, std::wstring& CommandLine, bool Force){
    if (Force == false &&
        Argument.empty() == false &&
        Argument.find_first_of(L" \t\n\v\"") == Argument.npos) {
        CommandLine.append(Argument);
    } else {
        CommandLine.push_back(L'"');

        for (auto It = Argument.begin();; ++It) {
            unsigned NumberBackslashes = 0;

            while (It != Argument.end() && *It == L'\\') {
                ++It;
                ++NumberBackslashes;
            }

            if (It == Argument.end()) {
                // Escape all backslashes, but let the terminating
                // double quotation mark we add below be interpreted
                // as a metacharacter.
                CommandLine.append(NumberBackslashes * 2, L'\\');
                break;
            } else if (*It == L'"') {
                //
                // Escape all backslashes and the following
                // double quotation mark.
                //
                CommandLine.append(NumberBackslashes * 2 + 1, L'\\');
                CommandLine.push_back(*It);
            } else {
                //
                // Backslashes aren't special here.
                //
                CommandLine.append(NumberBackslashes, L'\\');
                CommandLine.push_back(*It);
            }
        }

        CommandLine.push_back(L'"');
    }
    CommandLine.push_back(L' ');
}

LRESULT CMainDlg::OnPrintImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    std::vector<CString> selectedFiles;
    if (getSelectedFiles(selectedFiles)) {
        CString fileName = selectedFiles[0];
        bool isImageFile = IuCommonFunctions::IsImage(fileName);
        if (isImageFile) {
            WinUtils::DisplaySystemPrintDialogForImage(selectedFiles, m_hWnd);
        }
        /*std::wstring cmdLine;
        /*SHELLEXECUTEINFO TempInfo = { 0 };
        TCHAR filePath[1024];
        WinUtils::ExtractFilePath(selectedFiles[0], filePath);
        /*TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
        TempInfo.hwnd = m_hWnd;
        TempInfo.lpVerb = _T("open");
      
        TempInfo.lpDirectory = filePath;
        TempInfo.nShow = SW_NORMAL;
        
        for (const auto& file : selectedFiles) {
            ArgvQuote(static_cast<LPCTSTR>(file), cmdLine, true);
        }
        HINSTANCE hinst = ShellExecute(m_hWnd, _T("print"), selectedFiles[0], /*cmdLine.c_str()*0, filePath, SW_SHOWNORMAL);
        
        if (reinterpret_cast<int>(hinst) <= 32) {
            TCHAR buffer[256];
            wsprintf(buffer, _T("ShellExecute failed. Error code=%d"), reinterpret_cast<int>(hinst));
        }*/
    }
    return 0;
}