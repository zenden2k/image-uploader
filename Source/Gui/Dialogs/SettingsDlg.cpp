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
#include "SettingsDlg.h"
#include "langselect.h"
#include "traysettings.h"
#include "hotkeysettings.h"
#include "ScreenshotSettingsPage.h"
#include "ThumbSettingsPage.h"
#include "generalsettings.h"
#include "uploadSettingsPage.h"
#include "VideoGrabberParams.h"
#include "IntegrationSettings.h"
#include "DefaultServersSettings.h"
#include <Gui/GuiTools.h>

// CSettingsDlg
CSettingsDlg::CSettingsDlg(int Page)
{
	CurPage = -1;
	PageToShow = Page;
	PrevPage = -1;
	ZeroMemory(Pages, sizeof(Pages));
}

CSettingsDlg::~CSettingsDlg()
{
	for(int i=0; i<SettingsPageCount; i++)
		if(Pages[i]) delete Pages[i];
}

LRESULT CSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// center the dialog on the screen
	CenterWindow();
	HWND parent = GetParent();
	if(!parent || !::IsWindowVisible(parent))
	{
		SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & WS_EX_APPWINDOW);
	}
	m_SettingsPagesListBox.SubclassWindow(GetDlgItem(IDC_SETTINGSPAGESLIST));
	m_SettingsPagesListBox.AddString(TR("Основные"));
	m_SettingsPagesListBox.AddString(TR("Серверы"));
	m_SettingsPagesListBox.AddString(TR("Изображения"));
	m_SettingsPagesListBox.AddString(TR("Миниатюры"));
	m_SettingsPagesListBox.AddString(TR("Снимок экрана"));
	m_SettingsPagesListBox.AddString(TR("Видео"));
	m_SettingsPagesListBox.AddString(TR("Загрузка"));
	m_SettingsPagesListBox.AddString(TR("Интеграция"));
	m_SettingsPagesListBox.AddString(TR("Трей"));
	m_SettingsPagesListBox.AddString(TR("Горячие клавиши"));
	// set icons 

	hIcon = GuiTools::LoadBigIcon(IDR_MAINFRAME);
	hIconSmall = GuiTools::LoadSmallIcon(IDR_MAINFRAME);
	
	SetIcon(hIcon, TRUE);
	SetIcon(hIconSmall, FALSE);

	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_APPLY, "Применить");
	SetWindowText(TR("Настройки"));
	
	m_SettingsPagesListBox.SetCurSel(PageToShow);
	ShowPage(PageToShow);
	return 0;  // Let the system set the focus
}

LRESULT CSettingsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for(int i=0; i<SettingsPageCount; i++)
	{
		if(Pages[i] && !Pages[i]->Apply()) 
		{
			ShowPage(i);
			return 0;
		}// If some tab cannot apply changes - do not close dialog
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
	/*if(CurPage !=	TabCtrl_GetCurSel(GetDlgItem(IDC_TABCONTROL)))
	TabCtrl_SetCurSel(GetDlgItem(IDC_TABCONTROL), CurPage);*/
	return true;
}

LRESULT CSettingsDlg::OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	for(int i=0; i<SettingsPageCount; i++)
		if(Pages[i] && !Pages[i]->Apply()) 
		{
			ShowPage(i);
			return 0;
		}
	
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
		CDefaultServersSettings *dlg = new CDefaultServersSettings();
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
		 CIntegrationSettings *dlg = new CIntegrationSettings();
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
	::SetWindowPos(Pages[PageID]->PageWnd,  0, wp.rcNormalPosition.left, wp.rcNormalPosition.top, -wp.rcNormalPosition.left+wp.rcNormalPosition.right,  -wp.rcNormalPosition.top+wp.rcNormalPosition.bottom, 0);

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
   
