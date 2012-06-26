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

#ifndef IU_GUI_WIZARDCOMMON_H
#define IU_GUI_WIZARDCOMMON_H

#pragma once
#include <windows.h>

#define IU_IDC_CONST 12255
#define IDC_SETTINGS    (IU_IDC_CONST + 1)
#define IDC_REGIONPRINT (IU_IDC_CONST + 2)
#define IDC_MEDIAFILEINFO (IU_IDC_CONST + 3)
#define IDC_CLIPBOARD (IU_IDC_CONST + 4)
#define IDC_ADDFOLDER (IU_IDC_CONST + 5)
#define IDC_ADDFILES (IU_IDC_CONST + 6)
#define IDC_DOWNLOADIMAGES (IU_IDC_CONST + 7)

class CWizardDlg;
class CWizardPage
{
	public:
		CWizardDlg* WizardDlg;
		virtual ~CWizardPage() = NULL;
		HBITMAP HeadBitmap;
		virtual bool OnShow();
		virtual bool OnHide();
		virtual bool OnNext();
		void EnableNext(bool Enable = true);
		void EnablePrev(bool Enable = true);
		void EnableExit(bool Enable = true);
		void SetNextCaption(LPCTSTR Caption);
		HWND PageWnd;
		void ShowNext(bool Show = true);
		void ShowPrev(bool Show = true);
};

extern CWizardDlg* pWizardDlg;

#endif // IU_GUI_WIZARDCOMMON_H
