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

#include "ServerParamsDlg.h"

#include "Func/LangClass.h"
#include "Func/Settings.h"
#include <Func/Myutils.h>
#include "Func/common.h"
#include <Func/Settings.h>
#include <Gui/GuiTools.h>
#include <Gui/Dialogs/ServerFolderSelect.h>
#include <Gui/Dialogs/InputDialog.h>


// CServerParamsDlg
CServerParamsDlg::CServerParamsDlg(const ServerProfile& serverProfile, bool focusOnLoginControl ){
	m_ue  = _EngineList->byName(( serverProfile.serverName() ) );
	serverProfile_ = serverProfile;
	focusOnLoginControl_ =  focusOnLoginControl;
	catchChanges_ = false;
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
	TRC(IDCANCEL, "Отмена");
	kNewProfileName = TR("Новый профиль");
	createToolbar();
	profileListCombobox_.Attach( GetDlgItem(IDC_PROFILELISTCOMBO) );
	SetDlgItemText(IDC_LOGINFRAME, Utf8ToWCstring(m_ue->Name));
	DlgResize_Init();
	CString WindowTitle;
	CString serverName = Utf8ToWCstring(m_ue->Name);
	WindowTitle.Format(TR("Параметры сервера %s"),(LPCTSTR)serverName);
	SetWindowText(WindowTitle);
	
	m_wndParamList.SubclassWindow(GetDlgItem(IDC_PARAMLIST));
	m_wndParamList.SetExtendedListStyle(PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);

	std::map<CString,ServerSettingsStruct>& serverProfiles = Settings.ServersSettings[serverProfile_.serverName()];
	int selectedProfileIndex = 0;

	int i = 0;
	for ( std::map<CString,ServerSettingsStruct>::const_iterator it = serverProfiles.begin(); it != serverProfiles.end(); ++it ) {
		CString profileName  =  it->first;
		CString profileTitle =  it->first;
		
		if ( profileTitle == "" ) {
			profileTitle = TR("Профиль по-умолчанию");
		}
		int newItemIndex = profileListCombobox_.AddString(profileTitle);
		TCHAR* profileNameBuffer = new TCHAR[profileName.GetLength() + 1];
		lstrcpy( profileNameBuffer, profileName );
		profileListCombobox_.SetItemDataPtr(newItemIndex, profileNameBuffer);
		if ( profileName == serverProfile_.profileName() ) {
			selectedProfileIndex = newItemIndex;
			CString message;
			message.Format(_T("%d"), newItemIndex);
			
		}
		i++;
	}
	
	profileListCombobox_.SetCurSel(selectedProfileIndex);


	::EnableWindow(GetDlgItem(IDC_BROWSESERVERFOLDERS), m_ue->SupportsFolders);
	GuiTools::ShowDialogItem(m_hWnd, IDC_DOAUTH, m_ue->NeedAuthorization == CUploadEngineData::naAvailable );
	
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, m_ue->SupportsFolders );
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERNAMELABEL, m_ue->SupportsFolders );
	GuiTools::ShowDialogItem(m_hWnd, IDC_BROWSESERVERFOLDERS, m_ue->SupportsFolders );

	m_wndParamList.EnableWindow( false );
	serverProfile_.setProfileName(getSelectedProfileName());
	doAuthChanged();
	updateDialog(serverProfile_);
	int result = 1;
	if ( focusOnLoginControl_ && m_ue->NeedAuthorization ) {
		GuiTools::SetCheck(m_hWnd, IDC_DOAUTH, true);
		doAuthChanged();
		::SetFocus(GetDlgItem(IDC_LOGINEDIT) );
		SendDlgItemMessage(IDC_LOGINEDIT, EM_SETSEL, 0, -1);
		result = 0;
	}
	
	return result;  // Let the system set the focus
}

LRESULT CServerParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	saveCurrentProfile();
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

void CServerParamsDlg::createToolbar() {
	RECT profileRect = GuiTools::GetDialogItemRect(m_hWnd, IDC_TOOLBARPLACEHOLDER);

	m_ProfileEditToolbar.Create(m_hWnd,profileRect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| TBSTYLE_TOOLTIPS|CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
	m_ProfileEditToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_ProfileEditToolbar.SetButtonStructSize();
	m_ProfileEditToolbar.SetButtonSize(17,17);

	CIcon ico = (HICON)LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONWHITEPAGE));
	CIcon saveIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONSAVE));
	CIcon deleteIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONDELETE));
	CIcon renameIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONRENAME));
	CImageList list;
	list.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	list.AddIcon(ico);
	list.AddIcon(renameIcon);
	list.AddIcon(saveIcon);
	list.AddIcon(deleteIcon);
	m_ProfileEditToolbar.SetImageList(list);
	m_ProfileEditToolbar.AddButton(BUTTON_NEWPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0, TR("Создать профиль"), 0);
	m_ProfileEditToolbar.AddButton(BUTTON_RENAMEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Переименовать профиль"), 0);
	m_ProfileEditToolbar.AddButton(BUTTON_SAVEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR("Сохранить профиль"), 0);
	m_ProfileEditToolbar.AddButton(BUTTON_DELETEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE,  TBSTATE_ENABLED, 3, TR("Удалить профиль"), 0);
}

void CServerParamsDlg::doAuthChanged() {
	::EnableWindow(GetDlgItem(IDC_LOGINEDIT), IS_CHECKED(IDC_DOAUTH) || m_ue->NeedAuthorization == CUploadEngineData::naObligatory);
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), IS_CHECKED(IDC_DOAUTH) || m_ue->NeedAuthorization == CUploadEngineData::naObligatory);
}

ServerProfile CServerParamsDlg::serverProfile() const {
	return serverProfile_;
}

LRESULT CServerParamsDlg::OnBrowseServerFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CServerFolderSelect folderSelectDlg(m_ue);
	folderSelectDlg.m_SelectedFolder.id = serverProfile_.serverSettings().params["FolderID"];
	if ( folderSelectDlg.DoModal() == IDOK ) {
		CFolderItem folder = folderSelectDlg.m_SelectedFolder;
		ServerSettingsStruct& serverSettings = Settings.ServersSettings[serverProfile_.serverName()][serverProfile_.profileName()];
		
		if(!folder.id.empty()){
			serverSettings.params["FolderID"]    = folder.getId();
			serverSettings.params["FolderTitle"] = folder.getTitle();
			serverSettings.params["FolderUrl"]   = folder.viewUrl;
		} else {
			serverSettings.params["FolderID"]    = "";
			serverSettings.params["FolderTitle"] = "";
			serverSettings.params["FolderUrl"]   = "";
		}

		SetDlgItemText(IDC_FOLDERNAMELABEL, Utf8ToWCstring( folder.getTitle() ));

	};
	return 0;
}

LRESULT CServerParamsDlg::OnNewProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CString profileName = TR("Новый профиль");
	int newItemIndex = profileListCombobox_.AddString(profileName);
	CString alias = "new_profile";
	TCHAR* profileNameBuffer = new TCHAR[alias.GetLength() + 1];
	lstrcpy( profileNameBuffer, alias );
	profileListCombobox_.SetItemDataPtr(newItemIndex, profileNameBuffer);

	profileListCombobox_.SetCurSel( newItemIndex );
	serverProfile_ = ServerProfile(serverProfile_.serverName());
	serverProfile_.setProfileName(profileName);
	updateDialog(serverProfile_);

	return 0;
}
LRESULT CServerParamsDlg::OnSaveProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	saveCurrentProfile();
	return 0;
}
LRESULT CServerParamsDlg::OnDeleteProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CString profileName = getSelectedProfileName();
	int currentComboBoxIndex = profileListCombobox_.GetCurSel();

	if ( profileName.IsEmpty() ) { // deleting of default profile is forbidden
		return 0;
	}
	std::map<CString,ServerSettingsStruct>::iterator it;
	std::map<CString,ServerSettingsStruct>& serverProfiles = Settings.ServersSettings[serverProfile_.serverName()];
	if ( (it = serverProfiles.find(profileName)) != serverProfiles.end()){
		serverProfiles.erase(it);
	}
	profileListCombobox_.DeleteString( currentComboBoxIndex );
	serverProfile_.setProfileName(_T(""));
	profileListCombobox_.SetCurSel(0);
	updateDialog(serverProfile_);
	return 0;
}

void CServerParamsDlg::updateDialog(ServerProfile serverProfile) {
	catchChanges_ = false;
	m_wndParamList.ResetContent();
	ServerSettingsStruct& serverSettings = Settings.ServersSettings[serverProfile.serverName()][serverProfile.profileName()];
	m_ProfileEditToolbar.SetButtonInfo(BUTTON_DELETEPROFILE, TBIF_STATE, 0, serverProfile.profileName().IsEmpty() ? 0 : TBSTATE_ENABLED, 0 ,  0, 0,0, 0);
	m_ProfileEditToolbar.SetButtonInfo(BUTTON_RENAMEPROFILE, TBIF_STATE, 0, serverProfile.profileName().IsEmpty() ? 0 : TBSTATE_ENABLED, 0 ,  0, 0,0, 0);
	CString folderTitle =Utf8ToWCstring( serverSettings.params["FolderTitle"] );
	if ( folderTitle.IsEmpty() ) {
		folderTitle = TR("<не выбрано>");
	}
	SetDlgItemText(IDC_FOLDERNAMELABEL, folderTitle);

	LoginInfo li = serverSettings.authData;
	SetDlgItemText(IDC_LOGINEDIT, Utf8ToWCstring(li.Login));
	oldLogin_ =  Utf8ToWCstring(li.Login);
	SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));

	SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, (li.DoAuth ? BST_CHECKED : BST_UNCHECKED));

	CScriptUploadEngine *m_pluginLoader = iuPluginManager.getPlugin(m_ue->PluginName, Settings.ServerByUtf8Name(m_ue->Name));
	if ( m_pluginLoader ) {
		m_pluginLoader->getServerParamList(m_paramNameList);
		
		std::map<std::string,std::string>::iterator it;
		for( it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it)
		{
			CString name = it->first.c_str();
			CString humanName = it->second.c_str();
			m_wndParamList.AddItem( PropCreateSimple(humanName, Utf8ToWCstring(serverSettings.params[WCstringToUtf8(name)])) );
		}
		m_wndParamList.EnableWindow( m_paramNameList.size() != 0 );
	}

	doAuthChanged();
	catchChanges_ = true;
}

LRESULT CServerParamsDlg::OnRenameProfile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CInputDialog inputDialog(TR("Переименование профиля"), TR("Название профиля"), serverProfile_.profileName());
	if ( inputDialog.DoModal() == IDOK ) {
		renameCurrentProfile(inputDialog.getValue());
	} 
	return 0;
}

void CServerParamsDlg::renameCurrentProfile(CString newProfileName) {
	CString oldProfileName = serverProfile_.profileName();
	/*std::map<CString,ServerSettingsStruct>& serverProfiles = Settings.ServersSettings[serverProfile_.serverName()];
	std::map<CString,ServerSettingsStruct>::iterator		it;

	if ( (it = serverProfiles.find(oldProfileName)) != serverProfiles.end()){
		serverProfiles.insert(std::map<CString,ServerSettingsStruct>::value_type(newProfileName, it->second));
		serverProfiles.erase(it);
	}*/

	serverProfile_.setProfileName(newProfileName);
	int selectedProfileIndex = profileListCombobox_.GetCurSel();
	TCHAR* oldProfileNameBuffer = reinterpret_cast<TCHAR*>( profileListCombobox_.GetItemDataPtr(selectedProfileIndex) );
	//delete[] oldProfileNameBuffer;
	profileListCombobox_.DeleteString(selectedProfileIndex);

	//TCHAR* profileNameBuffer = new TCHAR[serverProfile_.profileName().GetLength()+1];
	//lstrcpy( profileNameBuffer, serverProfile_.profileName() );

	int newIndex = profileListCombobox_.InsertString(selectedProfileIndex, serverProfile_.profileName() );
	profileListCombobox_.SetItemDataPtr(newIndex, /*profileNameBuffer*/oldProfileNameBuffer );
	profileListCombobox_.SetCurSel(newIndex);
}

LRESULT CServerParamsDlg::OnLoginEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if ( !catchChanges_ ) {
		return 0;
	}
	updateCurrentProfileName();
	return 0;
}

void CServerParamsDlg::updateCurrentProfileName() {
	CString oldProfileName = serverProfile_.profileName();
	CString login = GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT);
	CString newProfileName = login;

	if ( m_ue->Name == "FTP Server" ) {
		std::map<std::string,std::string>::iterator it;
		CString hostName;
		for(it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it) {
			CString name      = it->first.c_str();
			CString humanName = it->second.c_str();
			HPROPERTY pr = m_wndParamList.FindProperty(humanName);
			CComVariant vValue;
			pr->GetValue(&vValue);
			if ( name == "hostname") {
				hostName = vValue.bstrVal;
				break;
			}
		}

		if ( !hostName.IsEmpty() ) {
			newProfileName = login + _T("@") + hostName;
		} 
	} 
	if ( ( oldProfileName == kNewProfileName 
		|| ( !oldProfileName.IsEmpty() && oldProfileName == oldLogin_ ) ) ) {
			renameCurrentProfile( login.IsEmpty() ? kNewProfileName : newProfileName );
	}
	oldLogin_ = newProfileName;
}

LRESULT  CServerParamsDlg::OnProfileListSelectChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int selectedProfileIndex = profileListCombobox_.GetCurSel();
	if ( selectedProfileIndex < 0 ) {
		return 0;
	}
	TCHAR* profileName = reinterpret_cast<TCHAR*>( profileListCombobox_.GetItemDataPtr(selectedProfileIndex) );
	serverProfile_.setProfileName(profileName);
	updateDialog(serverProfile_);
	return 0;
}

LRESULT CServerParamsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int profileCount = profileListCombobox_.GetCount();
	for ( int i = 0; i < profileCount; i++ ) {
		TCHAR* profileNameBuffer = reinterpret_cast<TCHAR*>( profileListCombobox_.GetItemDataPtr(i) );
		delete[] profileNameBuffer;
	}
	return 0;
}

CString CServerParamsDlg::getSelectedProfileName() {
	int selectedProfileIndex = profileListCombobox_.GetCurSel();
	if ( selectedProfileIndex < 0 ) {
		return CString();
	}
	TCHAR* profileName = reinterpret_cast<TCHAR*>( profileListCombobox_.GetItemDataPtr(selectedProfileIndex) );
	return profileName;
}

LRESULT CServerParamsDlg::OnParamsListItemChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	updateCurrentProfileName();
	return 0;
}

void  CServerParamsDlg::saveCurrentProfile() {
	CString currentProfile = getSelectedProfileName();

	CString newProfileName;

	int curProfileIndex = profileListCombobox_.GetCurSel();
	profileListCombobox_.GetLBText( curProfileIndex, newProfileName );

	if ( newProfileName != currentProfile || (currentProfile == "new_profile") ) {
		CString oldProfileName = currentProfile;
		std::map<CString,ServerSettingsStruct>& serverProfiles = Settings.ServersSettings[serverProfile_.serverName()];
		std::map<CString,ServerSettingsStruct>::iterator		it;

		if ( (it = serverProfiles.find(oldProfileName)) != serverProfiles.end()){
			serverProfiles.insert(std::map<CString,ServerSettingsStruct>::value_type(newProfileName, it->second));
			serverProfiles.erase(it);
		}

		serverProfile_.setProfileName(newProfileName);
		int selectedProfileIndex = profileListCombobox_.GetCurSel();
		TCHAR* oldProfileNameBuffer = reinterpret_cast<TCHAR*>( profileListCombobox_.GetItemDataPtr(selectedProfileIndex) );
		delete[] oldProfileNameBuffer;
		profileListCombobox_.DeleteString(selectedProfileIndex);

		TCHAR* profileNameBuffer = new TCHAR[serverProfile_.profileName().GetLength()+1];
		lstrcpy( profileNameBuffer, serverProfile_.profileName() );

		int newIndex = profileListCombobox_.InsertString(selectedProfileIndex, serverProfile_.profileName() );
		profileListCombobox_.SetItemDataPtr(newIndex, profileNameBuffer );
		profileListCombobox_.SetCurSel(newIndex);
		currentProfile = newProfileName;
	}

	ServerSettingsStruct& serverSettings = Settings.ServersSettings[serverProfile_.serverName()][currentProfile];
	std::map<std::string,std::string>::iterator it;
	for(it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it) {
		CString name      = it->first.c_str();
		CString humanName = it->second.c_str();
		HPROPERTY pr = m_wndParamList.FindProperty(humanName);
		CComVariant vValue;
		pr->GetValue(&vValue);
		serverSettings.params[WCstringToUtf8(name)] = WCstringToUtf8(vValue.bstrVal);	      
	}
	serverProfile_.setProfileName( currentProfile );

	serverSettings.authData.DoAuth = GuiTools::GetCheck(m_hWnd, IDC_DOAUTH);
	serverSettings.authData.Login    = WCstringToUtf8( GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT) );
	serverSettings.authData.Password = WCstringToUtf8( GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT) );
}