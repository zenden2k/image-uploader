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
#define IDC_REUPLOADIMAGES (IU_IDC_CONST + 8)
#define IDC_SHORTENURL (IU_IDC_CONST + 9)

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
