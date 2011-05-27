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

#include "WizardCommon.h"
#include "../atlheaders.h"
#include "Dialogs/WizardDlg.h"

CWizardPage::~CWizardPage()
{
}

bool CWizardPage::OnShow()
{
	EnableNext();
	EnablePrev();
	ShowNext();
	ShowPrev();
	return true;
}

bool CWizardPage:: OnNext()
{
	return true;
}

void CWizardPage::EnableNext(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDC_NEXT), Enable);
}

void CWizardPage::EnablePrev(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDC_PREV), Enable);
}

void CWizardPage::EnableExit(bool Enable)
{
	if(!WizardDlg) return;
	::EnableWindow(WizardDlg->GetDlgItem(IDCANCEL), Enable);
}

void CWizardPage::SetNextCaption(LPCTSTR Caption)
{
	if(!WizardDlg) return;
	WizardDlg->SetDlgItemText(IDC_NEXT, Caption);
}

void CWizardPage::ShowNext(bool Show)
{
	if(!WizardDlg) return;
	::ShowWindow(WizardDlg->GetDlgItem(IDC_NEXT), Show?SW_SHOW:SW_HIDE);
}

void CWizardPage::ShowPrev(bool Show)
{
	if(!WizardDlg) return;
	::ShowWindow(WizardDlg->GetDlgItem(IDC_PREV), Show?SW_SHOW:SW_HIDE);
}

bool CWizardPage:: OnHide()
{
	return false;
}