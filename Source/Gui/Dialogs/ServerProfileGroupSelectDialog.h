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

#ifndef IU_GUI_DIALOGS_SERVERPROFILEGROUPSELECTDIALOG_H_
#define IU_GUI_DIALOGS_SERVERPROFILEGROUPSELECTDIALOG_H_

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "Core/Upload/ServerProfileGroup.h"
#include "atlscrl.h"
#include "Gui/Controls/ServerSelectorControl.h"

constexpr unsigned int IDC_DELETESERVER_FIRST_ID = 14000;
constexpr unsigned int IDC_DELETESERVER_LAST_ID = 15000;
constexpr unsigned int IDC_SCROLLCONTAINER_ID = 15001;

class CMyPanel : public CScrollWindowImpl<CMyPanel>
{
public:    
    typedef CScrollWindowImpl<CMyPanel> TBase;
    DECLARE_WND_CLASS_EX(NULL, 0, COLOR_BTNFACE)

    BEGIN_MSG_MAP(CMyPanel)
        COMMAND_RANGE_HANDLER(IDC_DELETESERVER_FIRST_ID, IDC_DELETESERVER_LAST_ID, OnClickedDelete)
        MESSAGE_HANDLER(WM_SERVERSELECTCONTROL_SERVERLIST_CHANGED, OnServerListChanged)
        CHAIN_MSG_MAP(CScrollWindowImpl<CMyPanel>)
    END_MSG_MAP()
    void setScrollDimensions(int width, int height);
    void DoPaint(CDCHandle dc);
private:
    LRESULT OnClickedDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CIconButton;
class UploadEngineManager;
// CServerProfileGroupSelectDialog
class CServerSelectorControl;
class CServerProfileGroupSelectDialog : public CDialogImpl<CServerProfileGroupSelectDialog>, public CDialogResize<CServerProfileGroupSelectDialog>
{
    public:
        explicit CServerProfileGroupSelectDialog(UploadEngineManager* uploadEngineManager, ServerProfileGroup group, int serverMask);
        ~CServerProfileGroupSelectDialog();
        enum { IDD = IDD_SERVERPROFILESELECT };

        
        const ServerProfileGroup& serverProfileGroup() const;
    protected:
        BEGIN_MSG_MAP(CServerProfileGroupSelectDialog)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_ADDBUTTON, BN_CLICKED, OnClickedAdd)
            COMMAND_RANGE_HANDLER(IDC_DELETESERVER_FIRST_ID, IDC_DELETESERVER_LAST_ID, OnClickedDelete)
            MESSAGE_HANDLER(WM_SERVERSELECTCONTROL_SERVERLIST_CHANGED, OnServerListChanged)
            CHAIN_MSG_MAP(CDialogResize<CServerProfileGroupSelectDialog>)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CServerProfileGroupSelectDialog)
            DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y)  
            DLGRESIZE_CONTROL(IDC_ADDBUTTON, DLSZ_MOVE_X)
            //DLGRESIZE_CONTROL(IDC_SCROLLCONTAINER_ID, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        END_DLGRESIZE_MAP()

        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    private:
        ServerProfileGroup profileGroup_;
        std::vector<std::unique_ptr<CServerSelectorControl>> serverSelectors_;
        std::vector<std::unique_ptr<CButton>> deleteButtons_;
        UploadEngineManager* uploadEngineManager_;
        CIcon icon_, iconSmall_;
        CIcon hIconSmall;
        CIcon deleteIcon_;
        CMyPanel panel_;
        int serverMask_;
        void addSelector(const ServerProfile& profile, HDC dc);
        void updateSelectorPos(size_t index, HDC dc);
        void updateScroll();
};


#endif
