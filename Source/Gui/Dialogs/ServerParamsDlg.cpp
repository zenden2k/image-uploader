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

#include "atlheaders.h"
#include "ServerParamsDlg.h"
#include "Func/LangClass.h"
#include "Func/Settings.h"
#include <Gui/GuiTools.h>
#include <Gui/Dialogs/ServerFolderSelect.h>
#include <Gui/Dialogs/InputDialog.h>
// CServerParamsDlg
CServerParamsDlg::CServerParamsDlg(ServerProfile  serverProfile,bool focusOnLoginEdit): m_ue(serverProfile.uploadEngineData()),
			serverProfile_(serverProfile)
{
	focusOnLoginControl_ = focusOnLoginEdit;
}

CServerParamsDlg::~CServerParamsDlg()
{
}

LRESULT CServerParamsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_DOAUTH, "Выполнять авторизацию");
	TRC(IDC_FOLDERLABEL, "Папка/альбом:");
	TRC(IDC_BROWSESERVERFOLDERS, "Выбрать...");
	TRC(IDC_PARAMETERSLABEL, "Параметры:");
	DlgResize_Init();
	CString WindowTitle;
	CString serverName = Utf8ToWCstring(m_ue->Name);
	WindowTitle.Format(TR("Параметры сервера %s"),(LPCTSTR)serverName);
	SetWindowText(WindowTitle);
	GuiTools::ShowDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, m_ue->SupportsFolders);
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, m_ue->SupportsFolders);
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERNAMELABEL, m_ue->SupportsFolders);
	::EnableWindow(GetDlgItem(IDC_BROWSESERVERFOLDERS), m_ue->SupportsFolders);
	GuiTools::ShowDialogItem(m_hWnd, IDC_DOAUTH, m_ue->NeedAuthorization == CUploadEngineData::naAvailable );

	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, m_ue->SupportsFolders );
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERNAMELABEL, m_ue->SupportsFolders );
	GuiTools::ShowDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, m_ue->SupportsFolders );
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERICON, m_ue->SupportsFolders );

	
	ServerSettingsStruct &serverSettings = serverProfile_.serverSettings();
	LoginInfo li = serverSettings.authData;
	SetDlgItemText(IDC_LOGINEDIT, Utf8ToWCstring(li.Login));
	oldLogin_ =  Utf8ToWCstring(li.Login);
	SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));


	SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, (li.DoAuth ? BST_CHECKED : BST_UNCHECKED));
	doAuthChanged();
	int result = 1;
	if ( focusOnLoginControl_ && m_ue->NeedAuthorization ) {
		GuiTools::SetCheck(m_hWnd, IDC_DOAUTH, true);
		doAuthChanged();
		::SetFocus(GetDlgItem(IDC_LOGINEDIT) );
		SendDlgItemMessage(IDC_LOGINEDIT, EM_SETSEL, 0, -1);
		result = 0;
	}

	GuiTools::EnableDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, !oldLogin_.IsEmpty());

	


	m_wndParamList.SubclassWindow(GetDlgItem(IDC_PARAMLIST));
	m_wndParamList.SetExtendedListStyle(PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);

	m_pluginLoader = iuPluginManager.getPlugin(m_ue->Name, m_ue->PluginName,serverSettings);
	if(!m_pluginLoader)
	{
		return 0;
	}
	m_pluginLoader->getServerParamList(m_paramNameList);

	std::map<std::string,std::string>::iterator it;
	for( it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it)
	{
		CString name = it->first.c_str();
		CString humanName = it->second.c_str();
		m_wndParamList.AddItem( PropCreateSimple(humanName, Utf8ToWCstring(serverSettings.params[WCstringToUtf8(name)])) );
	}
	CString folderTitle = Utf8ToWCstring( serverProfile_.folderTitle()) ;

	SetDlgItemText(IDC_FOLDERNAMELABEL, folderTitle.IsEmpty() ? (CString("<") + TR("не выбран") + CString(">")) : folderTitle);

	return 1;  // Let the system set the focus
}

LRESULT CServerParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::map<std::string,std::string>::iterator it;
	CString login = GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT);
	serverProfile_.setProfileName(login);
	ServerSettingsStruct &serverSettings = serverProfile_.serverSettings();
	serverSettings.authData.DoAuth = GuiTools::GetCheck(m_hWnd, IDC_DOAUTH);
	serverSettings.authData.Login    = WCstringToUtf8( login );
	serverSettings.authData.Password = WCstringToUtf8( GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT) );

	for(it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it)
	{

		CString name = it->first.c_str();
		CString humanName = it->second.c_str();
	
		HPROPERTY pr = m_wndParamList.FindProperty(humanName);
		CComVariant vValue;
		pr->GetValue(&vValue);

		serverSettings.params[WCstringToUtf8(name)]= WCstringToUtf8(vValue.bstrVal);	      
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
	serverProfile_.setProfileName(login);
	ServerSettingsStruct& serverSettings = serverProfile_.serverSettings();
	
	CString password = GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT);
	serverSettings.authData.Login = WCstringToUtf8(login);
	serverSettings.authData.Password = WCstringToUtf8(password);
	serverSettings.authData.DoAuth = GuiTools::GetCheck(m_hWnd, IDC_DOAUTH);
	CServerFolderSelect folderSelectDlg(serverProfile_);
	folderSelectDlg.m_SelectedFolder.id = serverProfile_.folderId();

	if ( folderSelectDlg.DoModal() == IDOK ) {
		CFolderItem folder = folderSelectDlg.m_SelectedFolder;
	

		if(!folder.id.empty()){
			serverProfile_.setFolderId(folder.getId());
			serverProfile_.setFolderTitle(folder.getTitle());
			serverProfile_.setFolderUrl(folder.viewUrl);
		} else {
			serverProfile_.setFolderId("");
			serverProfile_.setFolderTitle("");
			serverProfile_.setFolderUrl("");
		}

		SetDlgItemText(IDC_FOLDERNAMELABEL, Utf8ToWCstring( folder.getTitle() ));


	};
	if(!m_pluginLoader)
	{
		return 0;
	}
	m_wndParamList.ResetContent();
	m_pluginLoader->getServerParamList(m_paramNameList);
	
	std::map<std::string,std::string>::iterator it;
	for( it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it)
	{
		CString name = it->first.c_str();
		CString humanName = it->second.c_str();
		m_wndParamList.AddItem( PropCreateSimple(humanName, Utf8ToWCstring(serverSettings.params[WCstringToUtf8(name)])) );
	}

	return 0;
}

LRESULT CServerParamsDlg::OnLoginEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if ( ::GetFocus() == hWndCtl ) {
		serverProfile_.setFolderId("");
		serverProfile_.setFolderTitle("");
		serverProfile_.setFolderUrl("");
		SetDlgItemText(IDC_FOLDERNAMELABEL,  (CString("<") + TR("не выбран") + CString(">")) );

	}
	return 0;
}

ServerProfile CServerParamsDlg::serverProfile()
{
	return serverProfile_;
}
