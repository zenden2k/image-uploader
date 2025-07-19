/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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
#ifndef ContextMenuItemDlg_H
#define ContextMenuItemDlg_H

#pragma once
#include <memory>
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/Upload/UploadEngine.h"
#include "Gui/Controls/ServerSelectorControl.h"
#include "Gui/Controls/DialogIndirect.h"

class ServerProfile;
class CServerSelectorControl;
// CContextMenuItemDlg

class UploadEngineManager;
class CContextMenuItemDlg : public CCustomDialogIndirectImpl<CContextMenuItemDlg>
{
    public:
        CContextMenuItemDlg(UploadEngineManager * uploadEngineManager);
        ~CContextMenuItemDlg();
        enum { IDD = IDD_CONTEXTMENUITEMDLG };
    protected:
        BEGIN_MSG_MAP(CContextMenuItemDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            MESSAGE_HANDLER(WM_SERVERSELECTCONTROL_CHANGE, OnServerSelectControlChanged)
            COMMAND_HANDLER(IDC_MENUITEMTITLEEDIT, EN_CHANGE, OnMenuItemTitleEditChange);

        END_MSG_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnServerSelectControlChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnMenuItemTitleEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        ServerProfile serverProfile() const;
        CString menuItemTitle() const;
protected:
    std::unique_ptr<CServerSelectorControl> imageServerSelector_;
    ServerProfile serverProfile_;
    bool titleEdited_;
    CString title_;
    UploadEngineManager * uploadEngineManager_;
    void generateTitle();
};

#endif // ContextMenuItemDlg_H


