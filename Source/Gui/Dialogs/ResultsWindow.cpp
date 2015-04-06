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
#include "ResultsWindow.h"

#include "atlheaders.h"
#include "Func/Common.h"
#include "Func/Settings.h"
#include <Gui/GuiTools.h>
// CResultsWindow
CResultsWindow::CResultsWindow(CWizardDlg *wizardDlg,CAtlArray<CUrlListItem>  & urlList,bool ChildWindow)
{
	m_WizardDlg = wizardDlg;
	ResultsPanel = new CResultsPanel(wizardDlg, urlList);
	m_childWindow = ChildWindow;
}


CResultsWindow::~CResultsWindow()
{
	delete ResultsPanel;
}

void CResultsWindow::setOnShortenUrlChanged(fastdelegate::FastDelegate1<bool> fd) {
	ResultsPanel->OnShortenUrlChanged = fd;
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
		HICON hIcon = GuiTools::LoadBigIcon(IDR_MAINFRAME);
		HICON hIconSmall = GuiTools::LoadSmallIcon(IDR_MAINFRAME);
		SetIcon(hIcon, TRUE);
		SetIcon(hIconSmall, FALSE);
	}

	::SetFocus(GetDlgItem(IDOK));
	SetWindowText(TR("Результаты загрузки"));
	
	TRC(IDCANCEL, "Закрыть");
	
	TC_ITEM item;
	item.pszText = TR("Для форума (BBCode)"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 0, &item);
	item.pszText = TR("Для сайта (HTML)"); 
	item.mask = TCIF_TEXT;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 1, &item);
	item.pszText = TR("Просто ссылки (URL)"); 
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
void CResultsWindow::AddServer(ServerProfile server)
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

void CResultsWindow::setShortenUrls(bool shorten) {
	ResultsPanel->shortenUrl_ = shorten;
}