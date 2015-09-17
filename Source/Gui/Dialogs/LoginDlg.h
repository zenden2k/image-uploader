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
#ifndef GUI_DIALOGS_LOGINDLG_H
#define GUI_DIALOGS_LOGINDLG_H

#pragma once
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/Upload/UploadEngine.h"
#include "3rdpart/thread.h"
#include "Gui/Controls/PictureExWnd.h"

class ServerProfile;
class UploadEngineManager;
// CLoginDlg

LoginInfo LoadLogin(int ServerId);

class CLoginDlg : public CDialogImpl<CLoginDlg>    , public CThreadImpl<CLoginDlg>
{
    public:
        int ServerId;
        CLoginDlg(ServerProfile& serverProfile, UploadEngineManager* uem, bool CreateNew = false );
        ~CLoginDlg();
        enum { IDD = IDD_LOGINDLG };
        enum { ID_DELETEACCOUNT = 30 };
    protected:
        BEGIN_MSG_MAP(CLoginDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_USEIECOOKIES, BN_CLICKED, OnClickedUseIeCookies)
            COMMAND_ID_HANDLER(IDC_DELETEACCOUNTLABEL, OnDeleteAccountClicked)
            COMMAND_ID_HANDLER(IDC_DOLOGINLABEL, OnDoLoginClicked)    
            COMMAND_HANDLER(IDC_LOGINEDIT, EN_CHANGE, OnLoginEditChange);
        END_MSG_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedUseIeCookies(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnDeleteAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnDoLoginClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnLoginEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        CUploadEngineData *m_UploadEngine;
        CString accountName();
        DWORD Run();
        void OnProcessFinished();
        void Accept();
protected:
    ServerProfile& serverProfile_;
    CHyperLink deleteAccountLabel_;
    CHyperLink doLoginLabel_;
    CString accountName_;
    bool createNew_;
    bool ignoreExistingAccount_;
    bool serverSupportsBeforehandAuthorization_;
    void enableControls(bool enable);
    NetworkClient NetworkClient_;
    CPictureExWnd wndAnimation_;
    UploadEngineManager* uploadEngineManager_;
    
};

#endif // LOGINDLG_H


