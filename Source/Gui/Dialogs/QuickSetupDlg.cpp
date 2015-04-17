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
	hIconSmall = GuiTools::LoadSmallIcon(IDR_MAINFRAME);
	SetIcon(hIconSmall, FALSE);
	serverComboBox_.Attach( GetDlgItem( IDC_SERVERCOMBOBOX ) );

	if ( !Settings.IsPortable ) {
		SendDlgItemMessage( IDC_AUTOSTARTUPCHECKBOX, BM_SETCHECK, BST_CHECKED, 0);
		SendDlgItemMessage( IDC_CAPTUREPRINTSCREENCHECKBOX, BM_SETCHECK, BST_CHECKED, 0);
		SendDlgItemMessage( IDC_EXPLORERINTEGRATION, BM_SETCHECK, BST_CHECKED, 0);
	}
	LogoImage.SubclassWindow(GetDlgItem( IDC_STATICLOGO ) );
	LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE );
	LogoImage.LoadImage(0, 0, Settings.UseNewIcon ? IDR_ICONMAINNEW : IDR_PNG1, false, GetSysColor(COLOR_BTNFACE));

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
	
	comboBoxImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);

	//serverComboBox_.AddItem( _T("<") + CString(TR("Случайный сервер")) + _T(">"), -1, -1, 0, static_cast<LPARAM>( -1 ) );

	HICON hImageIcon = NULL, hFileIcon = NULL;
	int selectedIndex = 0;

	//CUploadEngineData *uploadEngine = _EngineList->byIndex( Settings.getServerID() );
	std::string selectedServerName = "directupload.net" ;
	for( int i = 0; i < _EngineList->count(); i++) {	
		CUploadEngineData * ue = _EngineList->byIndex( i ); 
		if ( ue->Type !=  CUploadEngineData::TypeImageServer && ue->Type !=  CUploadEngineData::TypeFileServer ) {
			continue;
		}
		HICON hImageIcon = _EngineList->getIconForServer(ue->Name);
		int nImageIndex = -1;
		if ( hImageIcon) {
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

LRESULT CQuickSetupDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int count = serverComboBox_.GetCount();
	for ( int i = 0; i < count ; i++ ) {
		char* serverName = reinterpret_cast<char*>(serverComboBox_.GetItemData(i));
		delete[] serverName;
	}
	serverComboBox_.ResetContent();
	return 0;
}

LRESULT CQuickSetupDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {


	int serverComboElementIndex = serverComboBox_.GetCurSel();
	if ( serverComboElementIndex > 0 ) {
		std::string serverNameA = reinterpret_cast<char*>(serverComboBox_.GetItemData(serverComboElementIndex));
		CUploadEngineData * uploadEngineData =( (CUploadEngineList *)_EngineList)->byName( serverNameA );
		Settings.imageServer.setServerName(uploadEngineData->Name) ;
		bool needAuth = GuiTools::GetCheck( m_hWnd, IDC_DOAUTHCHECKBOX );
		if ( needAuth ) {
			CString login = GuiTools::GetDlgItemText( m_hWnd, IDC_LOGINEDIT );
			Settings.imageServer.setProfileName(WCstringToUtf8(login));
			CString password = GuiTools::GetDlgItemText( m_hWnd, IDC_PASSWORDEDIT );
			if ( login.IsEmpty() ) {
				MessageBox(TR("Введите данные учетной записи"), APPNAME, MB_ICONEXCLAMATION);
				return 0;
			}
			LoginInfo& loginInfo = Settings.ServersSettings[WCstringToUtf8((LPCTSTR)Settings.getServerName())][WCstringToUtf8((LPCTSTR)(login))].authData;
			loginInfo.DoAuth = true;
			loginInfo.Login = WCstringToUtf8( login );
			loginInfo.Password = WCstringToUtf8( password );
		}
		Settings.quickScreenshotServer = Settings.imageServer;
		Settings.contextMenuServer = Settings.imageServer;
		Settings.fileServer.setServerName("zippyshare.com");

	} else {

	}

	GuiTools::GetCheck(m_hWnd, IDC_AUTOSTARTUPCHECKBOX, Settings.AutoStartup);
	Settings.AutoStartup_changed = true;

	
	Settings.ExplorerContextMenu = GuiTools::GetCheck(m_hWnd, IDC_EXPLORERINTEGRATION);
	Settings.ExplorerContextMenu_changed = Settings.ExplorerContextMenu;

	bool capturePrintScreen = GuiTools::GetCheck( m_hWnd, IDC_CAPTUREPRINTSCREENCHECKBOX );
	if ( capturePrintScreen ) {
		Settings.Hotkeys.getByFunc( _T("regionscreenshot") ).globalKey.DeSerialize("44"); // PrintScreen
		Settings.Hotkeys.getByFunc( _T("windowscreenshot") ).globalKey.DeSerialize("65580"); // Alt + PrintScreen
	} else {
		Settings.Hotkeys.getByFunc( _T("regionscreenshot") ).Clear();
		Settings.Hotkeys.getByFunc( _T("windowscreenshot") ).Clear();

	}

	if ( Settings.AutoStartup || capturePrintScreen ) {
		Settings.ShowTrayIcon        = true;
		Settings.ShowTrayIcon_changed = true;
	}
	Settings.SaveSettings();
	
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
		std::string serverNameA = reinterpret_cast<char*>(serverComboBox_.GetItemData(serverComboElementIndex));
		CUploadEngineData * uploadEngineData =( (CUploadEngineList *)_EngineList)->byName( serverNameA );
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
		bool isDoAuthChecked = SendDlgItemMessage(IDC_DOAUTHCHECKBOX, BM_GETCHECK) == BST_CHECKED;
		::EnableWindow( GetDlgItem( IDC_PASSWORDEDIT),isDoAuthChecked && uploadEngineData->NeedPassword);
	} else {
		showAuthorizationControls(false);
	}
}

LRESULT CQuickSetupDlg::OnClickedFtpSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int serverComboElementIndex = serverComboBox_.GetCurSel();
	if ( serverComboElementIndex > 0 ) {
		int serverIndex = serverComboElementIndex - 1;
		CUploadEngineData * uploadEngineData = _EngineList->byIndex( serverIndex );
/*		ServerProfile serverProfile;
		serverProfile.setServerName( Utf8ToWCstring( uploadEngineData->Name ) );
		CServerParamsDlg dlg(serverProfile);
		dlg.DoModal();*/
	}
	return 0;
}

