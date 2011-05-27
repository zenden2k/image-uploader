#ifndef WIZARDCOMMON_H
#define WIZARDCOMMON_H
#pragma once

#define IU_IDC_CONST 12255
#define IDC_SETTINGS		(IU_IDC_CONST+1)
#define IDC_REGIONPRINT (IU_IDC_CONST+2)
#define IDC_MEDIAFILEINFO (IU_IDC_CONST+3)
#define IDC_CLIPBOARD (IU_IDC_CONST+4)
#define IDC_ADDFOLDER (IU_IDC_CONST+5)
#define IDC_ADDFILES (IU_IDC_CONST+6)
#define IDC_DOWNLOADIMAGES (IU_IDC_CONST+7)

class CWizardDlg;
class CWizardPage
{
public:
	CWizardDlg *WizardDlg;
	virtual ~CWizardPage()=NULL;
	HBITMAP HeadBitmap;
	virtual bool OnShow();
	virtual bool OnHide();
	virtual bool OnNext();
	void EnableNext(bool Enable=true);
	void EnablePrev(bool Enable=true);
	void EnableExit(bool Enable=true);
	void SetNextCaption(LPCTSTR Caption);
	HWND PageWnd;
	void ShowNext(bool Show=true);
	void ShowPrev(bool Show=true);
};

extern CWizardDlg* pWizardDlg;
#endif // WIZARDCOMMON_H





