#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#pragma once 

#include "atlheaders.h"
#include <windows.h>

class CWizardDlg;
class CSettingsDlg;
class CSettingsPage
{
    public:
        virtual ~CSettingsPage()=0;
        CWizardDlg *WizardDlg;
        HBITMAP HeadBitmap;
        virtual bool OnShow();
        virtual bool OnHide();
        virtual bool OnNext();
        void EnableNext(bool Enable = true);
        void EnablePrev(bool Enable = true);
        void EnableExit(bool Enable = true);
        void SetNextCaption(LPTSTR Caption);
        HWND PageWnd;
        void ShowNext(bool Show = true);
        void ShowPrev(bool Show = true);

        void FixBackground();
        
        virtual bool Apply();

};
void TabBackgroundFix(HWND hwnd);

#endif // SETTINGSPAGE_H