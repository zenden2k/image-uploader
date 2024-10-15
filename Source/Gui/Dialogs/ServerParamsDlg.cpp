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

#include "ServerParamsDlg.h"

#include "Core/i18n/Translator.h"
#include "Gui/GuiTools.h"
#include "Gui/Dialogs/ServerFolderSelect.h"
#include "Gui/Dialogs/InputDialog.h"
#include "Func/WinUtils.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/Upload/Parameters/TextParameter.h"
#include "Core/Upload/Parameters/ChoiceParameter.h"

// CServerParamsDlg
CServerParamsDlg::CServerParamsDlg(const ServerProfile& serverProfile, UploadEngineManager * uploadEngineManager, bool focusOnLoginEdit) : m_ue(serverProfile.uploadEngineData()), serverProfile_(serverProfile)
        
{
    focusOnLoginControl_ = focusOnLoginEdit;
    uploadEngineManager_ = uploadEngineManager;
    m_pluginLoader = nullptr;
}

CServerParamsDlg::~CServerParamsDlg()
{
}

LRESULT CServerParamsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    TRC(IDCANCEL, "Cancel");
    TRC(IDOK, "OK");
    TRC(IDC_LOGINLABEL, "Login:");
    TRC(IDC_PASSWORDLABEL, "Password:");
    TRC(IDC_DOAUTH, "Authorize");
    TRC(IDC_FOLDERLABEL, "Folder/album:");
    TRC(IDC_BROWSESERVERFOLDERS, "Select...");
    TRC(IDC_PARAMETERSLABEL, "Parameters:");
    DlgResize_Init();
    CString WindowTitle;
    CString serverName = Utf8ToWCstring(m_ue->Name);
    WindowTitle.Format(TR("%s server settings"), serverName.GetString());
    SetWindowText(WindowTitle);
    GuiTools::ShowDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, m_ue->SupportsFolders);
    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, m_ue->SupportsFolders);
    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERNAMELABEL, m_ue->SupportsFolders);
    ::EnableWindow(GetDlgItem(IDC_BROWSESERVERFOLDERS), m_ue->SupportsFolders);
    GuiTools::ShowDialogItem(m_hWnd, IDC_DOAUTH, m_ue->NeedAuthorization == CUploadEngineData::naAvailable);

    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, m_ue->SupportsFolders);
    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERNAMELABEL, m_ue->SupportsFolders);
    GuiTools::ShowDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, m_ue->SupportsFolders);
    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERICON, m_ue->SupportsFolders);

    BasicSettings* settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = settings->getServerSettings(serverProfile_);

    LoginInfo li = serverSettings ? serverSettings->authData : LoginInfo();
    SetDlgItemText(IDC_LOGINEDIT, Utf8ToWCstring(li.Login));
    oldLogin_ = Utf8ToWCstring(li.Login);
    SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));

    SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, (li.DoAuth ? BST_CHECKED : BST_UNCHECKED));
    doAuthChanged();

    if (focusOnLoginControl_ && m_ue->NeedAuthorization) {
        GuiTools::SetCheck(m_hWnd, IDC_DOAUTH, true);
        doAuthChanged();
        ::SetFocus(GetDlgItem(IDC_LOGINEDIT));
        SendDlgItemMessage(IDC_LOGINEDIT, EM_SETSEL, 0, -1);
    }

    GuiTools::EnableDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, !oldLogin_.IsEmpty());

    m_wndParamList.SubclassWindow(GetDlgItem(IDC_PARAMLIST));
    m_wndParamList.SetExtendedListStyle(PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);

    parameterListAdapter_ = std::make_unique<ParameterListAdapter>(&m_paramNameList, &m_wndParamList);

    m_pluginLoader = dynamic_cast<CAdvancedUploadEngine*>(uploadEngineManager_->getUploadEngine(serverProfile_));
    if (m_pluginLoader) {
        m_pluginLoader->getServerParamList(m_paramNameList);

        parameterListAdapter_->updateControl(serverSettings);
    }
    CString folderTitle = Utf8ToWCstring( serverProfile_.folderTitle());

    SetDlgItemText(IDC_FOLDERNAMELABEL, folderTitle.IsEmpty() ? (CString("<") + TR("not selected") + CString(">")) : folderTitle);

    return 1;  // Let the system set the focus
}

LRESULT CServerParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CString login = GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT);
    serverProfile_.setProfileName(WCstringToUtf8(login));

    BasicSettings* settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = settings->getServerSettings(serverProfile_);

    if (serverSettings) {
        serverSettings->authData.DoAuth = GuiTools::GetCheck(m_hWnd, IDC_DOAUTH);
        serverSettings->authData.Login = WCstringToUtf8(login);
        serverSettings->authData.Password = WCstringToUtf8(GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT));

        parameterListAdapter_->saveValues(serverSettings);
    } else {
        LOG(WARNING) << "No server settings for name=" << serverProfile_.serverName() << " login=" << serverProfile_.profileName();
    }

    EndDialog(wID);
    return 0;
}

LRESULT CServerParamsDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}


LRESULT CServerParamsDlg::OnClickedDoAuth(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    doAuthChanged();
    return 0;
}

void CServerParamsDlg::doAuthChanged() {
    ::EnableWindow(GetDlgItem(IDC_DOAUTH), false);
    ::EnableWindow(GetDlgItem(IDC_LOGINEDIT), false);
    ::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), false);
    //::EnableWindow(GetDlgItem(IDC_LOGINEDIT), IS_CHECKED(IDC_DOAUTH) || m_ue->NeedAuthorization == CUploadEngineData::naObligatory);
    //::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), (IS_CHECKED(IDC_DOAUTH) || m_ue->NeedAuthorization == CUploadEngineData::naObligatory) && m_ue->NeedPassword);
}

LRESULT CServerParamsDlg::OnBrowseServerFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CString login = GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT);
    serverProfile_.setProfileName(WCstringToUtf8(login));

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = Settings->getServerSettings(serverProfile_);
    
    CString password = GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT);
    if (serverSettings) {
        serverSettings->authData.Login = WCstringToUtf8(login);
        serverSettings->authData.Password = WCstringToUtf8(password);
        serverSettings->authData.DoAuth = GuiTools::GetCheck(m_hWnd, IDC_DOAUTH);
    } else {
        LOG(WARNING) << "No server settings for name=" << serverProfile_.serverName() << " login=" << serverProfile_.profileName();
    }
    CServerFolderSelect folderSelectDlg(serverProfile_, uploadEngineManager_);
    folderSelectDlg.m_SelectedFolder.id = serverProfile_.folderId();
    folderSelectDlg.m_SelectedFolder.parentIds = serverProfile().parentIds();

    if ( folderSelectDlg.DoModal(m_hWnd) == IDOK ) {
        CFolderItem folder = folderSelectDlg.m_SelectedFolder;
    

        if(!folder.id.empty()){
            serverProfile_.setFolderId(folder.getId());
            serverProfile_.setFolderTitle(folder.getTitle());
            serverProfile_.setFolderUrl(folder.viewUrl);
            serverProfile_.setParentIds(folder.parentIds);
        } else {
            serverProfile_.setFolderId("");
            serverProfile_.setFolderTitle("");
            serverProfile_.setFolderUrl("");
            serverProfile_.setParentIds({});
        }

        SetDlgItemText(IDC_FOLDERNAMELABEL, Utf8ToWCstring( folder.getTitle() ));
    }
    if(!m_pluginLoader)
    {
        return 0;
    }
    m_wndParamList.ResetContent();
    m_pluginLoader->getServerParamList(m_paramNameList);

    if (serverSettings) {
        parameterListAdapter_->updateControl(serverSettings);
    }

    return 0;
}

LRESULT CServerParamsDlg::OnLoginEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    if ( ::GetFocus() == hWndCtl ) {
        serverProfile_.setFolderId("");
        serverProfile_.setFolderTitle("");
        serverProfile_.setFolderUrl("");
        serverProfile_.setParentIds({});
        SetDlgItemText(IDC_FOLDERNAMELABEL,  (CString("<") + TR("not selected") + CString(">")) );

    }
    return 0;
}

ServerProfile CServerParamsDlg::serverProfile() const
{
    return serverProfile_;
}
