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

#include "wizarddlg.h"
#include "Gui/GuiTools.h"
#include <Func/Common.h>
#include <Func/MyUtils.h>
#include <Gui/Dialogs/ServerParamsDlg.h>

// CServerSelectorControl
CServerSelectorControl::CServerSelectorControl()
{
		showDefaultServerItem_ = false;
		serversMask_ = smAll;
}

CServerSelectorControl::~CServerSelectorControl()
{
}

void CServerSelectorControl::TranslateUI() {
	TRC(IDC_EDIT, "Параметры");
}
	
LRESULT CServerSelectorControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TranslateUI();
	GuiTools::MakeLabelBold( GetDlgItem( IDC_SERVERGROUPBOX) );

	serverComboBox_.Attach( GetDlgItem( IDC_SERVERCOMBOBOX ) );

	comboBoxImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);

	if ( showDefaultServerItem_ ) {
		serverComboBox_.AddItem(TR("По умолчанию"), -1, -1, 0, reinterpret_cast<LPARAM>( strdup("default") ));
	}
	serverComboBox_.AddItem( _T("<") + CString(TR("Случайный сервер")) + _T(">"), -1, -1, 0, reinterpret_cast<LPARAM>( strdup("random") ) );
	
	CIcon hImageIcon = NULL, hFileIcon = NULL;
	int selectedIndex = 0;

	CUploadEngineData *uploadEngine = _EngineList->byIndex( Settings.ServerID() );
	std::string selectedServerName = "Imageshack.us" ;
	for ( int mask = 1; mask <= 2; mask*=2 ) {
		if ( mask != 1) {
			TCHAR line[40];
			for ( int i=0; i < ARRAY_SIZE(line)-1; i++ ) {
				line[i] = '-';
			}
			line[ARRAY_SIZE(line)-1] = 0;

			serverComboBox_.AddItem(line, -1, -1, 0,  0 );
		}
		for( int i = 0; i < _EngineList->count(); i++) {	
			CUploadEngineData * ue = _EngineList->byIndex( i ); 
			int currentLoopMask = mask & serversMask_;
			if ( ue->ImageHost && !(currentLoopMask & smImageServers) ) {
				continue;
			}
			if ( !ue->ImageHost && !(currentLoopMask & smFileServers) ) {
				continue;
			}
			CIcon hImageIcon = (HICON)LoadImage(0, IU_GetDataFolder() + _T("Favicons\\")+Utf8ToWCstring(ue->Name)
				+_T(".ico"), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
			int nImageIndex = -1;
			if ( !hImageIcon.IsNull() ) {
				nImageIndex = comboBoxImageList_.AddIcon( hImageIcon);
			}
			char *serverName = new char[ue->Name.length() + 1];
			lstrcpyA( serverName, ue->Name.c_str() );
			int itemIndex = serverComboBox_.AddItem( Utf8ToWCstring( ue->Name ), nImageIndex, nImageIndex, 1, reinterpret_cast<LPARAM>( serverName ) );
			if ( ue->Name == selectedServerName ){
				selectedIndex = itemIndex;
			}
		}
		

	}
	serverComboBox_.SetImageList( comboBoxImageList_ );
	serverComboBox_.SetCurSel( selectedIndex );
	

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
	serverParamsDlg.DoModal(m_hWnd);
	serverProfile_ = serverParamsDlg.serverProfile();
	updateInfoLabel();
	return 0;
}

LRESULT CServerSelectorControl::OnServerComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	serverChanged();
	return 0;
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

		if ( serverName != CMyEngineList::DefaultServer && serverName != CMyEngineList::RandomServer ) {
			

			uploadEngineData = _EngineList->byName( serverNameW );
			if ( !uploadEngineData ) {
				return ;
			}

		
			std::map<CString,ServerSettingsStruct>& serverProfiles = Settings.ServersSettings[serverNameW];
			
			if ( serverProfiles.size() ) {
				profileName = serverProfiles.begin()->first;
			}
		}

		serverProfile_.setServerName(serverNameW);
		serverProfile_.setProfileName(profileName);
		
	}
	updateInfoLabel();
}

void CServerSelectorControl::updateInfoLabel() {
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	std::string serverName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );

	bool showServerParams = (serverName != CMyEngineList::DefaultServer && serverName != CMyEngineList::RandomServer );
	GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showServerParams);
	GuiTools::ShowDialogItem(m_hWnd, IDC_EDIT, showServerParams);

	CUploadEngineData* uploadEngineData = _EngineList->byName(Utf8ToWCstring( serverName ));
	if ( ! uploadEngineData ) {
		return;
	}

	if ( uploadEngineData ) {
		showServerParams = uploadEngineData->UsingPlugin || uploadEngineData->NeedAuthorization;
	}

	CString accountInfoText = TR("Prof:") + serverProfile_.profileName() + _T(" ");
	accountInfoText += TR("Учетная запись:") + CString(" ");
	if ( !serverProfile_.serverSettings().authData.DoAuth ) {
		accountInfoText += TR("не задана");
	} else {
		accountInfoText += Utf8ToWCstring( serverProfile_.serverSettings().authData.Login );
	}
	if ( uploadEngineData->SupportsFolders ) {
		CString folderTitle = Utf8ToWCstring( serverProfile_.serverSettings().params["FolderTitle"] );
		accountInfoText += CString(_T("\r\n")) + TR("Папка/альбом:") + _T(" ");
		if ( folderTitle.IsEmpty() ) {
			accountInfoText += TR("не задана");
		} else {
			accountInfoText += folderTitle;
		}
	}
	SetDlgItemText(IDC_ACCOUNTINFO, accountInfoText);
}

void CServerSelectorControl::setShowDefaultServerItem(bool show) {
	showDefaultServerItem_ = show;
}

void CServerSelectorControl::setServersMask(int mask) {
	serversMask_ = mask;
}

LRESULT CServerSelectorControl::OnServerComboEndEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	MessageBox(0);
	/*PNMCBEENDEDIT pnmEditInfo = (PNMCBEENDEDIT) pnmh;
	int serverComboElementIndex = pnmEditInfo->iNewSelection;
	char *lpstrServerName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
	if ( ! lpstrServerName ) {
		return TRUE;
	}*/
	return FALSE;
}