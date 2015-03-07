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

#include "ServerSelectorControl.h"
#include <uxtheme.h>

#include "Gui/Dialogs/wizarddlg.h"
#include "Gui/GuiTools.h"
#include <Func/Common.h>
#include <Func/MyUtils.h>
#include <Gui/Dialogs/ServerParamsDlg.h>
#include <Gui/Dialogs/UploadParamsDlg.h>
#include <Func/WinUtils.h>
#include <Gui/IconBitmapUtils.h>
#include <Gui/Dialogs/LoginDlg.h>
#include <Gui/Dialogs/AddFtpServerDialog.h>
#include <Gui/Dialogs/AddDirectoryServerDialog.h>

const char CServerSelectorControl::kAddFtpServer[]=("<add_ftp_server>");
const char CServerSelectorControl::kAddDirectoryAsServer[]=("<add_directory_as_server>");
// CServerSelectorControl
CServerSelectorControl::CServerSelectorControl(bool defaultServer)
{
		showDefaultServerItem_ = false;
		serversMask_ = smImageServers | smFileServers;
		showImageProcessingParamsLink_ = true;
		defaultServer_ = defaultServer;
		iconBitmapUtils_ = new IconBitmapUtils();
}

CServerSelectorControl::~CServerSelectorControl()
{
	delete iconBitmapUtils_;
}

void CServerSelectorControl::TranslateUI() {
	TRC(IDC_EDIT, "Параметры");
}
	
LRESULT CServerSelectorControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TranslateUI();

	imageProcessingParamsLink_.SubclassWindow(GetDlgItem(IDC_IMAGEPROCESSINGPARAMS));
	imageProcessingParamsLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
	imageProcessingParamsLink_.m_clrLink = CSettings::DefaultLinkColor;
	imageProcessingParamsLink_.SetLabel(TR("Обработка изображений..."));

	accountLink_.SubclassWindow(GetDlgItem(IDC_ACCOUNTINFO));
	accountLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON ; 
	accountLink_.m_clrLink = CSettings::DefaultLinkColor;
	accountLink_.SetToolTipText(TR("Имя пользователя"));

	createSettingsButton();

	GuiTools::MakeLabelBold( GetDlgItem( IDC_SERVERGROUPBOX) );
	CIcon deleteIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONDELETE));
	serverComboBox_.Attach( GetDlgItem( IDC_SERVERCOMBOBOX ) );

	updateServerList();
	GuiTools::ShowDialogItem(m_hWnd, IDC_IMAGEPROCESSINGPARAMS, showImageProcessingParamsLink_);

	return 1;  // Let the system set the focus
}

void CServerSelectorControl::setTitle(CString title) {
	SetDlgItemText(IDC_SERVERGROUPBOX, title);
}

void CServerSelectorControl::setServerProfile(ServerProfile serverProfile) {
	serverProfile_ = serverProfile;

	int comboboxItemIndex = serverComboBox_.FindStringExact(-1, serverProfile.serverName());

	if ( comboboxItemIndex == CB_ERR) {
		serverComboBox_.SetCurSel(0); //random server
	} else {
		serverComboBox_.SetCurSel(comboboxItemIndex);
	}
	updateInfoLabel();
}

ServerProfile CServerSelectorControl::serverProfile() const {
	return serverProfile_;
}

LRESULT CServerSelectorControl::OnClickedEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CServerParamsDlg serverParamsDlg(serverProfile_);
	if ( serverParamsDlg.DoModal(m_hWnd) == IDOK ) {
		serverProfile_ = serverParamsDlg.serverProfile();
		notifyChange();
		
	}

	updateInfoLabel();
	return 0;
}

LRESULT CServerSelectorControl::OnServerComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	serverChanged();
	return 0;
}

void CServerSelectorControl::addAccount()
{
	ServerProfile serverProfileCopy = serverProfile_;
	serverProfileCopy.setProfileName("");
	CLoginDlg dlg(serverProfileCopy, true);

	if( dlg.DoModal(m_hWnd) == IDOK)
	{
		serverProfileCopy.setProfileName(dlg.accountName());
		serverProfileCopy.setFolderId("");
		serverProfileCopy.setFolderTitle("");
		serverProfileCopy.setFolderUrl("");

		serverProfile_ = serverProfileCopy;
		updateInfoLabel();
		notifyChange();
	}
}

void CServerSelectorControl::serverChanged() {
	//return;
	CUploadEngineData * uploadEngineData =  0;
	CString profileName;
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	char *lpstrServerName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );

	if ( !lpstrServerName ) {
		serverComboElementIndex++;
		serverComboBox_.SetCurSel(serverComboElementIndex);
		lpstrServerName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
	}
	if ( serverComboElementIndex > 0 && lpstrServerName ) {
		std::string serverName = lpstrServerName;
		CString serverNameW = Utf8ToWCstring( serverName );
		serverProfile_.setServerName(serverNameW);
		if ( serverName == kAddFtpServer ) {
			CAddFtpServerDialog dlg(_EngineList);
			if ( dlg.DoModal(m_hWnd) == IDOK ) {
				serverProfile_ = ServerProfile();
				serverProfile_.setServerName(dlg.createdServerName());
				serverProfile_.setProfileName(dlg.createdServerLogin());
				serverProfile_.clearFolderInfo();
				notifyServerListChanged();

			}
		} else if (serverName == kAddDirectoryAsServer  ) {
			CAddDirectoryServerDialog dlg(_EngineList);
			if ( dlg.DoModal(m_hWnd) == IDOK ) {
				serverProfile_ = ServerProfile();
				serverProfile_.setServerName(dlg.createdServerName());
				serverProfile_.setProfileName("");
				serverProfile_.clearFolderInfo();
				notifyServerListChanged();
			}
		} else if ( serverName != CMyEngineList::DefaultServer && serverName != CMyEngineList::RandomServer ) {
			uploadEngineData = _EngineList->byName( serverNameW );
			if ( !uploadEngineData ) {
				return ;
			}

		
			//std::map<CString,ServerSettingsStruct> serverProfiles;
			//serverProfiles[serverNameW]= Settings.ServersSettings[serverName];
		
		//	ShowVar((int)Settings.ServersSettings[serverName].size());
			
			if ( Settings.ServersSettings[serverName].size() ) {


				std::map <std::string, ServerSettingsStruct>& ss = Settings.ServersSettings[serverName];
				std::map <std::string, ServerSettingsStruct>::iterator it = ss.begin();
				if ( it->first == "" ) {
					++it;
				}
				if ( it!= ss.end() ) {
					ServerSettingsStruct & s = it->second;
					profileName = Utf8ToWCstring(s.authData.Login);
					serverProfile_.setProfileName(profileName);
					serverProfile_.setFolderId(s.defaultFolder.getId());
					serverProfile_.setFolderTitle(s.defaultFolder.getTitle());
					serverProfile_.setFolderUrl(s.defaultFolder.viewUrl);
				} else {
					serverProfile_.setProfileName("");
					serverProfile_.setFolderId("");
					serverProfile_.setFolderTitle("");
					serverProfile_.setFolderUrl("");
				}
				
			}
		}

	
		
	}

	notifyChange();

	updateInfoLabel();
}

void CServerSelectorControl::updateInfoLabel() {
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	std::string serverName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
	currentUserName_.Empty();

	bool showServerParams = (serverName != CMyEngineList::DefaultServer && serverName != CMyEngineList::RandomServer );

	if (  !showServerParams ) {
		GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, showServerParams );
		GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERICON, showServerParams );
		GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showServerParams );
		GuiTools::ShowDialogItem(m_hWnd, IDC_USERICON, showServerParams );
		settingsButtonToolbar_.ShowWindow(SW_HIDE);
		return;
	}
	//GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showServerParams);
//	GuiTools::ShowDialogItem(m_hWnd, IDC_EDIT, showServerParams);

	CUploadEngineData* uploadEngineData = _EngineList->byName(Utf8ToWCstring( serverName ));
	if ( ! uploadEngineData ) {
		return;
	}

	if ( uploadEngineData ) {
		showServerParams = showServerParams && (uploadEngineData->UsingPlugin || uploadEngineData->NeedAuthorization);
	}


	CString accountInfoText;// = TR("Prof:") + serverProfile_.profileName() + _T(" ");
	LoginInfo loginInfo = serverProfile_.serverSettings().authData;
	//accountInfoText += TR("Учетная запись:") + CString(" ");
	
	if ( loginInfo.Login.empty() || (!loginInfo.DoAuth  && uploadEngineData->NeedAuthorization != 2 ) ) {
		accountInfoText += TR("Учетная запись...");
		accountLink_.SetToolTipText(TR("Ввести данные учетной записи"));
	} else {
		accountInfoText += Utf8ToWCstring( serverProfile_.serverSettings().authData.Login );
		currentUserName_  =  Utf8ToWCstring( serverProfile_.serverSettings().authData.Login );
	}
	CString folderTitle;
	if ( uploadEngineData->SupportsFolders ) {
		 folderTitle = Utf8ToWCstring( serverProfile_.folderTitle() );
		//accountInfoText += CString(_T("\r\n")) + TR("Папка/альбом:") + _T(" ");
		/*if ( folderTitle.IsEmpty() ) {
			folderTitle = TR("не задана");
		} */
		
	}
	SetDlgItemText(IDC_ACCOUNTINFO, accountInfoText);
	bool showAccount = uploadEngineData->NeedAuthorization != 0 && showServerParams;
	GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showAccount);
	accountLink_.SetLabel(accountInfoText);
	SetDlgItemText(IDC_FOLDERLABEL, folderTitle);
	
	GuiTools::ShowDialogItem(m_hWnd, IDC_USERICON, showAccount);
	
	bool showFolder = !folderTitle.IsEmpty() && showServerParams;

	RECT rect;
	int settingsBtnPlaceHolderId = ( showFolder || showAccount ) ? IDC_SETTINGSBUTTONPLACEHOLDER : IDC_SETTINGSBUTTONPLACEHOLDER2;
	::GetWindowRect(GetDlgItem(settingsBtnPlaceHolderId), &rect);
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&rect, 2);
	settingsButtonToolbar_.SetWindowPos(0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
	settingsButtonToolbar_.ShowWindow(showServerParams ? SW_SHOW : SW_HIDE);

	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, showFolder );
	GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERICON, showFolder );
	RECT accountLabelRect = GuiTools::AutoSizeStaticControl(GetDlgItem(IDC_ACCOUNTINFO));
	int folderIconX = accountLabelRect.right + GuiTools::dlgX(10);
	//::SetWindowPos(GetDlgItem(IDC_FOLDERICON), 0, folderIconX, accountLabelRect.top, 0, 0, SWP_NOSIZE );
	//::SetWindowPos(GetDlgItem(IDC_FOLDERLABEL), 0, folderIconX + 16 + GuiTools::dlgX(3), accountLabelRect.top, 0, 0, SWP_NOSIZE );
}

void CServerSelectorControl::setShowDefaultServerItem(bool show) {
	showDefaultServerItem_ = show;
}

void CServerSelectorControl::setServersMask(int mask) {
	serversMask_ = mask;
}

void CServerSelectorControl::notifyChange()
{
	::SendMessage(GetParent(), WM_SERVERSELECTCONTROL_CHANGE, (WPARAM)m_hWnd, 0);
}

void CServerSelectorControl::notifyServerListChanged()
{
	::SendMessage(GetParent(), WM_SERVERSELECTCONTROL_SERVERLIST_CHANGED, (WPARAM)m_hWnd, 0);
}

void CServerSelectorControl::updateServerList()
{
	serverComboBox_.ResetContent();
	comboBoxImageList_.Destroy();
	comboBoxImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	

	if ( showDefaultServerItem_ ) {
		serverComboBox_.AddItem(TR("По умолчанию"), -1, -1, 0, reinterpret_cast<LPARAM>( strdup("default") ));
	}
	serverComboBox_.AddItem( /*_T("<") +*/ CString(TR("")) /*+ _T(">")*/, -1, -1, 0, reinterpret_cast<LPARAM>( strdup("random") ) );

	CIcon hImageIcon = NULL, hFileIcon = NULL;
	int selectedIndex = 0;
	int addedItems = 0;
	std::string selectedServerName = WCstringToUtf8(serverProfile_.serverName());
	TCHAR line[40];
	for ( int i=0; i < ARRAY_SIZE(line)-1; i++ ) {
		line[i] = '-';
	}
	line[ARRAY_SIZE(line)-1] = 0;
	for ( int mask = 1; mask <= 4; mask*=2 ) {
		int currentLoopMask = mask & serversMask_;
		if ( addedItems && currentLoopMask ) {
			serverComboBox_.AddItem(line, -1, -1, 0,  0 );
		}
		for( int i = 0; i < _EngineList->count(); i++) {	
			CUploadEngineData * ue = _EngineList->byIndex( i ); 

			if ( serversMask_ != smUrlShorteners && ue->Type != CUploadEngineData::TypeFileServer && ue->Type != CUploadEngineData::TypeImageServer) {
				continue;
			}
			if ( ue->Type == CUploadEngineData::TypeImageServer && !(currentLoopMask & smImageServers) ) {
				continue;
			}
			if ( ue->Type == CUploadEngineData::TypeFileServer && !(currentLoopMask & smFileServers) ) {
				continue;
			}

			if ( ue->Type == CUploadEngineData::TypeUrlShorteningServer && !(currentLoopMask & smUrlShorteners) ) {
				continue;
			}
			HICON hImageIcon = _EngineList->getIconForServer(ue->Name);
			int nImageIndex = -1;
			if ( hImageIcon ) {
				nImageIndex = comboBoxImageList_.AddIcon( hImageIcon);
			}
			char *serverName = new char[ue->Name.length() + 1];
			lstrcpyA( serverName, ue->Name.c_str() );
			int itemIndex = serverComboBox_.AddItem( Utf8ToWCstring( ue->Name ), nImageIndex, nImageIndex, 1, reinterpret_cast<LPARAM>( serverName ) );
			if ( ue->Name == selectedServerName ){
				selectedIndex = itemIndex;
			}
			addedItems++;
		}
	}
	serverComboBox_.AddItem(line, -1, -1, 0,  0 );
	serverComboBox_.AddItem(  TR("Добавить FTP сервер..."), -1, -1, 1, reinterpret_cast<LPARAM>( kAddFtpServer ) );
	serverComboBox_.AddItem(  TR("Добавить локальную папку как сервер..."), -1, -1, 1, reinterpret_cast<LPARAM>( kAddDirectoryAsServer ) );


	serverComboBox_.SetImageList( comboBoxImageList_ );
	serverComboBox_.SetCurSel( selectedIndex );
	serverChanged();
}

LRESULT CServerSelectorControl::OnAccountClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	
	CMenu sub;	
	MENUITEMINFO mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_TYPE|MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
	CUploadEngineData* uploadEngine = serverProfile_.uploadEngineData();



		std::map <std::string, ServerSettingsStruct>& serverUsers = Settings.ServersSettings[WCstringToUtf8(serverProfile_.serverName())];

		if(/*uploadEngine->UsingPlugin &&*/ (serverUsers.size()>1 || serverUsers.find("") == serverUsers.end()) )
		{
			bool addedSeparator = false;
			
			int i =0;
			//ShowVar((int)serverUsers.size() );
			if ( serverUsers.size() && !serverProfile_.profileName().IsEmpty()) {
				mi.wID = IDC_LOGINMENUITEM;
				mi.dwTypeData  = TR("Изменить данные учетной записи");
				sub.InsertMenuItem(i++, true, &mi);
			} else {
				addedSeparator = true;
			}

			menuOpenedUserNames_.clear();

			int command = IDC_USERNAME_FIRST_ID;
			HICON userIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONUSER));
			
			for( std::map <std::string, ServerSettingsStruct>::iterator it = serverUsers.begin(); it!= serverUsers.end(); ++it ) {
				//	CString login = Utf8ToWCstring(it->second.authData.Login);
				CString login = Utf8ToWCstring(it->first);
				if (!login.IsEmpty() )/*&& it->second.authData.DoAuth**/ {
					if ( !addedSeparator ) {
						ZeroMemory(&mi,sizeof(mi));
						mi.cbSize = sizeof(mi);
						mi.fMask = MIIM_TYPE|MIIM_ID;
						mi.wID =  1;
						mi.fType = MFT_SEPARATOR;

						sub.InsertMenuItem(i++, true, &mi);
						addedSeparator =  true;
					}
					ZeroMemory(&mi,sizeof(mi));
					mi.cbSize = sizeof(mi);

					mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
					mi.fType = MFT_STRING;
					mi.wID = command;

					mi.dwTypeData  = (LPWSTR)(LPCTSTR)login;

					mi.hbmpItem =  WinUtils::IsVista() ? iconBitmapUtils_->HIconToBitmapPARGB32(userIcon): HBMMENU_CALLBACK;
					if ( mi.hbmpItem ) {
						mi.fMask |= MIIM_BITMAP;
					}
					menuOpenedUserNames_.push_back(login);
					sub.InsertMenuItem(i++, true, &mi);
					command++;
				}

			}
			if ( uploadEngine->NeedAuthorization != CUploadEngineData::naObligatory ) {
				ZeroMemory(&mi,sizeof(mi));
				mi.cbSize = sizeof(mi);
				mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
				mi.fType = MFT_STRING;
				mi.wID = IDC_NO_ACCOUNT;

				mi.dwTypeData  = (LPWSTR)(LPCTSTR)TR("<без авторизации>");
				sub.InsertMenuItem(i++, true, &mi);
			}



			ZeroMemory(&mi,sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_TYPE|MIIM_ID;
			mi.wID = 1;
			mi.fType = MFT_SEPARATOR;


			sub.InsertMenuItem(i++, true, &mi);


			ZeroMemory(&mi,sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
			mi.fType = MFT_STRING;
			mi.wID = IDC_ADD_ACCOUNT ;

			mi.dwTypeData  = (LPWSTR)(LPCTSTR)TR("Добавить учетную запись...");


			sub.InsertMenuItem(i++, true, &mi);
			sub.SetMenuDefaultItem(0,TRUE);
		} else {
			addAccount();
			return 0;
		}
		
		RECT rc={0,0,0,0};
		::GetClientRect(GetDlgItem(IDC_ACCOUNTINFO), &rc);
		::ClientToScreen(GetDlgItem(IDC_ACCOUNTINFO), (LPPOINT)&rc);
		::ClientToScreen(GetDlgItem(IDC_ACCOUNTINFO), 1+(LPPOINT)&rc);

	TPMPARAMS excludeArea;
	ZeroMemory(&excludeArea, sizeof(excludeArea));
	excludeArea.cbSize = sizeof(excludeArea);
	excludeArea.rcExclude = rc;
	sub.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
	updateInfoLabel();
	return 0;
}

LRESULT CServerSelectorControl::OnAddAccountClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	addAccount();
	return 0;
}

LRESULT CServerSelectorControl::OnLoginMenuItemClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ServerSettingsStruct  ss = serverProfile_.serverSettings();
	std::string UserName = ss.authData.Login; 
	//bool prevAuthEnabled = ss.authData.DoAuth;
	ServerProfile copy = serverProfile_;
	CLoginDlg dlg(copy);
	
	if( dlg.DoModal(m_hWnd) == IDOK)
	{
		copy.setProfileName(dlg.accountName());
		if(Utf8ToWCstring(UserName) != dlg.accountName())
		{
			
			copy.setFolderId("");
			copy.setFolderTitle("");
			copy.setFolderUrl("");
			serverProfile_ = copy;
			//iuPluginManager.UnloadPlugins();
			//_EngineList->DestroyCachedEngine(WCstringToUtf8(serverProfile_.serverName()), WCstringToUtf8(serverProfile_.profileName()));
		}
		notifyChange();
		updateInfoLabel();
	}
	return 0;
}

LRESULT CServerSelectorControl::OnNoAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	serverProfile_.setProfileName("");
	serverProfile_.setFolderId("");
	serverProfile_.setFolderTitle("");
	serverProfile_.setFolderUrl("");
	updateInfoLabel();
	notifyChange();
	return 0;
}

void CServerSelectorControl::createSettingsButton() {
	CIcon ico = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONSETTINGS2), IMAGE_ICON	, 16,16,0);
	RECT profileRect;
	::GetWindowRect(GetDlgItem(IDC_SETTINGSBUTTONPLACEHOLDER), &profileRect);
	::MapWindowPoints(0, m_hWnd, (LPPOINT)&profileRect, 2);

	settingsButtonToolbar_.Create(m_hWnd,profileRect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|CCS_RIGHT|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/TBSTYLE_TOOLTIPS|CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
	settingsButtonToolbar_.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	settingsButtonToolbar_.SetButtonStructSize();
	settingsButtonToolbar_.SetButtonSize(17,17);

	CImageList list;
	list.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	list.AddIcon(ico);
	settingsButtonToolbar_.SetImageList(list);
	settingsButtonToolbar_.AddButton(IDC_EDIT, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0,TR("Настройки сервера и параметры авторизации"), 0);
}

void CServerSelectorControl::setShowImageProcessingParamsLink(bool show) {
	showImageProcessingParamsLink_ = show;
}

LRESULT CServerSelectorControl::OnImageProcessingParamsClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	std::string serverName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
	CUploadEngineData* uploadEngineData = _EngineList->byName(Utf8ToWCstring( serverName ));
	CUploadParamsDlg dlg(serverProfile_, defaultServer_);
	if ( dlg.DoModal(m_hWnd) == IDOK) {
		serverProfile_.setImageUploadParams(dlg.imageUploadParams());
	}
	return 0;
}

LRESULT CServerSelectorControl::OnUserNameMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int userNameIndex = wID - IDC_USERNAME_FIRST_ID;
	CString userName = menuOpenedUserNames_[userNameIndex];
	serverProfile_.setProfileName(userName);
	ServerSettingsStruct& sss = serverProfile_.serverSettings();

	serverProfile_.setFolderId(sss.defaultFolder.getId());
	serverProfile_.setFolderTitle(sss.defaultFolder.getTitle());
	serverProfile_.setFolderUrl(sss.defaultFolder.viewUrl);

	notifyChange();
	updateInfoLabel();
	return 0;
}
