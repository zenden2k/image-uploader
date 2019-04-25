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

#include "LoginDlg.h"

#include "wizarddlg.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Func/WinUtils.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/Network/NetworkClientFactory.h"

// CLoginDlg
CLoginDlg::CLoginDlg(ServerProfile& serverProfile, UploadEngineManager* uem, bool createNew) : serverProfile_(serverProfile)
{
    m_UploadEngine = serverProfile.uploadEngineData();
    ignoreExistingAccount_ = false;
    serverSupportsBeforehandAuthorization_ = false;
    uploadEngineManager_ = uem;
    
    if (!m_UploadEngine->PluginName.empty() || !m_UploadEngine->Engine.empty()) {
        CAdvancedUploadEngine* plugin_ = dynamic_cast<CAdvancedUploadEngine*>(uploadEngineManager_->getUploadEngine(serverProfile));
        if ( plugin_ ) {
            serverSupportsBeforehandAuthorization_ = plugin_->supportsBeforehandAuthorization();
        }
    }
    createNew_ = createNew;

    NetworkClientFactory factory;
    NetworkClient_ = factory.create();
}

CLoginDlg::~CLoginDlg()
{
    
}

LRESULT CLoginDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());

    LoginInfo li = serverProfile_.serverSettings().authData;
    auto uploadEngineData = serverProfile_.uploadEngineData();
    SetWindowText(TR("Autorization parameters"));
    CString loginLabelText = uploadEngineData->LoginLabel.empty() ? CString(TR("Login:")) : CString(U2W(uploadEngineData->LoginLabel)) + _T(":");
    SetDlgItemText(IDC_LOGINLABEL, loginLabelText);
    CString passwordLabelText = uploadEngineData->PasswordLabel.empty() ? CString(TR("Password:")) : CString(U2W(uploadEngineData->PasswordLabel)) + _T(":");
    SetDlgItemText(IDC_PASSWORDLABEL, passwordLabelText);
    TRC(IDCANCEL, "Cancel");
    TRC(IDC_DELETEACCOUNTLABEL, "Delete account");

    HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
    if (hWnd)
    {
        wndAnimation_.SubclassWindow(hWnd);
        if (wndAnimation_.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF), _T("GIF")))
            wndAnimation_.Draw();
        wndAnimation_.ShowWindow(SW_HIDE);
    }

    loginButton_.m_hWnd = GetDlgItem(IDC_AUTHORIZEBUTTON);
    loginButton_.SetWindowText(TR("Sign in..."));
    loginButton_.ShowWindow(serverSupportsBeforehandAuthorization_ ? SW_SHOW : SW_HIDE);
    
    if (!m_UploadEngine->RegistrationUrl.empty()) {
        signupLink_.SubclassWindow(GetDlgItem(IDC_SIGNUPLINK));
        signupLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER;
        signupLink_.m_clrLink = CSettings::DefaultLinkColor;
        CString linkText;
        linkText.Format(TR("Don't have an account? Sign up on %s right now"), static_cast<LPCTSTR>(U2W(m_UploadEngine->Name)));
        signupLink_.SetLabel(linkText);
        signupLink_.SetHyperLink(U2W(m_UploadEngine->RegistrationUrl));
        signupLink_.ShowWindow(SW_SHOW);
    }

    accountName_ = Utf8ToWCstring(li.Login);

    SetDlgItemText(IDC_LOGINEDIT, accountName_);
    SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));
    SetDlgItemText(IDC_LOGINFRAME, Utf8ToWCstring(m_UploadEngine->Name));
    ::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT),m_UploadEngine->NeedPassword);
    ::EnableWindow(GetDlgItem(IDC_PASSWORDLABEL),m_UploadEngine->NeedPassword);

    deleteAccountLabel_.SubclassWindow(GetDlgItem(IDC_DELETEACCOUNTLABEL));
    deleteAccountLabel_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
//    deleteAccountLabel_.SetLabel(deleteAccountLabelText);
    deleteAccountLabel_.m_clrLink = CSettings::DefaultLinkColor;
    deleteAccountLabel_.SetToolTipText(TR("Remove this account from the list (not from the remote server)"));

    deleteAccountLabel_.ShowWindow((createNew_ || accountName_.IsEmpty()) ? SW_HIDE : SW_SHOW);

    ::SetFocus(GetDlgItem(IDC_LOGINEDIT));
    return 0; 
}

LRESULT CLoginDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    Accept();
    return 0;
}

LRESULT CLoginDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CLoginDlg::OnDeleteAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (LocalizedMessageBox(TR("Are you sure you want to delete this account from Image Uploader's internal list?"), APPNAME, MB_ICONQUESTION | MB_YESNO) != IDYES) {
        return 0;
    }
    std::map <std::string, ServerSettingsStruct>& ss = Settings.ServersSettings[serverProfile_.serverName()];
    ss.erase(WCstringToUtf8(accountName_));

    accountName_.Empty();
    EndDialog(ID_DELETEACCOUNT);
    return 0;
}

LRESULT CLoginDlg::OnDoLoginClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    enableControls(false);
    Start();
    return 0;
}

LRESULT CLoginDlg::OnLoginEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CString text = GuiTools::GetWindowText(hWndCtl);
    loginButton_.ShowWindow(serverSupportsBeforehandAuthorization_ && !text.IsEmpty() ? SW_SHOW : SW_HIDE);
    return 0;
}

CString CLoginDlg::accountName()
{
    return accountName_;
}

DWORD CLoginDlg::Run()
{
    if (!m_UploadEngine->PluginName.empty() ) {

        LoginInfo li;
        CString login =GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT); 
        li.Login = WCstringToUtf8(login);
        std::string serverNameA = serverProfile_.serverName();
        if ( !ignoreExistingAccount_ && createNew_ && Settings.ServersSettings[serverNameA].find(li.Login ) != Settings.ServersSettings[serverNameA].end() ) {
            LocalizedMessageBox(TR("Account with such name already exists."),TR("Error"), MB_ICONERROR);
            OnProcessFinished();
            return 0;
        }

        ignoreExistingAccount_ = true;

        if ( li.Login != WCstringToUtf8(accountName_) ) {
            serverProfile_.clearFolderInfo();
        }

        accountName_ = login;
        serverProfile_.setProfileName(WCstringToUtf8(login));
        li.Password = WCstringToUtf8(GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT));
        li.DoAuth = true;
        ServerSettingsStruct& ss = serverProfile_.serverSettings();
        ss.authData = li;
        CAdvancedUploadEngine* plugin_ = dynamic_cast<CAdvancedUploadEngine*>(uploadEngineManager_->getUploadEngine(serverProfile_));

        if (!plugin_ || !plugin_->supportsBeforehandAuthorization()) {
            OnProcessFinished();
            return 0;
        }

        plugin_->setNetworkClient(NetworkClient_.get());
        int res = plugin_->doLogin();
        if ( res ) {
            OnProcessFinished();
            LocalizedMessageBox(TR("Authenticated succesfully."));
            Accept();
            
            return 0;
        } else {
            loginButton_.SetWindowText(TR("Could not authenticate. Please try again."));
        }
        
    }
    OnProcessFinished();
    return 0;
}

void CLoginDlg::OnProcessFinished()
{
    uploadEngineManager_->clearThreadData();
    enableControls(true);
}

void CLoginDlg::Accept()
{
    LoginInfo li;
    TCHAR Buffer[256];

    GetDlgItemText(IDC_LOGINEDIT, Buffer, 256);
    li.Login = WCstringToUtf8(Buffer);

    if ( li.Login.empty() ) {
        LocalizedMessageBox(TR("Login cannot be empty"),TR("Error"), MB_ICONERROR);
        return;
    }
    std::string serverNameA = serverProfile_.serverName();
    if ( !ignoreExistingAccount_ &&  createNew_ && Settings.ServersSettings[serverNameA].find(li.Login ) != Settings.ServersSettings[serverNameA].end() ) {
        LocalizedMessageBox(TR("Account with such name already exists."),TR("Error"), MB_ICONERROR);
        return;
    }

    if ( li.Login != WCstringToUtf8(accountName_) ) {
        serverProfile_.clearFolderInfo();
    }

    accountName_ = Buffer;
    serverProfile_.setProfileName(WCstringToUtf8(Buffer));
    GetDlgItemText(IDC_PASSWORDEDIT, Buffer, 256);
    li.Password = WCstringToUtf8(Buffer);
    li.DoAuth = true;
    uploadEngineManager_->resetAuthorization(serverProfile_);
    serverProfile_.serverSettings().authData = li;
    EndDialog(IDOK);
}

void CLoginDlg::enableControls(bool enable)
{
    wndAnimation_.ShowWindow(!enable ? SW_SHOW : SW_HIDE);
    GuiTools::EnableDialogItem(m_hWnd, IDC_LOGINEDIT, enable);
    GuiTools::EnableDialogItem(m_hWnd, IDC_PASSWORDEDIT, enable&&m_UploadEngine->NeedPassword);
    GuiTools::EnableDialogItem(m_hWnd, IDOK, enable);
    GuiTools::EnableDialogItem(m_hWnd, IDCANCEL, enable);
    loginButton_.EnableWindow(enable);
    deleteAccountLabel_.EnableWindow(enable);
}
