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
	::EnableWindow(GetDlgItem(IDC_LOGINEDIT), IS_CHECKED(IDC_DOAUTH) || m_ue->NeedAuthorization == CUploadEngineData::naObligatory);
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), (IS_CHECKED(IDC_DOAUTH) || m_ue->NeedAuthorization == CUploadEngineData::naObligatory) && m_ue->NeedPassword);
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
