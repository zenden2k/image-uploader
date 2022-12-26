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

class CIconButton;
class UploadEngineManager;
// CServerProfileGroupSelectDialog
class CServerSelectorControl;
class CServerProfileGroupSelectDialog : public CDialogImpl<CServerProfileGroupSelectDialog>, public CDialogResize<CServerProfileGroupSelectDialog>
{
    public:
        explicit CServerProfileGroupSelectDialog(UploadEngineManager* uploadEngineManager, ServerProfileGroup group);
        ~CServerProfileGroupSelectDialog();
        enum { IDD = IDD_SERVERPROFILESELECT };
        const ServerProfileGroup& serverProfileGroup() const;
    protected:
        BEGIN_MSG_MAP(CServerProfileGroupSelectDialog)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_ADDBUTTON, BN_CLICKED, OnClickedAdd)
            CHAIN_MSG_MAP(CDialogResize<CServerProfileGroupSelectDialog>)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CMediaInfoDlg)
            DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y)  
        END_DLGRESIZE_MAP()

        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   
    private:
        ServerProfileGroup profileGroup_;
        std::vector<CServerSelectorControl*> serverSelectors_;
        std::vector<CIconButton*> deleteButtons_;
        UploadEngineManager* uploadEngineManager_;
        CIcon icon_, iconSmall_;
        CIcon hIconSmall;
        void addSelector(const ServerProfile& profile, HDC dc);
};


#endif