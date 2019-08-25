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

#include "TraySettings.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/GuiTools.h"
 
CTrayActions::CTrayActions(){
}

// CTraySettingsPage
CTraySettingsPage::CTraySettingsPage() {

}

CTraySettingsPage::~CTraySettingsPage() {
}

LRESULT CTraySettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    DoDataExchange(FALSE);
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    TRC(IDC_SHOWTRAYICON, "Show tray icon");
    TRC(IDC_MOUSEREACTIONGROUP, "Mouse clicks handlers");
    TRC(IDC_LEFTBUTTONDOUBLECLICKLABEL, "Left button double-click");
    TRC(IDC_LEFTBUTTONCLICKLABEL, "Left button single click:");
    TRC(IDC_MIDDLEBUTTONCLICKLABEL, "Middle button click:");
    TRC(IDC_RIGHTBUTTONCLICKLABEL, "Right button click:");
    TRC(IDC_ONEINSTANCE, "Do not launch new program's instance from tray");
    TRC(IDC_AUTOSTARTUP, "Launch program on Windows startup");
    
    for(const auto& hotkey: Settings.Hotkeys) {
        leftButtonDblClickCombo_.AddString(hotkey.GetDisplayName());
        leftButtonClickCombo_.AddString(hotkey.GetDisplayName());
        middleButtonClickCombo_.AddString(hotkey.GetDisplayName());
        rightButtoClickCombo_.AddString(hotkey.GetDisplayName());
    }

    SendDlgItemMessage(IDC_SHOWTRAYICON, BM_SETCHECK,Settings.ShowTrayIcon);
    SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK,Settings.AutoStartup);

    SendDlgItemMessage(IDC_ONEINSTANCE, BM_SETCHECK,Settings.TrayIconSettings.DontLaunchCopy);
    OnShowTrayIconBnClicked(BN_CLICKED, IDC_SHOWTRAYICON, 0);

    leftButtonDblClickCombo_.SetCurSel(Settings.TrayIconSettings.LeftDoubleClickCommand);
    leftButtonClickCombo_.SetCurSel(Settings.TrayIconSettings.LeftClickCommand);
    middleButtonClickCombo_.SetCurSel(Settings.TrayIconSettings.MiddleClickCommand);
    rightButtoClickCombo_.SetCurSel(Settings.TrayIconSettings.RightClickCommand);
    
    return 1;  // Let the system set the focus
}

bool CTraySettingsPage::Apply() {
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    Settings.ShowTrayIcon_changed = Settings.ShowTrayIcon;
    Settings.ShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK)==BST_CHECKED;
    Settings.ShowTrayIcon_changed ^= Settings.ShowTrayIcon;


    Settings.AutoStartup_changed = Settings.AutoStartup;
    Settings.AutoStartup = SendDlgItemMessage(IDC_AUTOSTARTUP, BM_GETCHECK)==BST_CHECKED;
    Settings.AutoStartup_changed ^= Settings.AutoStartup;

    //Settings.ExplorerContextMenu_changed = true;
    Settings.TrayIconSettings.LeftDoubleClickCommand = leftButtonDblClickCombo_.GetCurSel();
    Settings.TrayIconSettings.LeftClickCommand = leftButtonClickCombo_.GetCurSel();
    Settings.TrayIconSettings.MiddleClickCommand = middleButtonClickCombo_.GetCurSel();
    Settings.TrayIconSettings.RightClickCommand = rightButtoClickCombo_.GetCurSel();
    
    Settings.TrayIconSettings.DontLaunchCopy = SendDlgItemMessage(IDC_ONEINSTANCE, BM_GETCHECK) == BST_CHECKED;
    
    return true;
}

LRESULT CTraySettingsPage::OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl) {
    bool bShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK) == BST_CHECKED;
    if (!bShowTrayIcon ) {
        SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK, FALSE);
    }
    GuiTools::EnableNextN(GetDlgItem(wID),11,bShowTrayIcon);
    return 0;
}
