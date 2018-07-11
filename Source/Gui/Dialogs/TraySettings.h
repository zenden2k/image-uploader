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

#ifndef TRAYSETTINGS_H
#define TRAYSETTINGS_H

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
    CTrayActions();
    void AddTrayAction(CString text, DWORD id)
    {
        TrayItem item;
        item.text = text;
        item.commandId = id;
        Add(item);
    }
};


class CTraySettingsPage : 
    public CDialogImpl<CTraySettingsPage>, public CSettingsPage    
{
public:
    CTraySettingsPage();
    ~CTraySettingsPage();
    enum { IDD = IDD_TRAYSETTINGSPAGE};

    BEGIN_MSG_MAP(CTraySettingsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER_EX(IDC_SHOWTRAYICON, BN_CLICKED, OnShowTrayIconBnClicked)
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    bool Apply();
    LRESULT OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
};



#endif // TRAYSETTINGS_H