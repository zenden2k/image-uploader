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

#include "ResultsWindow.h"
#include "../Common.h"

// CResultsWindow
CResultsWindow::CResultsWindow(CWizardDlg *wizardDlg,CAtlArray<CUrlListItem>  & urlList,bool ChildWindow)
{
	m_WizardDlg = wizardDlg;
	ResultsPanel = new CResultsPanel(wizardDlg,urlList);
	m_childWindow = ChildWindow;
}

CResultsWindow::~CResultsWindow()
{
	delete ResultsPanel;
}

LRESULT CResultsWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(m_childWindow)
	{
		::ShowWindow(GetDlgItem(IDOK),SW_HIDE);
		::ShowWindow(GetDlgItem(IDCANCEL),SW_HIDE);
		::SetWindowPos(GetDlgItem(IDC_RESULTSTAB), 0,0,0,0,0,SWP_NOSIZE);

	}
	else
	{
		CenterWindow(GetParent());
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);
	}

	::SetFocus(GetDlgItem(IDOK));
	SetWindowText(TR("���������� ��������"));
	
	TRC(IDCANCEL, "�������");
	
	TC_ITEM item;
	item.pszText = TR("��� ������ (BBCode)"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 0, &item);
	item.pszText = TR("��� ����� (HTML)"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 1, &item);
	item.pszText = TR("������ ������ (URL)"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 2, &item);

	TabCtrl_SetCurSel(GetDlgItem(IDC_RESULTSTAB), Settings.CodeLang);

	// Creating panel with results
	WINDOWPLACEMENT wp;
	::GetWindowPlacement(GetDlgItem(IDC_RESULTSTAB), &wp);
	TabCtrl_AdjustRect(GetDlgItem(IDC_RESULTSTAB),FALSE, &wp.rcNormalPosition); 	
	RECT rc =  { wp.rcNormalPosition.left, wp.rcNormalPosition.top, -wp.rcNormalPosition.left+wp.rcNormalPosition.right,  -wp.rcNormalPosition.top+wp.rcNormalPosition.bottom };
	ResultsPanel->rectNeeded = rc;
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&rc, 2);
 	ResultsPanel->setEngineList(_EngineList);
	ResultsPanel->Create(m_hWnd,rc);

	ResultsPanel->GetClientRect(&rc);
	BOOL b;
	OnTabChanged(IDC_RESULTSTAB, 0, b);
	return 0; 
}


LRESULT CResultsWindow::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	EndDialog(wID);
	return 0;
}

LRESULT CResultsWindow::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CResultsWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;

}

int CResultsWindow::GetCodeType()
{
	return ResultsPanel->GetCodeType();
}

void CResultsWindow::UpdateOutput()
{
	ResultsPanel->UpdateOutput();
}

void CResultsWindow::SetCodeType(int Index)
{
	ResultsPanel->SetCodeType(Index);
}

void CResultsWindow::Clear()
{
	ResultsPanel->Clear();
}

void CResultsWindow::EnableMediaInfo(bool Enable)
{
	ResultsPanel->EnableMediaInfo(Enable);
}

void CResultsWindow::SetPage(int Index)
{
	TabCtrl_SetCurSel(GetDlgItem(IDC_RESULTSTAB), Index);
	ResultsPanel->SetPage(Index);
}

int CResultsWindow::GetPage()
{
	return TabCtrl_GetCurSel(GetDlgItem(IDC_RESULTSTAB));
}
void CResultsWindow::AddServer(CString server)
{
	ResultsPanel->AddServer( server);
}

void CResultsWindow::InitUpload()
{
	ResultsPanel->InitUpload();
}

void CResultsWindow::Lock()
{
	ResultsPanel->UrlListCS.Lock();
}

void CResultsWindow::Unlock()
{
	ResultsPanel->UrlListCS.Unlock();
}

void CResultsWindow::setUrlList(CAtlArray<CUrlListItem>  * urlList)
{
	ResultsPanel->setUrlList(urlList);
}

LRESULT CResultsWindow::OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int Index = TabCtrl_GetCurSel(GetDlgItem(idCtrl));
	ResultsPanel->SetPage(Index);
	return 0;
}

DLGTEMPLATE* CResultsWindow::GetTemplate()
{
	HINSTANCE hInst =  GetModuleHandle(0);
	HRSRC res = FindResource( hInst, MAKEINTRESOURCE(IDD_RESULTSWINDOW),RT_DIALOG);
	DLGTEMPLATE* dit=(DLGTEMPLATE*)LockResource( LoadResource(hInst, res));
	
	unsigned long sizeDlg = ::SizeofResource(hInst, res);
	HGLOBAL hMyDlgTemplate = ::GlobalAlloc(GPTR, sizeDlg);
	DLGTEMPLATEEX *pMyDlgTemplate = (DLGTEMPLATEEX *)::GlobalLock(hMyDlgTemplate);
	::memcpy(pMyDlgTemplate, dit, sizeDlg);

	if(m_childWindow)
	{
		//pMyDlgTemplate->style = pMyDlgTemplate->style & ~ WS_POPUP;
		pMyDlgTemplate->style = pMyDlgTemplate->style |  WS_CHILD;
	}
	else
	{
		pMyDlgTemplate->style -= WS_CHILD;
		pMyDlgTemplate->style -= DS_CONTROL;
		pMyDlgTemplate->exStyle|=WS_EX_APPWINDOW;
		pMyDlgTemplate->style = pMyDlgTemplate->style |  WS_POPUP | WS_CAPTION;
	}
	return (DLGTEMPLATE*)pMyDlgTemplate;
}

void CResultsWindow::FinishUpload()
{
	BOOL b;
	ResultsPanel->OnCbnSelchangeCodetype(0,0,0, b);
}