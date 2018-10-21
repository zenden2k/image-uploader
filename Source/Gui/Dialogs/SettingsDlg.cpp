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

#include "SettingsDlg.h"

#include "traysettings.h"
#include "hotkeysettings.h"
#include "ScreenshotSettingsPage.h"
#include "ThumbSettingsPage.h"
#include "generalsettings.h"
#include "uploadSettingsPage.h"
#include "VideoGrabberParams.h"
#include "IntegrationSettings.h"
#include "DefaultServersSettings.h"
#include "Gui/GuiTools.h"

// CSettingsDlg
CSettingsDlg::CSettingsDlg(int Page, UploadEngineManager* uploadEngineManager)
{
    CurPage = -1;
    PageToShow = Page;
    PrevPage = -1;
    ZeroMemory(Pages, sizeof(Pages));
    uploadEngineManager_ = uploadEngineManager;
    backgroundBrush_.CreateSysColorBrush(COLOR_BTNFACE);
}

CSettingsDlg::~CSettingsDlg()
{
    for(int i=0; i<SettingsPageCount; i++)
        if(Pages[i]) delete Pages[i];
}

LRESULT CSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) | WS_EX_LAYOUTRTL);  // test :))
    // center the dialog on the screen
    CenterWindow();
    saveStatusLabel_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_SAVESTATUSLABEL));

    HWND parent = GetParent();
    if(!parent || !::IsWindowVisible(parent))
    {
        SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & WS_EX_APPWINDOW);
    }
    m_SettingsPagesListBox.SubclassWindow(GetDlgItem(IDC_SETTINGSPAGESLIST));
    m_SettingsPagesListBox.AddString(TR("General"));
    m_SettingsPagesListBox.AddString(TR("Servers"));
    m_SettingsPagesListBox.AddString(TR("Images"));
    m_SettingsPagesListBox.AddString(TR("Thumbnails"));
    m_SettingsPagesListBox.AddString(TR("Screen Capture"));
    m_SettingsPagesListBox.AddString(TR("Video"));
    m_SettingsPagesListBox.AddString(TR("Uploading"));
    m_SettingsPagesListBox.AddString(TR("Integration"));
    m_SettingsPagesListBox.AddString(TR("Tray icon"));
    m_SettingsPagesListBox.AddString(TR("Hotkeys"));
    // set icons 

    hIcon = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    hIconSmall = GuiTools::LoadSmallIcon(IDR_MAINFRAME);
    
    SetIcon(hIcon, TRUE);
    SetIcon(hIconSmall, FALSE);

    TRC(IDOK, "OK");
    TRC(IDCANCEL, "Cancel");
    TRC(IDC_APPLY, "Apply");
    TRC(IDC_SAVESTATUSLABEL, "Settings have been saved.");
    SetWindowText(TR("Settings"));
    
    m_SettingsPagesListBox.SetCurSel(PageToShow);
    ShowPage(PageToShow);
    return 0;  // Let the system set the focus
}

LRESULT CSettingsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    for (int i = 0; i < SettingsPageCount; i++) {
        try {
            if (Pages[i] && !Pages[i]->Apply()) {
                ShowPage(i);
                return 0;
            }
        } catch (ValidationException& ex) {
            ShowPage(i);
            if (ex.errors_.size()) {
                MessageBox(ex.errors_[0].Message, TR("Error"), MB_ICONERROR);
                if (ex.errors_[0].Control) {
                    ::SetFocus(ex.errors_[0].Control);
                }
            }

            return 0;
            // If some tab cannot apply changes - do not close dialog
        }
    }
    Settings.SaveSettings();
    EndDialog(wID);
    return 0;
}

LRESULT CSettingsDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CSettingsDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}

LRESULT CSettingsDlg::OnTimer(UINT, WPARAM, LPARAM, BOOL&)
{
    GuiTools::ShowDialogItem(m_hWnd, IDC_SAVESTATUSLABEL, false);
    KillTimer(kStatusLabelTimer);
    return 0;
}

void CSettingsDlg::CloseDialog(int nVal)
{
    if(CurPage >= 0)
        Pages[CurPage]->OnHide();
    
    DestroyWindow();
    ::PostQuitMessage(nVal);
}

bool CSettingsDlg::ShowPage(int idPage)
{
    if(idPage< 0 || idPage> SettingsPageCount-1) return false;

    if(idPage == CurPage) return true;

    if(!Pages[idPage]) 
        CreatePage(idPage);

    if(Pages[idPage]) 
        ::ShowWindow(Pages[idPage]->PageWnd, SW_SHOW);

    if(CurPage != -1 && Pages[CurPage]) ::ShowWindow(Pages[CurPage]->PageWnd, SW_HIDE);
    CurPage = idPage;

    m_SettingsPagesListBox.SetCurSel(CurPage);
    /*if(CurPage !=    TabCtrl_GetCurSel(GetDlgItem(IDC_TABCONTROL)))
    TabCtrl_SetCurSel(GetDlgItem(IDC_TABCONTROL), CurPage);*/
    return true;
}

LRESULT CSettingsDlg::OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    for (int i = 0; i < SettingsPageCount; i++) {
        try {
            if (Pages[i] && !Pages[i]->Apply()) {
                ShowPage(i);
                return 0;
            }
        } catch (ValidationException& ex) {
            ShowPage(i);
            if (ex.errors_.size()) {
                MessageBox(ex.errors_[0].Message, TR("Error"), MB_ICONERROR);
                if (ex.errors_[0].Control) {
                    ::SetFocus(ex.errors_[0].Control);
                }
            }

            return 0;
            // If some tab cannot apply changes - do not close dialog
        }
    }
    GuiTools::ShowDialogItem(m_hWnd, IDC_SAVESTATUSLABEL, true);
    SetTimer(kStatusLabelTimer, 3000);
    Settings.SaveSettings();
    return 0;
}

bool CSettingsDlg::CreatePage(int PageID)
{
    RECT rc = {150,3,636,400};

    if(PageID == spGeneral)
    {
        CGeneralSettings *dlg = new CGeneralSettings();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd=dlg->m_hWnd;
    }
    else if(PageID == spServers)
    {
        CDefaultServersSettings *dlg = new CDefaultServersSettings(uploadEngineManager_);
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd = dlg->m_hWnd;
    }
    else if(PageID == spImages)
    {
        CLogoSettings *dlg = new CLogoSettings();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd = dlg->m_hWnd;
    }

    else if(PageID == spThumbnails)
    {
        CThumbSettingsPage *dlg = new CThumbSettingsPage();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd = dlg->m_hWnd;
    }
    else if(PageID==spScreenshot)
    {
        CScreenshotSettingsPagePage *dlg = new CScreenshotSettingsPagePage();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd=dlg->m_hWnd;
        
    }

    else if(PageID == spVideo)
    {
        CVideoGrabberParams *dlg = new CVideoGrabberParams();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd=dlg->m_hWnd;
    }

     else if(PageID==spUploading)
    {
        CUploadSettingsPage *dlg = new CUploadSettingsPage();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd=dlg->m_hWnd;
    }
     else if(PageID==spIntegration)
     {
         CIntegrationSettings *dlg = new CIntegrationSettings(uploadEngineManager_);
         Pages[PageID]= dlg;
         dlg->Create(m_hWnd,rc);
         Pages[PageID]->PageWnd=dlg->m_hWnd;
     }

     else if(PageID==spTrayIcon)
    {
        CTraySettingsPage *dlg = new CTraySettingsPage();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd=dlg->m_hWnd;
    }
    else if(PageID==spHotkeys)
    {
        CHotkeySettingsPage *dlg = new CHotkeySettingsPage();
        Pages[PageID]= dlg;
        dlg->Create(m_hWnd,rc);
        Pages[PageID]->PageWnd=dlg->m_hWnd;
        
    }    

    WINDOWPLACEMENT wp;
    ::GetWindowPlacement(GetDlgItem(IDC_TABCONTROL), &wp);
    TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &wp.rcNormalPosition);     
    ::SetWindowPos(Pages[PageID]->PageWnd, 0, wp.rcNormalPosition.left, wp.rcNormalPosition.top, -wp.rcNormalPosition.left+wp.rcNormalPosition.right,  -wp.rcNormalPosition.top+wp.rcNormalPosition.bottom, 0);

    Pages[PageID]->FixBackground();
    return true;
}

LRESULT CSettingsDlg::OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    int Index = TabCtrl_GetCurSel(GetDlgItem(idCtrl));
    ShowPage(Index);
    return 0;
}

LRESULT CSettingsDlg::OnSettingsPagesSelChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int iPageIndex = m_SettingsPagesListBox.GetCurSel();
    ShowPage(iPageIndex);

    return 0;
}

LRESULT CSettingsDlg::OnCtlColorStatic(HDC hdc, HWND hwndChild)
{
    if (hwndChild == GetDlgItem(IDC_SAVESTATUSLABEL)) {
        SetTextColor(hdc, RGB(0, 180, 0));
        SetBkMode(hdc, TRANSPARENT);
        return reinterpret_cast<LRESULT>(static_cast<HBRUSH>(backgroundBrush_));
    }
    return 0;
}   
