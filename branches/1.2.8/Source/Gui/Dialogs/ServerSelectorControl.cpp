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
		
}

CServerSelectorControl::~CServerSelectorControl()
{
}

void CServerSelectorControl::TranslateUI()
{

}
	
LRESULT CServerSelectorControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TranslateUI();
	GuiTools::MakeLabelBold( GetDlgItem( IDC_SERVERGROUPBOX) );

	serverComboBox_.Attach( GetDlgItem( IDC_SERVERCOMBOBOX ) );

	comboBoxImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);

	serverComboBox_.AddItem( _T("<") + CString(TR("Случайный сервер")) + _T(">"), -1, -1, 0, static_cast<LPARAM>( -1 ) );

	CIcon hImageIcon = NULL, hFileIcon = NULL;
	int selectedIndex = 0;

	CUploadEngineData *uploadEngine = _EngineList->byIndex( Settings.ServerID );
	std::string selectedServerName = "Imageshack.us" ;
	for( int i = 0; i < _EngineList->count(); i++) {	
		CUploadEngineData * ue = _EngineList->byIndex( i ); 
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
	serverChanged();
}

ServerProfile CServerSelectorControl::getServerProfile() const {
	return serverProfile_;
}

LRESULT CServerSelectorControl::OnClickedEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CServerParamsDlg serverParamsDlg(serverProfile_);
	serverParamsDlg.DoModal(m_hWnd);
	serverProfile_ = serverParamsDlg.serverProfile();
	serverChanged();
	return 0;
}

LRESULT CServerSelectorControl::OnServerComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	serverChanged();
	return 0;
}

void CServerSelectorControl::serverChanged() {
	CUploadEngineData * uploadEngineData =  0;
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	if ( serverComboElementIndex > 0 ) {
		int serverIndex = serverComboElementIndex - 1;
		uploadEngineData = _EngineList->byIndex( serverIndex );
		if ( !uploadEngineData ) {
			return ;
		}
		serverProfile_.setServerName(Utf8ToWCstring( uploadEngineData->Name ));
		serverProfile_.setProfileName(CString());
	}
	
	bool showServerParams = (serverComboElementIndex != 0);

	if ( uploadEngineData ) {
		showServerParams = uploadEngineData->UsingPlugin || uploadEngineData->NeedAuthorization;
	}
	GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showServerParams);
	GuiTools::ShowDialogItem(m_hWnd, IDC_EDIT, showServerParams);

	CString accountInfoText;
	accountInfoText = TR("Учетная запись:") + CString(" ");
	if ( !serverProfile_.serverSettings().authData.DoAuth ) {
		accountInfoText += TR("не задана");
	} else {
		accountInfoText += Utf8ToWCstring( serverProfile_.serverSettings().authData.Login );
	}
	accountInfoText += CString(_T("\r\n")) + TR("Папка/альбом: не задана");
	SetDlgItemText(IDC_ACCOUNTINFO, accountInfoText);
}