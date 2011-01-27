#include "WizardCommon.h"

CAppModule _Module;
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