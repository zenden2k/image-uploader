/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef IU_GUI_DIALOGS_UPLOADSETTINGSPAGE_H
#define IU_GUI_DIALOGS_UPLOADSETTINGSPAGE_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "settingspage.h"
// CUploadSettingsPage

class CUploadSettingsPage : 
    public CDialogImpl<CUploadSettingsPage>, public CSettingsPage,
        public CWinDataExchange <CUploadSettingsPage >
{
public:
    CUploadSettingsPage();
virtual ~CUploadSettingsPage();
    enum { IDD = IDD_UPLOADSETTINGSPAGE };

    BEGIN_MSG_MAP(CUploadSettingsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        COMMAND_HANDLER(IDC_USEPROXYSERVER, BN_CLICKED, OnClickedUseProxy)
        COMMAND_HANDLER(IDC_NEEDSAUTH, BN_CLICKED, OnClickedUseProxyAuth)
        COMMAND_HANDLER(IDC_BROWSESCRIPTBUTTON, BN_CLICKED, OnBnClickedBrowseScriptButton)
        COMMAND_HANDLER(IDC_NOPROXY, BN_CLICKED, OnClickedNoProxy)
        COMMAND_HANDLER(IDC_USESYSTEMPROXY, BN_CLICKED, OnClickedUseSystemProxy)
        COMMAND_HANDLER(IDC_EXECUTESCRIPTCHECKBOX, BN_CLICKED, OnExecuteScriptCheckboxClicked)
    END_MSG_MAP()
        
    BEGIN_DDX_MAP(CScreenshotDlg)
        DDX_CONTROL_HANDLE(IDC_SERVERTYPECOMBO, serverTypeCombo_)
    END_DDX_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedUseProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnClickedUseProxyAuth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnClickedNoProxy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedUseSystemProxy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnExecuteScriptCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    bool Apply() override;
    void TranslateUI();
    LRESULT OnBnClickedBrowseScriptButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void proxyRadioChanged();
    void executeScriptCheckboxChanged();
protected:
    CComboBox serverTypeCombo_;
    void CheckBounds(int controlId, int minValue, int maxValue, int labelId = -1) const;
};

#endif // IU_GUI_DIALOGS_UPLOADSETTINGSPAGE_H


