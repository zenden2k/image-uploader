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

#include "TraySettings.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
 
CTrayActions::CTrayActions()
    {
        /*AddTrayAction(TR("Нет действия"), 0);
        AddTrayAction(TR("Контекстное меню значка"), IDM_CONTEXTMENU);
        AddTrayAction(TR("Загрузить файлы"), IDM_UPLOADFILES);
        AddTrayAction(TR("Загрузить папку"), IDM_ADDFOLDER);
        AddTrayAction(TR("Импорт видео"),IDM_IMPORTVIDEO);
        AddTrayAction(TR("Скриншот"),IDM_SCREENSHOTDLG);
        AddTrayAction(TR("Скриншот выделенной области"),IDM_REGIONSCREENSHOT);
        AddTrayAction(TR("Скриншот всего экрана"),IDM_FULLSCREENSHOT);
        AddTrayAction(TR("Скриншот текущего окна"),IDM_WINDOWSCREENSHOT);
        AddTrayAction(TR("Показать окно программы"), IDM_SHOWAPPWINDOW);
        AddTrayAction(TR("Настройки"), IDM_SETTINGS);
        
        AddTrayAction(TR("Выход"), IDM_EXIT);*/
    
    }

// CTraySettingsPage
CTraySettingsPage::CTraySettingsPage()
{

}

CTraySettingsPage::~CTraySettingsPage()
{
}

LRESULT CTraySettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TRC(IDC_SHOWTRAYICON,"Показывать значок в трее");
    TRC(IDC_MOUSEREACTIONGROUP,"Реакция на действия мыши");
    TRC(IDC_LEFTBUTTONDOUBLECLICKLABEL,"Двойной клик левой кнопкой");
    TRC(IDC_LEFTBUTTONCLICKLABEL,"Одиночный клик левой кнопкой");
    TRC(IDC_MIDDLEBUTTONCLICKLABEL,"Клик средней кнопкой");
    TRC(IDC_RIGHTBUTTONCLICKLABEL,"Клик правой кнопкой");
    TRC(IDC_ONEINSTANCE,"Не запускать новые копии программы из трея");
    TRC(IDC_AUTOSTARTUP, "Запуск программы при старте Windows");
    //CTrayActions trayActions;
    for(size_t i=0; i< Settings.Hotkeys/*trayActions*/.size(); i++)
    {
        SendDlgItemMessage(IDC_LEFTBUTTONDOUBLECLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
        SendDlgItemMessage(IDC_LEFTBUTTONCLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
        SendDlgItemMessage(IDC_MIDDLEBUTTONCLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
    
        SendDlgItemMessage(IDC_RIGHTBUTTONCLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
    
    }

    SendDlgItemMessage(IDC_SHOWTRAYICON, BM_SETCHECK,Settings.ShowTrayIcon);
    SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK,Settings.AutoStartup);

    SendDlgItemMessage(IDC_ONEINSTANCE, BM_SETCHECK,Settings.TrayIconSettings.DontLaunchCopy);
    OnShowTrayIconBnClicked(BN_CLICKED, IDC_SHOWTRAYICON, 0);

    SendDlgItemMessage(IDC_LEFTBUTTONDOUBLECLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.LeftDoubleClickCommand);
    SendDlgItemMessage(IDC_LEFTBUTTONCLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.LeftClickCommand);
    SendDlgItemMessage(IDC_MIDDLEBUTTONCLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.MiddleClickCommand);
    SendDlgItemMessage(IDC_RIGHTBUTTONCLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.RightClickCommand);
    
    return 1;  // Let the system set the focus
}

bool CTraySettingsPage::Apply()
{
    Settings.ShowTrayIcon_changed = Settings.ShowTrayIcon;
    Settings.ShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK)==BST_CHECKED;
    Settings.ShowTrayIcon_changed ^= Settings.ShowTrayIcon;


    Settings.AutoStartup_changed = Settings.AutoStartup;
    Settings.AutoStartup = SendDlgItemMessage(IDC_AUTOSTARTUP, BM_GETCHECK)==BST_CHECKED;
    Settings.AutoStartup_changed ^= Settings.AutoStartup;

    //Settings.ExplorerContextMenu_changed = true;
    Settings.TrayIconSettings.LeftDoubleClickCommand = SendDlgItemMessage(IDC_LEFTBUTTONDOUBLECLICKCOMBO, CB_GETCURSEL);
    Settings.TrayIconSettings.LeftClickCommand = SendDlgItemMessage(IDC_LEFTBUTTONCLICKCOMBO, CB_GETCURSEL);
    Settings.TrayIconSettings.MiddleClickCommand = SendDlgItemMessage(IDC_MIDDLEBUTTONCLICKCOMBO, CB_GETCURSEL);
    Settings.TrayIconSettings.RightClickCommand = SendDlgItemMessage(IDC_RIGHTBUTTONCLICKCOMBO, CB_GETCURSEL);
    
    Settings.TrayIconSettings.DontLaunchCopy = SendDlgItemMessage(IDC_ONEINSTANCE, BM_GETCHECK);
    
    return true;
}
LRESULT CTraySettingsPage::OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    bool bShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK) == BST_CHECKED;
    if (!  bShowTrayIcon ) {
        SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK, FALSE);
    }
    GuiTools::EnableNextN(GetDlgItem(wID),11,bShowTrayIcon);
    return 0;
}
