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

#include "TraySettings.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/GuiTools.h"

LRESULT CTraySettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    DoDataExchange(FALSE);
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    TRC(IDC_SHOWTRAYICON, "Show tray icon");
    TRC(IDC_MOUSEREACTIONGROUP, "Mouse clicks handlers");
    TRC(IDC_LEFTBUTTONDOUBLECLICKLABEL, "Left button double-click");
    TRC(IDC_LEFTBUTTONCLICKLABEL, "Left button single click:");
    TRC(IDC_MIDDLEBUTTONCLICKLABEL, "Middle button click:");
    TRC(IDC_RIGHTBUTTONCLICKLABEL, "Right button click:");
    TRC(IDC_ONEINSTANCE, "Do not launch new program's instance from tray");
    TRC(IDC_AUTOSTARTUP, "Launch program on Windows startup");

    int leftDoubleClickIndex = 0, leftClickIndex = 0,
    middleClickIndex = 0, rightClickIndex = 0;
    int index = 0;
    for(const auto& hotkey: settings->Hotkeys) {
        leftButtonDblClickCombo_.AddString(hotkey.GetDisplayName()); 
        leftButtonClickCombo_.AddString(hotkey.GetDisplayName());
        middleButtonClickCombo_.AddString(hotkey.GetDisplayName());
        rightButtoClickCombo_.AddString(hotkey.GetDisplayName());

        if (hotkey.func == settings->TrayIconSettings.LeftDoubleClickCommandStr) {
            leftDoubleClickIndex = index;
        }
        if (hotkey.func == settings->TrayIconSettings.LeftClickCommandStr) {
            leftClickIndex = index;
        }
        if (hotkey.func == settings->TrayIconSettings.MiddleClickCommandStr) {
            middleClickIndex = index;
        }
        if (hotkey.func == settings->TrayIconSettings.RightClickCommandStr) {
            rightClickIndex = index;
        }
        indexToCommand_[index] = hotkey.func;
        index++;
    }

    SendDlgItemMessage(IDC_SHOWTRAYICON, BM_SETCHECK, settings->ShowTrayIcon);
    SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK, settings->AutoStartup);

    SendDlgItemMessage(IDC_ONEINSTANCE, BM_SETCHECK, settings->TrayIconSettings.DontLaunchCopy);
    OnShowTrayIconBnClicked(BN_CLICKED, IDC_SHOWTRAYICON, 0);

    leftButtonDblClickCombo_.SetCurSel(leftDoubleClickIndex);
    leftButtonClickCombo_.SetCurSel(leftClickIndex);
    middleButtonClickCombo_.SetCurSel(middleClickIndex);
    rightButtoClickCombo_.SetCurSel(rightClickIndex);
    
    return 1;  // Let the system set the focus
}

bool CTraySettingsPage::Apply() {
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->ShowTrayIcon_changed = settings->ShowTrayIcon;
    settings->ShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK)==BST_CHECKED;
    settings->ShowTrayIcon_changed ^= settings->ShowTrayIcon;


    settings->AutoStartup_changed = settings->AutoStartup;
    settings->AutoStartup = SendDlgItemMessage(IDC_AUTOSTARTUP, BM_GETCHECK)==BST_CHECKED;
    settings->AutoStartup_changed ^= settings->AutoStartup;

    //Settings.ExplorerContextMenu_changed = true;
    settings->TrayIconSettings.LeftDoubleClickCommandStr = getCommandByIndex(leftButtonDblClickCombo_.GetCurSel());
    settings->TrayIconSettings.LeftClickCommandStr = getCommandByIndex(leftButtonClickCombo_.GetCurSel());
    settings->TrayIconSettings.MiddleClickCommandStr = getCommandByIndex(middleButtonClickCombo_.GetCurSel());
    settings->TrayIconSettings.RightClickCommandStr = getCommandByIndex(rightButtoClickCombo_.GetCurSel());
    
    settings->TrayIconSettings.DontLaunchCopy = SendDlgItemMessage(IDC_ONEINSTANCE, BM_GETCHECK) == BST_CHECKED;
    
    return true;
}

LRESULT CTraySettingsPage::OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl) {
    bool bShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK) == BST_CHECKED;
    if (!bShowTrayIcon ) {
        SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK, FALSE);
    }
    GuiTools::EnableNextN(GetDlgItem(wID), 11, bShowTrayIcon);
    return 0;
}

CString CTraySettingsPage::getCommandByIndex(int index) const {
    auto it = indexToCommand_.find(index);
    if (it != indexToCommand_.end()) {
        return it->second;
    }
    return {};
}
