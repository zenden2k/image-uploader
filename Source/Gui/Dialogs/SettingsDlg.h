/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#pragma once
#include "atlheaders.h"
#include "welcomedlg.h"
#include "maindlg.h"
#include "VideoGrabberPage.h"
#include "uploadsettings.h"
#include "aboutdlg.h"
#include "resource.h"       // main symbols
#include <atlcrack.h>
#include "logosettings.h"
#include "settingspage.h"
#include "Gui/Controls/tablistbox.h"
// CSettingsDlg

//const int SettingsPageCount = 10;

class CSettingsDlg : public CCustomDialogIndirectImpl<CSettingsDlg>
{
    public:
        CSettingsDlg(int Page, UploadEngineManager* uploadEngineManager);
        ~CSettingsDlg();
        enum { IDD = IDD_SETTINGSDLG };
        enum { kStatusLabelTimer = 1, SettingsPageCount = 10 };
        enum SettingsPage { spGeneral, spServers, spImages, spThumbnails, spScreenshot,
        spVideo, spUploading, spIntegration, spTrayIcon, spHotkeys};
    protected:
        BEGIN_MSG_MAP(CSettingsDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_TIMER, OnTimer)
            NOTIFY_HANDLER(IDC_TABCONTROL, TCN_SELCHANGE, OnTabChanged)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER_EX(IDC_APPLY, BN_CLICKED, OnApplyBnClicked)
            COMMAND_HANDLER(IDC_SETTINGSPAGESLIST, LBN_SELCHANGE, OnSettingsPagesSelChanged)
            MSG_WM_CTLCOLORSTATIC(OnCtlColorStatic)
            REFLECT_NOTIFICATIONS()
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    void CloseDialog(int nVal);
    LRESULT OnSettingsPagesSelChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCtlColorStatic(HDC hdc, HWND hwndChild);
    int CurPage;
    CIcon hIcon;
    CIcon hIconSmall;
    bool CreatePage(int PageID);
    CSettingsPage* Pages[SettingsPageCount];
    int PageToShow;
    bool ShowPage(int idPage);
    CTabListBox m_SettingsPagesListBox;
    LRESULT OnApplyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
protected:
    UploadEngineManager* uploadEngineManager_;
    CBrush backgroundBrush_;
    CFont saveStatusLabel_;
};

#endif // SETTINGSDLG_H