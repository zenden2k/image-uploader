/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#include "stdafx.h"
#include "SettingsDlg.h"
#include "langclass.h"
#include "langselect.h"
#include "traysettings.h"
#include "hotkeysettings.h"

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
	for(int i=0; i<6; i++) /* Freeing memory xD */
		if(Pages[i]) delete Pages[i];
}

LRESULT CSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_APPLY, "Применить");
	SetWindowText(TR("Настройки"));
	
	TC_ITEM item;
	item.pszText = TR("Основные"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 0, &item);

	item.pszText = TR("Видео");
	item.mask=TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 1, &item);

	item.pszText = TR("Изображения");
	item.mask=TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 2, &item);
	
	item.pszText = TR("Загрузка"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 3, &item);

	item.pszText = TR("Трей"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 4, &item);

	item.pszText = TR("Горячие клавиши"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 5, &item);

	ShowPage(PageToShow);
	return 0;  // Let the system set the focus
}

LRESULT CSettingsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for(int i=0; i<6; i++)
		if(Pages[i] && !Pages[i]->Apply()) return 0; // If some tab cannot apply changes - do not close dialog

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
	if(idPage< 0 || idPage> 5) return false;

	if(!Pages[idPage]) 
		CreatePage(idPage);

	if(Pages[idPage]) 
		::ShowWindow(Pages[idPage]->PageWnd, SW_SHOW);

	if(CurPage != -1 && Pages[CurPage]) ::ShowWindow(Pages[CurPage]->PageWnd, SW_HIDE);
	CurPage = idPage;

	if(CurPage !=	TabCtrl_GetCurSel(GetDlgItem(IDC_TABCONTROL)))
	TabCtrl_SetCurSel(GetDlgItem(IDC_TABCONTROL), CurPage);
	return true;
}

LRESULT CSettingsDlg::OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	for(int i=0; i<6; i++)
		if(Pages[i] && !Pages[i]->Apply()) return 0;
	
	Settings.SaveSettings();
	return 0;
}

bool CSettingsDlg::CreatePage(int PageID)
{
	RECT rc = {150,3,636,400};

	if(PageID == 2)
	{
		CLogoSettings *dlg = new CLogoSettings();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);

		RECT rc={0,0,0,0},Rec;
		dlg->GetClientRect(&rc);
		TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &rc); 	
		::GetWindowRect(GetDlgItem(IDC_TABCONTROL),&Rec);
		POINT p={Rec.left, Rec.top};
		ScreenToClient(&p);

		rc.left+=p.x;
		rc.top+=p.y;
		rc.right+=p.x;
		rc.bottom+=p.y;

		Pages[PageID]->PageWnd = dlg->m_hWnd;
		dlg->SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,/*SWP_NOZORDER*/0);
	}

	if(PageID == 0)
	{
		CGeneralSettings *dlg = new CGeneralSettings();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);

		RECT rc={0,0,0,0},Rec;
		dlg->GetClientRect(&rc);
		TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &rc); 	
		::GetWindowRect(GetDlgItem(IDC_TABCONTROL),&Rec);
		POINT p={Rec.left, Rec.top};
		ScreenToClient(&p);
		
		rc.left+=p.x;
		rc.top+=p.y;
		rc.right+=p.x;
		rc.bottom+=p.y;

		Pages[PageID]->PageWnd = dlg->m_hWnd;
		dlg->SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,/*SWP_NOZORDER*/0);
	}

	if(PageID == 1)
	{
		CVideoGrabberParams *dlg = new CVideoGrabberParams();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);

		RECT rc={0,0,0,0},Rec;
		dlg->GetClientRect(&rc);
		TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &rc); 	
		::GetWindowRect(GetDlgItem(IDC_TABCONTROL),&Rec);
		POINT p={Rec.left, Rec.top};
		ScreenToClient(&p);
		rc.left+=p.x;
		rc.top+=p.y;
		rc.right+=p.x;
		rc.bottom+=p.y;
		Pages[PageID]->PageWnd = dlg->m_hWnd;
		dlg->SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,/*SWP_NOZORDER*/0);
	}

	if(PageID==3)
	{
		CUploadSettingsPage *dlg = new CUploadSettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);

		RECT rc={0,0,0,0},Rec;
		dlg->GetClientRect(&rc);
		TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &rc); 	
		::GetWindowRect(GetDlgItem(IDC_TABCONTROL),&Rec);
		POINT p={Rec.left, Rec.top};
		ScreenToClient(&p);
		rc.left+=p.x;
		rc.top+=p.y;
		rc.right+=p.x;
		rc.bottom+=p.y;
		Pages[PageID]->PageWnd = dlg->m_hWnd;
		dlg->SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,/*SWP_NOZORDER*/0);
	}

	if(PageID==4)
	{
		CTraySettingsPage *dlg = new CTraySettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);

		RECT rc={0,0,0,0},Rec;
		dlg->GetClientRect(&rc);
		TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &rc); 	
		::GetWindowRect(GetDlgItem(IDC_TABCONTROL),&Rec);
		POINT p={Rec.left, Rec.top};
		ScreenToClient(&p);
		rc.left+=p.x;
		rc.top+=p.y;
		rc.right+=p.x;
		rc.bottom+=p.y;
		Pages[PageID]->PageWnd = dlg->m_hWnd;
		dlg->SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,/*SWP_NOZORDER*/0);
	}
	if(PageID==5)
	{
		CHotkeySettingsPage *dlg = new CHotkeySettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);

		RECT rc={0,0,0,0},Rec;
		dlg->GetClientRect(&rc);
		TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &rc);
		
		::GetWindowRect(GetDlgItem(IDC_TABCONTROL),&Rec);
		POINT p={Rec.left, Rec.top};
		ScreenToClient(&p);
		rc.left+=p.x;
		rc.top+=p.y;
		rc.right+=p.x;
		rc.bottom+=p.y;
		Pages[PageID]->PageWnd = dlg->m_hWnd;
		//dlg->MoveWindow(&rc);
		dlg->SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,/*SWP_NOZORDER*/0);
		
	}
	return true;
}

LRESULT CSettingsDlg::OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int Index = TabCtrl_GetCurSel(GetDlgItem(idCtrl));
	ShowPage(Index);
	return 0;
}

