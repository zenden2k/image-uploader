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

#include "QuickSetupDlg.h"

#include "Func/Common.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
#include <Func/Settings.h>
#include <Func/MyUtils.h>
#include <Gui/Dialogs/ServerParamsDlg.h>


// CQuickSetupDlg
CQuickSetupDlg::CQuickSetupDlg() {
}

CQuickSetupDlg::~CQuickSetupDlg() {
}


LRESULT CQuickSetupDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
	TRC(IDC_FTPSETTINGSBUTTON, "Настройки FTP");
	TRC(IDOK, "Продолжить");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_SERVERLABEL, "На какой сервер будем загружать картинки?");
	TRC(IDC_AUTOSTARTUPCHECKBOX, "Запуск программы при старте Windows");
	TRC(IDC_CAPTUREPRINTSCREENCHECKBOX, "Перехватывать нажатия PrintScreen и Alt+PrintScreen");
	TRC(IDC_EXPLORERINTEGRATION, "Добавить пункт в контекстное меню проводника Windows");
	SetWindowText( APPNAME );
	CString titleText;
	titleText.Format(TR("%s - быстрая настройка"), APPNAME );
	SetDlgItemText(IDC_TITLE, titleText );

	CenterWindow();
	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	serverComboBox_.m_hWnd = GetDlgItem( IDC_SERVERCOMBOBOX );
	SendDlgItemMessage( IDC_AUTOSTARTUPCHECKBOX, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage( IDC_CAPTUREPRINTSCREENCHECKBOX, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage( IDC_EXPLORERINTEGRATION, BM_SETCHECK, BST_CHECKED, 0);
	LogoImage.SubclassWindow(GetDlgItem( IDC_STATICLOGO ) );
	LogoImage.LoadImage(0, 0, IDR_PNG1, false, GetSysColor(COLOR_BTNFACE));

	HFONT font = GetFont();
	LOGFONT alf;

	bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

	if(ok)
	{
		alf.lfWeight = FW_BOLD;

		NewFont=CreateFontIndirect(&alf);

		HDC dc = ::GetDC(0);
		alf.lfHeight  =  - MulDiv(11, GetDeviceCaps(dc, LOGPIXELSY), 72);
		ReleaseDC(dc);
		NewFont = CreateFontIndirect(&alf);
		SendDlgItemMessage(IDC_TITLE,WM_SETFONT,(WPARAM)(HFONT)NewFont,MAKELPARAM(false, 0));
	}
	serverComboBox_.Attach( GetDlgItem( IDC_SERVERCOMBOBOX ) );

	comboBoxImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);

	serverComboBox_.AddItem( _T("<") + CString(TR("Случайный сервер")) + _T(">"), -1, -1, 0, static_cast<LPARAM>( -1 ) );

	CIcon hImageIcon = NULL, hFileIcon = NULL;
	int selectedIndex = 0;

	CUploadEngineData *uploadEngine = _EngineList->byIndex( Settings.ServerID() );
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

	doAuthCheckboxChanged();
	
	serverChanged();

	return 1;  
}

LRESULT CQuickSetupDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	GuiTools::GetCheck(m_hWnd, IDC_AUTOSTARTUPCHECKBOX, Settings.AutoStartup);
	Settings.AutoStartup_changed = Settings.AutoStartup;
	Settings.ShowTrayIcon        = true;
	Settings.ExplorerContextMenu = GuiTools::GetCheck(m_hWnd, IDC_EXPLORERINTEGRATION);
	Settings.ExplorerContextMenu_changed = Settings.ExplorerContextMenu;
	Settings.SaveSettings();

	bool capturePrintScreen = GuiTools::GetCheck( m_hWnd, IDC_CAPTUREPRINTSCREENCHECKBOX );
	if ( capturePrintScreen ) {
		Settings.Hotkeys.getByFunc( _T("fullscreenshot") ).globalKey.DeSerialize("44");
		Settings.Hotkeys.getByFunc( _T("windowscreenshot") ).globalKey.DeSerialize("65580");
	} else {
		Settings.Hotkeys.getByFunc( _T("fullscreenshot") ).Clear();
		Settings.Hotkeys.getByFunc( _T("windowscreenshot") ).Clear();

	}

	int serverComboElementIndex = serverComboBox_.GetCurSel();
	if ( serverComboElementIndex > 0 ) {
		int serverIndex = serverComboElementIndex - 1;
		CUploadEngineData * uploadEngineData = _EngineList->byIndex( serverIndex );
		Settings.imageServer.setServerName( Utf8ToWCstring( uploadEngineData->Name ) );
		bool needAuth = GuiTools::GetCheck( m_hWnd, IDC_DOAUTHCHECKBOX );
		if ( needAuth ) {
			CString login = GuiTools::GetDlgItemText( m_hWnd, IDC_LOGINEDIT );
			CString password = GuiTools::GetDlgItemText( m_hWnd, IDC_PASSWORDEDIT );
			if ( login.IsEmpty() || password.IsEmpty() ) {
				MessageBox(TR("Введите данные учетной записи"), APPNAME, MB_ICONEXCLAMATION);
				return 0;
			}
			LoginInfo& loginInfo = Settings.ServersSettings[Settings.ServerName()][_T("")].authData;
			loginInfo.DoAuth = true;
			loginInfo.Login = WCstringToUtf8( login );
			loginInfo.Password = WCstringToUtf8( password );
		}
	} else {
		Settings.imageServer.setServerName(CString());
	}

	
	EndDialog(wID);
	return 0;
}

LRESULT CQuickSetupDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	EndDialog(wID);
	return 0;
}

LRESULT CQuickSetupDlg::OnClickedDoAuthCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	doAuthCheckboxChanged();
	return 0;
}

void CQuickSetupDlg::doAuthCheckboxChanged() {
	bool isDoAuthChecked = SendDlgItemMessage(IDC_DOAUTHCHECKBOX, BM_GETCHECK) == BST_CHECKED;
	GuiTools::EnableNextN( GetDlgItem( IDC_DOAUTHCHECKBOX) , 4, isDoAuthChecked);
}

void CQuickSetupDlg::showAuthorizationControls(bool show) {
	int cmdShow = show ? SW_SHOW : SW_HIDE;
	::ShowWindow( GetDlgItem( IDC_DOAUTHCHECKBOX), cmdShow);
	::ShowWindow( GetDlgItem( IDC_LOGINLABEL), cmdShow);
	::ShowWindow( GetDlgItem( IDC_LOGINEDIT), cmdShow);
	::ShowWindow( GetDlgItem( IDC_PASSWORDLABEL), cmdShow);
	::ShowWindow( GetDlgItem( IDC_PASSWORDEDIT), cmdShow);
}

LRESULT CQuickSetupDlg::OnServerComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	serverChanged();
	return 0;
}

void CQuickSetupDlg::showFtpButton(bool show) {
	::ShowWindow( GetDlgItem( IDC_FTPSETTINGSBUTTON ), show? SW_SHOW : SW_HIDE);
}

void  CQuickSetupDlg::serverChanged() {
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	if ( serverComboElementIndex > 0 ) {
		int serverIndex = serverComboElementIndex - 1;
		CUploadEngineData * uploadEngineData = _EngineList->byIndex( serverIndex );
		if ( !uploadEngineData ) {
			return ;
		}
		CString serverName = Utf8ToWCstring( uploadEngineData->Name );
		bool isFtpServer = serverName.Find(_T("FTP")) != -1;
		showFtpButton(isFtpServer);
		bool authorizationAvailable = uploadEngineData->NeedAuthorization != 0;
		showAuthorizationControls( authorizationAvailable );
		bool forceAuthorization = uploadEngineData->NeedAuthorization == 2;
		CString doAuthCheckboxText = forceAuthorization ? TR("Выполнять авторизацию") : CString(TR("У меня есть учетная запись на этом сервере") ); //+ (forceAuthorization? _T("") : TR(" (необязательно)"));
		SetDlgItemText( IDC_DOAUTHCHECKBOX, doAuthCheckboxText );
		::EnableWindow( GetDlgItem( IDC_DOAUTHCHECKBOX), !forceAuthorization);
		SendDlgItemMessage( IDC_DOAUTHCHECKBOX, BM_SETCHECK, forceAuthorization? BST_CHECKED : BST_UNCHECKED );
		CString loginLabelText = uploadEngineData->LoginLabel.empty()? CString(TR("Логин:")) : CString(Utf8ToWCstring( uploadEngineData->LoginLabel )) + _T(":");
		SetDlgItemText( IDC_LOGINLABEL, loginLabelText );
		doAuthCheckboxChanged();
	} else {
		showAuthorizationControls(false);
	}
}

LRESULT CQuickSetupDlg::OnClickedFtpSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	if ( serverComboElementIndex > 0 ) {
		int serverIndex = serverComboElementIndex - 1;
		CUploadEngineData * uploadEngineData = _EngineList->byIndex( serverIndex );
		ServerProfile serverProfile;
		serverProfile.setServerName( Utf8ToWCstring( uploadEngineData->Name ) );
		CServerParamsDlg dlg(serverProfile);
		dlg.DoModal();
	}
	return 0;
}

