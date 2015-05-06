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

#include "atlheaders.h"
#include "ScreenshotSettingsPage.h"
#include "Func/Common.h"
#include "LogWindow.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include "Gui/Components/NewStyleFolderDialog.h"

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}
#include "Func/myutils.h"
#include <Core/Scripting/API/HtmlDocumentPrivate_win.h>

// CScreenshotSettingsPagePage
CScreenshotSettingsPagePage::CScreenshotSettingsPagePage()
{

}

CScreenshotSettingsPagePage::~CScreenshotSettingsPagePage()
{
}

LRESULT CScreenshotSettingsPagePage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TRC(IDC_GROUPPARAMS, "Дополнительно");
    TRC(IDC_QUALITYLABEL, "Качество:");
    TRC(IDC_DELAYLABEL, "Задержка:");
    TRC(IDC_FORMATLABEL, "Формат:");
    TRC(IDC_SECLABEL, "сек");
    TRC(IDC_MSECLABEL, "мс");
    TRC(IDC_SCREENSHOTSFOLDERSELECT, "Обзор");
    TRC(IDC_SCREENSHOTFOLDERLABEL, "Папка для сохранения скриншотов:");
    TRC(IDC_SCREENSHOTFILENAMELABEL, "Формат имени файла");
    TRC(IDC_DELAYLABEL2, "Задержка при скрытии окон:");
    TRC(IDC_ALWAYSCOPYTOCLIPBOARD, "Всегда копировать в буфер обмена");
    TRC(IDC_SCREENSHOTSAVINGPARAMS, "Параметры сохранения снимков");
    TRC(IDC_FOREGROUNDWHENSHOOTING, "Выводить окно на передний план при выборе мышью");
    TRC(IDC_PARAMETERSHINTLABEL, "%y - год, %m - месяц, %d - день\n%h - час, %n - минута, %s - секунда\n %i - порядковый номер,\n%width% - ширина,  %height% - высота изображения");
    TRC(IDC_ADDSHADOW, "Добавлять тень окна");
    TRC(IDC_ALLOWALTTABINIMAGEEDITOR, "Разрешить Alt+Tab в полноэкранном редакторе");
    CString removeCornersText = TR("Удалять уголки у окна")+CString(_T(" (Windows Vista/7)"));
    SetDlgItemText(IDC_REMOVECORNERS, removeCornersText);
    TRC(IDC_REMOVEBACKGROUND, "Удалять фон окна");
    TRC(IDC_SHORTENURLFROMTRAYCHECKBOX, "Сокращать ссылки при быстрой загрузке (по горячей клавише)");
    TRC(IDC_AEROONLY, "Только для Aero (Windows Vista и новее)");
    TRC(IDC_USEOLDREGIONSCREENSHOTMETHOD, "Использовать старый способ выбора области экрана");
    
    SetDlgItemText(IDC_SCREENSHOTFILENAMEEDIT, Settings.ScreenshotSettings.FilenameTemplate);

    SetDlgItemText(IDC_SCREENSHOTFOLDEREDIT, Settings.ScreenshotSettings.Folder);
    SendDlgItemMessage(IDC_DELAYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)30, (short)0) );
    SendDlgItemMessage(IDC_QUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
    
    SendDlgItemMessage(IDC_FORMATLIST, CB_ADDSTRING, 0, (LPARAM)_T("JPEG"));
    SendDlgItemMessage(IDC_FORMATLIST, CB_ADDSTRING, 0, (LPARAM)_T("PNG"));
    SendDlgItemMessage(IDC_FOREGROUNDWHENSHOOTING, BM_SETCHECK,Settings.ScreenshotSettings.ShowForeground);
    SendDlgItemMessage(IDC_ALWAYSCOPYTOCLIPBOARD, BM_SETCHECK, Settings.ScreenshotSettings.CopyToClipboard);

    SendDlgItemMessage(IDC_REMOVECORNERS, BM_SETCHECK, Settings.ScreenshotSettings.RemoveCorners);
    SendDlgItemMessage(IDC_ADDSHADOW, BM_SETCHECK, Settings.ScreenshotSettings.AddShadow);
    SendDlgItemMessage(IDC_REMOVEBACKGROUND, BM_SETCHECK, Settings.ScreenshotSettings.RemoveBackground);

    GuiTools::SetCheck(m_hWnd, IDC_SHORTENURLFROMTRAYCHECKBOX, Settings.TrayIconSettings.ShortenLinks);
    GuiTools::SetCheck(m_hWnd, IDC_USEOLDREGIONSCREENSHOTMETHOD, Settings.ScreenshotSettings.UseOldRegionScreenshotMethod);

    int Quality, Delay, Format;
    Quality = Settings.ScreenshotSettings.Quality;
    Format = Settings.ScreenshotSettings.Format;
    Delay = Settings.ScreenshotSettings.Delay;

    if( Format < 0) Format = 0;
    if( Quality < 0) Quality = 85;
    if( Delay < 0 || Delay > 30) Delay = 2;

    SetDlgItemInt(IDC_QUALITYEDIT, Quality);
    SetDlgItemInt(IDC_DELAYEDIT, Delay);
    SetDlgItemInt(IDC_WINDOWHIDINGDELAY, Settings.ScreenshotSettings.WindowHidingDelay);
    SendDlgItemMessage(IDC_FORMATLIST, CB_SETCURSEL, Format, 0);
    GuiTools::SetCheck(m_hWnd, IDC_ALLOWALTTABINIMAGEEDITOR, Settings.ImageEditorSettings.AllowAltTab);
    bool isVista = IsVista();
    ::EnableWindow(GetDlgItem(IDC_AEROONLY), isVista);
    ::EnableWindow(GetDlgItem(IDC_REMOVECORNERS), isVista);
    ::EnableWindow(GetDlgItem(IDC_ADDSHADOW), isVista);
    ::EnableWindow(GetDlgItem(IDC_REMOVEBACKGROUND), isVista);
    return 1;  // Let the system set the focus
}

bool CScreenshotSettingsPagePage::Apply()
{
    CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_SCREENSHOTFILENAMEEDIT));
    if(!CheckFileName(fileName))
    {
        MessageBox(TR("Имя файла содержит запрещенные символы!"));
        ::SetFocus(GetDlgItem(IDC_SCREENSHOTFILENAMEEDIT));
        return false;
    }
    Settings.ScreenshotSettings.FilenameTemplate = fileName;
    
    Settings.ScreenshotSettings.Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
    Settings.ScreenshotSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
    Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);
    Settings.ScreenshotSettings.ShowForeground = SendDlgItemMessage(IDC_FOREGROUNDWHENSHOOTING, BM_GETCHECK) == BST_CHECKED;

    Settings.ScreenshotSettings.Folder = GuiTools::GetWindowText(GetDlgItem(IDC_SCREENSHOTFOLDEREDIT));
    Settings.ScreenshotSettings.Format = SendDlgItemMessage(IDC_FORMATLIST,CB_GETCURSEL,0,0);
    Settings.ScreenshotSettings.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
    Settings.ScreenshotSettings.Delay = GetDlgItemInt(IDC_DELAYEDIT);
    Settings.ScreenshotSettings.ShowForeground = SendDlgItemMessage(IDC_FOREGROUNDWHENSHOOTING, BM_GETCHECK) == BST_CHECKED;
    Settings.ScreenshotSettings.CopyToClipboard =  SendDlgItemMessage(IDC_ALWAYSCOPYTOCLIPBOARD, BM_GETCHECK) == BST_CHECKED;
    
    Settings.ScreenshotSettings.WindowHidingDelay = GetDlgItemInt(IDC_WINDOWHIDINGDELAY);

    Settings.ScreenshotSettings.RemoveCorners = SendDlgItemMessage(IDC_REMOVECORNERS, BM_GETCHECK)!=0;
    Settings.ScreenshotSettings.AddShadow = SendDlgItemMessage(IDC_ADDSHADOW, BM_GETCHECK)!=0;
    Settings.ScreenshotSettings.RemoveBackground = SendDlgItemMessage(IDC_REMOVEBACKGROUND, BM_GETCHECK)!=0;
    Settings.ScreenshotSettings.UseOldRegionScreenshotMethod = GuiTools::GetCheck(m_hWnd, IDC_USEOLDREGIONSCREENSHOTMETHOD );
    Settings.ImageEditorSettings.AllowAltTab = GuiTools::GetCheck(m_hWnd, IDC_ALLOWALTTABINIMAGEEDITOR);

     Settings.TrayIconSettings.ShortenLinks = GuiTools::GetCheck(m_hWnd, IDC_SHORTENURLFROMTRAYCHECKBOX);

    return true;
}

LRESULT CScreenshotSettingsPagePage::OnScreenshotsFolderSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CString path = GuiTools::GetWindowText(GetDlgItem(IDC_SCREENSHOTFOLDEREDIT));

    CNewStyleFolderDialog fd(m_hWnd, path, TR("Выбор папки") );
        
    if (fd.DoModal(m_hWnd) == IDOK)
    {
        SetDlgItemText(IDC_SCREENSHOTFOLDEREDIT, fd.GetFolderPath());
        return true;
    }
   
    return 0;
}
