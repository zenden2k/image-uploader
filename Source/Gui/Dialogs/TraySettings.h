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

#ifndef IU_GUI_DIALOGS_TRAYSETTINGS_H
#define IU_GUI_DIALOGS_TRAYSETTINGS_H

#pragma once
#include "atlheaders.h"
#include <atlcrack.h>
#include "resource.h"       // main symbols
#include "floatingwindow.h"
// CTraySettingsPage

struct TrayItem
{
    CString text;
    DWORD commandId;
};

class CTrayActions: public CAtlArray<TrayItem>
{
    public:
    void AddTrayAction(CString text, DWORD id)
    {
        TrayItem item;
        item.text = text;
        item.commandId = id;
        Add(item);
    }
};


class CTraySettingsPage : 
    public CDialogImpl<CTraySettingsPage>, 
    public CSettingsPage, 
    public CWinDataExchange <CTraySettingsPage>
{
public:
    enum { IDD = IDD_TRAYSETTINGSPAGE};

    BEGIN_MSG_MAP(CTraySettingsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER_EX(IDC_SHOWTRAYICON, BN_CLICKED, OnShowTrayIconBnClicked)
    END_MSG_MAP()

    BEGIN_DDX_MAP(CTraySettingsPage)
        DDX_CONTROL_HANDLE(IDC_LEFTBUTTONDOUBLECLICKCOMBO, leftButtonDblClickCombo_)
        DDX_CONTROL_HANDLE(IDC_LEFTBUTTONCLICKCOMBO, leftButtonClickCombo_)
        DDX_CONTROL_HANDLE(IDC_MIDDLEBUTTONCLICKCOMBO, middleButtonClickCombo_)
        DDX_CONTROL_HANDLE(IDC_RIGHTBUTTONCLICKCOMBO, rightButtoClickCombo_)
    END_DDX_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    bool apply() override;
    bool validate() override;
    LRESULT OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
protected:
    CComboBox leftButtonDblClickCombo_, leftButtonClickCombo_, middleButtonClickCombo_, rightButtoClickCombo_;
    std::map<int, CString> indexToCommand_;
    CString getCommandByIndex(int index) const;
};



#endif
