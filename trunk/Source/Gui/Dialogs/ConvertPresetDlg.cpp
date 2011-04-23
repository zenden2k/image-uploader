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

#include "ConvertPresetDlg.h"
#include "../../Func/langclass.h"
#include "ThumbSettingsPage.h"
// CConvertPresetDlg
CConvertPresetDlg::CConvertPresetDlg(int Page)
{
	CurPage = -1;
	PageToShow = Page;
	PrevPage = -1;
	ZeroMemory(Pages, sizeof(Pages));
}

CConvertPresetDlg::~CConvertPresetDlg()
{
	for(int i=0; i<ConvertPageCount; i++)
		if(Pages[i]) delete Pages[i];
}

LRESULT CConvertPresetDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// center the dialog on the screen
	CenterWindow();
	HWND parent = GetParent();
	if(!parent || !::IsWindowVisible(parent))
	{
		SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & WS_EX_APPWINDOW);
		//MessageBox(L"OF COURSE WI SET");
	}
	
	TC_ITEM item;
    item.pszText = TR("Изображения"); 
    item.mask = TCIF_TEXT;
    TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 0, &item);

	item.pszText = TR("Миниатюры");
    item.mask=TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_TABCONTROL), 1, &item);

	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_APPLY, "Применить");
	SetWindowText(TR("Настройки обработки изображений"));
	
	ShowPage(PageToShow);
	return 0;  // Let the system set the focus
}

LRESULT CConvertPresetDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for(int i=0; i<ConvertPageCount; i++)
	{
		if(Pages[i] && !Pages[i]->Apply()) 
		{
			ShowPage(i);
			return 0;
		}// If some tab cannot apply changes - do not close dialog
	}

	//Settings.SaveSettings();
	EndDialog(wID);
	return 0;
}

LRESULT CConvertPresetDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CConvertPresetDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

void CConvertPresetDlg::CloseDialog(int nVal)
{
	if(CurPage >= 0)
		Pages[CurPage]->OnHide();
	
	DestroyWindow();
	::PostQuitMessage(nVal);
}

bool CConvertPresetDlg::ShowPage(int idPage)
{
	if(idPage< 0 || idPage> ConvertPageCount-1) return false;

	if(idPage == CurPage) return true;

	if(!Pages[idPage]) 
		CreatePage(idPage);

	if(Pages[idPage]) 
		::ShowWindow(Pages[idPage]->PageWnd, SW_SHOW);

	if(CurPage != -1 && Pages[CurPage]) ::ShowWindow(Pages[CurPage]->PageWnd, SW_HIDE);
	CurPage = idPage;

	//m_SettingsPagesListBox.SetCurSel(CurPage);
	if(CurPage !=	TabCtrl_GetCurSel(GetDlgItem(IDC_TABCONTROL)))
	TabCtrl_SetCurSel(GetDlgItem(IDC_TABCONTROL), CurPage);
	return true;
}

LRESULT CConvertPresetDlg::OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	for(int i=0; i<ConvertPageCount; i++)
		if(Pages[i] && !Pages[i]->Apply()) 
		{
			ShowPage(i);
			return 0;
		}
	
//	Settings.SaveSettings();
	return 0;
}

bool CConvertPresetDlg::CreatePage(int PageID)
{
	RECT rc = {150,3,636,400};

	/*if(PageID == 0)
	{
		CGeneralSettings *dlg = new CGeneralSettings();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd=dlg->m_hWnd;
	}*/
	
	if(PageID == 0)
	{
		CLogoSettings *dlg = new CLogoSettings();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd = dlg->m_hWnd;
	}
	else if(PageID == 1)
	{
		CThumbSettingsPage *dlg = new CThumbSettingsPage();
		Pages[PageID]= dlg;
		dlg->Create(m_hWnd,rc);
		Pages[PageID]->PageWnd = dlg->m_hWnd;
	}

	WINDOWPLACEMENT wp;
	::GetWindowPlacement(GetDlgItem(IDC_TABCONTROL), &wp);
	TabCtrl_AdjustRect(GetDlgItem(IDC_TABCONTROL),FALSE, &wp.rcNormalPosition); 	
	::SetWindowPos(Pages[PageID]->PageWnd,  0, wp.rcNormalPosition.left, wp.rcNormalPosition.top, -wp.rcNormalPosition.left+wp.rcNormalPosition.right,  -wp.rcNormalPosition.top+wp.rcNormalPosition.bottom, 0);

	Pages[PageID]->FixBackground();
	return true;
}

LRESULT CConvertPresetDlg::OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int Index = TabCtrl_GetCurSel(GetDlgItem(idCtrl));
	ShowPage(Index);
	return 0;
}

