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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Dialogs/SettingsPage.h"

class CGeneralSettings : public CDialogImpl<CGeneralSettings>, 
                          public CSettingsPage    
{
    public:
        enum { IDD = IDD_GENERALSETTINGS };

        virtual ~CGeneralSettings() = default;
        bool apply() override;

    protected:
        BEGIN_MSG_MAP(CGeneralSettings)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
            COMMAND_HANDLER(IDC_BROWSEBUTTON, BN_CLICKED, OnBnClickedBrowse)
        END_MSG_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

        CComboBox langListCombo_;
        CToolTipCtrl toolTipCtrl_;
};

#endif // GENERALSETTINGS_H

