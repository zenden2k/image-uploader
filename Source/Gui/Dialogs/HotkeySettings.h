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
#ifndef HOTKEYSETTINGS_H
#define HOTKEYSETTINGS_H


#pragma once

#include "atlheaders.h"
#include <resource.h>       // main symbols
#include <atlcrack.h>
#include <atlctrls.h>
#include "settingspage.h"
#include <vector>

#define IDM_CLEARHOTKEY 10000
#define IDM_CLEARALLHOTKEYS (IDM_CLEARHOTKEY + 1)

#include "hotkeyeditor.h"

class CHotkeyItem;
class CHotkeyList: public std::vector<CHotkeyItem>
{
    public:
        CHotkeyList();
        CHotkeyList(const CHotkeyList&) = delete;
        bool m_bChanged;
        bool Changed();

        CHotkeyItem& getByFunc(const CString &func);
        void AddItem(CString name, CString func, DWORD commandId, bool setForegroundWindow = true, WORD Code = 0, WORD modif = 0);
        CHotkeyList& operator=( const CHotkeyList& );
        bool operator==( const CHotkeyList& );
        CString toString() const;
        bool DeSerialize(const CString &data);
        int getFuncIndex(const CString &func);
};

class CHotkeySettingsPage : 
    public CDialogImpl<CHotkeySettingsPage>, public CSettingsPage    
{
    public:
        CHotkeySettingsPage();
        ~CHotkeySettingsPage();
        bool Apply() override;
        enum { IDD = IDD_HOTKEYSETTINGSPAGE};
    
    protected:
         BEGIN_MSG_MAP(CHotkeySettingsPage)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)  
            COMMAND_HANDLER(IDC_EDITHOTKEY, BN_CLICKED, OnEditHotkeyBnClicked)
            COMMAND_HANDLER(IDM_CLEARHOTKEY,BN_CLICKED, OnClearHotkey)
            COMMAND_HANDLER(IDM_CLEARALLHOTKEYS,BN_CLICKED, OnClearAllHotkeys)
            NOTIFY_HANDLER_EX(IDC_HOTKEYLIST, NM_DBLCLK, OnHotkeylistNmDblclk)
            
         END_MSG_MAP()
         // Handler prototypes:
         //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
         //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
         //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClearHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClearAllHotkeys(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnEditHotkeyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

        LRESULT OnHotkeylistNmDblclk(LPNMHDR pnmh);
        void EditHotkey(int index);

        CListViewCtrl m_HotkeyList;
        CHotkeyList hotkeyList;
        CFont attentionLabelFont_;
};

#endif // HOTKEYSETTINGS_H


