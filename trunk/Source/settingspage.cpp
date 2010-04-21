/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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
#include "wizarddlg.h"
#include "settingspage.h"
#include "uxtheme.h"

#pragma comment(lib, "uxtheme.lib")

bool CSettingsPage::OnShow()
{
	EnableNext();
	EnablePrev();
	ShowNext();
	ShowPrev();
	return true;
}

bool CSettingsPage::OnNext()
{
	return true;
}

void CSettingsPage::EnableNext(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDC_NEXT), Enable);
}

void CSettingsPage::EnablePrev(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDC_PREV), Enable);
}

void CSettingsPage::EnableExit(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDCANCEL), Enable);
}

void CSettingsPage::SetNextCaption(LPTSTR Caption)
{
	if(!WizardDlg) return;
	WizardDlg->SetDlgItemText(IDC_NEXT, Caption);
}

void CSettingsPage::ShowNext(bool Show)
{
	if(!WizardDlg) return;
	::ShowWindow(WizardDlg->GetDlgItem(IDC_NEXT), Show?SW_SHOW:SW_HIDE);
}

void CSettingsPage::ShowPrev(bool Show)
{
	if(!WizardDlg) return;
	::ShowWindow(WizardDlg->GetDlgItem(IDC_PREV), Show?SW_SHOW:SW_HIDE);
}

bool CSettingsPage::OnHide()
{
	return false;
}	

HMODULE DllModule = LoadLibrary(_T("uxtheme.dll"));

typedef HRESULT (STDAPICALLTYPE *ETDT_Func)(HWND,DWORD);

// Функция установки фона вкладки в стиле XP+
void TabBackgroundFix(HWND hwnd)
{
	if(!DllModule) return;
	ETDT_Func Func = (ETDT_Func) GetProcAddress(DllModule, "EnableThemeDialogTexture");

	if(!Func) return; 
	Func(hwnd, ETDT_ENABLETAB);
}

bool CSettingsPage::Apply()
{
	return true;
}

CSettingsPage::~CSettingsPage()
{
}