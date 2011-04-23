/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../atlheaders.h"
#include "SettingsDlg.h"
#include "langselect.h"
#include "traysettings.h"
#include "hotkeysettings.h"
#include "ScreenshotSettingsPage.h"
#include "ThumbSettingsPage.h"
#include "generalsettings.h"
#include "uploadSettingsPage.h"
#include "VideoGrabberParams.h"

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
		//MessageBox(L"OF COURSE WI SET");
	}
	m_SettingsPagesListBox.SubclassWindow(GetDlgItem(IDC_SETTINGSPAGESLIST));
	m_SettingsPagesListBox.AddString(TR("Основные"));
	m_SettingsPagesListBox.AddString(TR("Изображения"));
	m_SettingsPagesListBox.AddString(TR("Миниатюры"));
	m_SettingsPagesListBox.AddString(TR("Снимок экрана"));
	m_SettingsPagesListBox.AddString(TR("Видео"));
	m_SettingsPagesListBox.AddString(TR("Загрузка"));
	m_SettingsPagesListBox.AddString(TR("Трей"));
	m_SettingsPagesListBox.AddString(TR("Горячие клавиши"));
	// set icons

	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
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

	if(PageID == 0)
	{
		CGeneralSettings *dlg = new CGeneralSettings();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd=dlg->m_hWnd;
	}
	
	if(PageID == 1)
	{
		CLogoSettings *dlg = new CLogoSettings();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd = dlg->m_hWnd;
	}

	else if(PageID == 2)
	{
		CThumbSettingsPage *dlg = new CThumbSettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd = dlg->m_hWnd;
	}
	else if(PageID==3)
	{
		CScreenshotSettingsPagePage *dlg = new CScreenshotSettingsPagePage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd=dlg->m_hWnd;
		
	}

	else if(PageID == 4)
	{
		CVideoGrabberParams *dlg = new CVideoGrabberParams();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd=dlg->m_hWnd;
	}

	 else if(PageID==5)
	{
		CUploadSettingsPage *dlg = new CUploadSettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd=dlg->m_hWnd;
	}


	if(PageID==6)
	{
		CTraySettingsPage *dlg = new CTraySettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd=dlg->m_hWnd;
	}
	else if(PageID==7)
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
   
