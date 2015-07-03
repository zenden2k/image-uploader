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
#ifndef SERVERFOLDERSELECT_H
#define SERVERFOLDERSELECT_H


#pragma once
#include "atlheaders.h"
#include "Core/Upload/UploadEngineManager.h"
#include "resource.h"       // main symbols
#include "3rdpart/thread.h"
#include "Func/common.h"
#include <atlcrack.h>
#include "Gui/Controls/PictureExWnd.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Upload/FolderList.h"

// CServerFolderSelect
#define ID_EDITFOLDER 10001
#define ID_OPENINBROWSER 10002
#define ID_CREATENESTEDFOLDER 10003
#include <atomic>

class UploadEngineManager;
class ServerProfile;

class CServerFolderSelect : 
    public CDialogImpl<CServerFolderSelect>, 
    public CThreadImpl<CServerFolderSelect>, 
    public CDialogResize<CServerFolderSelect>    
{
    public:
        CServerFolderSelect(ServerProfile& serverProfile, UploadEngineManager * uploadEngineManager);
        ~CServerFolderSelect();
        enum { IDD = IDD_SERVERFOLDERSELECT};
    protected:
    BEGIN_MSG_MAP(CServerFolderSelect)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
          COMMAND_HANDLER(ID_OPENINBROWSER, BN_CLICKED, OnOpenInBrowser)
          COMMAND_HANDLER(ID_CREATENESTEDFOLDER, BN_CLICKED, OnCreateNestedFolder)      
          COMMAND_HANDLER(IDC_NEWFOLDERBUTTON, BN_CLICKED, OnBnClickedNewfolderbutton)
        COMMAND_ID_HANDLER(ID_EDITFOLDER, OnEditFolder)
        CHAIN_MSG_MAP(CDialogResize<CServerFolderSelect>)
     END_MSG_MAP()

         BEGIN_DLGRESIZE_MAP(CServerFolderSelect)
        DLGRESIZE_CONTROL(IDC_FOLDERLISTLABEL, DLSZ_SIZE_X)
        DLGRESIZE_CONTROL(IDC_FOLDERTREE, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X| DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_ANIMATIONSTATIC, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_NEWFOLDERBUTTON, DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCreateNestedFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    DWORD Run();
    void OnLoadFinished();
    CImageList m_PlaceSelectorImageList;
    CFolderItem m_SelectedFolder;
protected:
    enum FolderOperationType          
    {
        foGetFolders = 0, foCreateFolder, foModifyFolder 
    };
    CPictureExWnd m_wndAnimation;
    CScriptUploadEngine *runningScript_;
    std::mutex runningScriptMutex_;
    UploadEngineManager * uploadEngineManager_;
    CUploadEngineData *m_UploadEngine;
    CFolderList m_FolderList;
    CFolderItem m_newFolder;
    std::vector<std::string> m_accessTypeList;
    std::map<std::wstring,HTREEITEM> m_FolderMap;
    std::atomic<bool> stopSignal;
    CTreeViewCtrl m_FolderTree;
    ServerProfile& serverProfile_;
    //IU_PLUGIN_FolderItem * m_folderItems;
    FolderOperationType m_FolderOperationType;
    NetworkClient m_NetworkClient;
    void BlockWindow(bool Block);
    void NewFolder(const CString& parentFolderId);
    int progressCallback(NetworkClient* userData, double dltotal, double dlnow, double ultotal, double ulnow);
    //CString m_sNewFolderName, m_sNewFolderDescription;
public:
    
    LRESULT OnBnClickedNewfolderbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFolderlistLbnDblclk(WORD wNotifyCode, WORD wID, HWND hWndCtl);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEditFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    void CreateLoadingThread();
    void BuildFolderTree(std::vector<CFolderItem> &list,const CString& parentFolderId);
};

#endif // SERVERFOLDERSELECT_H