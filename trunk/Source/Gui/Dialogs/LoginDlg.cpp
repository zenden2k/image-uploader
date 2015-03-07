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


#include "LoginDlg.h"
#include "wizarddlg.h"
#include "Func/Common.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"

// CLoginDlg
CLoginDlg::CLoginDlg(ServerProfile& serverProfile, bool createNew): serverProfile_(serverProfile)
{
	m_UploadEngine = serverProfile.uploadEngineData();
	ignoreExistingAccount_ = false;
	serverSupportsBeforehandAuthorization_ = false;
	CScriptUploadEngine *plugin_ = 0;
	if (!m_UploadEngine->PluginName.empty() ) {
		plugin_ = iuPluginManager.getPlugin(m_UploadEngine->Name, m_UploadEngine->PluginName, serverProfile.serverSettings());
		if ( plugin_ ) {
			serverSupportsBeforehandAuthorization_ = plugin_->supportsBeforehandAuthorization();
		}
	}
	createNew_ = createNew;
	//serverSupportsBeforehandAuthorization_ = serv
}

CLoginDlg::~CLoginDlg()
{
	
}

LRESULT CLoginDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	LoginInfo li = serverProfile_.serverSettings().authData;

	SetWindowText(TR("��������� �����������"));
	TRC(IDC_LOGINLABEL, "�����:");
	TRC(IDC_PASSWORDLABEL, "������:");
	TRC(IDC_DOAUTH, "��������� �����������");
	TRC(IDC_DOLOGINLABEL, "��������� �����������...");
	TRC(IDCANCEL, "������");
	TRC(IDC_DELETEACCOUNTLABEL, "������� �������");

	HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
	if (hWnd)
	{
		wndAnimation_.SubclassWindow(hWnd);
		if (wndAnimation_.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF), _T("GIF")))
			wndAnimation_.Draw();
		wndAnimation_.ShowWindow(SW_HIDE);
	}

	doLoginLabel_.SubclassWindow(GetDlgItem(IDC_DOLOGINLABEL));
	doLoginLabel_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
	doLoginLabel_.SetLabel(TR("��������� �����������..."));
	doLoginLabel_.m_clrLink = CSettings::DefaultLinkColor;
	doLoginLabel_.ShowWindow(serverSupportsBeforehandAuthorization_?SW_SHOW:SW_HIDE);

	CString deleteAccountLabelText;
	accountName_ = Utf8ToWCstring(li.Login);
	//deleteAccountLabelText.Format(R( "������� ������� ������ \"%s\" �� ������"), (LPCTSTR)accountName_);

	SetDlgItemText(IDC_LOGINEDIT, accountName_);
	SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));
	SetDlgItemText(IDC_LOGINFRAME, Utf8ToWCstring(m_UploadEngine->Name));
	SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, /*((li.DoAuth||createNew_)?BST_CHECKED:BST_UNCHECKED)*/true);
	::EnableWindow(GetDlgItem(IDC_DOAUTH),false);
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT),m_UploadEngine->NeedPassword);
	::EnableWindow(GetDlgItem(IDC_PASSWORDLABEL),m_UploadEngine->NeedPassword);

	deleteAccountLabel_.SubclassWindow(GetDlgItem(IDC_DELETEACCOUNTLABEL));
	deleteAccountLabel_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
//	deleteAccountLabel_.SetLabel(deleteAccountLabelText);
	deleteAccountLabel_.m_clrLink = CSettings::DefaultLinkColor;


	deleteAccountLabel_.ShowWindow((createNew_ || accountName_.IsEmpty()) ? SW_HIDE : SW_SHOW);

	
	OnClickedUseIeCookies(0, 0, 0, bHandled);
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

LRESULT CLoginDlg::OnClickedUseIeCookies(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow(GetDlgItem(IDC_LOGINEDIT), IS_CHECKED(IDC_DOAUTH));
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), IS_CHECKED(IDC_DOAUTH)&&m_UploadEngine->NeedPassword);
	return 0;
}

LRESULT CLoginDlg::OnDeleteAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::map <std::string, ServerSettingsStruct>& ss = Settings.ServersSettings[WCstringToUtf8(serverProfile_.serverName())];
	ss.erase(WCstringToUtf8(accountName_));

	accountName_ = "";
	EndDialog(IDOK);
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
	doLoginLabel_.ShowWindow( serverSupportsBeforehandAuthorization_ && !text.IsEmpty() ? SW_SHOW : SW_HIDE);
	return 0;
}

CString CLoginDlg::accountName()
{
	return accountName_;
}

DWORD CLoginDlg::Run()
{
	CScriptUploadEngine *plugin_ = 0;
	if (!m_UploadEngine->PluginName.empty() ) {

		LoginInfo li;
		CString login =GuiTools::GetDlgItemText(m_hWnd, IDC_LOGINEDIT); 
		li.Login = WCstringToUtf8(login);
		std::string serverNameA = WCstringToUtf8(serverProfile_.serverName());
		if ( !ignoreExistingAccount_ && createNew_ && Settings.ServersSettings[serverNameA].find(li.Login ) != Settings.ServersSettings[serverNameA].end() ) {
			MessageBox(TR("������� ������ � ����� ������ ��� ����������."),TR("������"), MB_ICONERROR);
			OnProcessFinished();
		}

		ignoreExistingAccount_ = true;

		if ( li.Login != WCstringToUtf8(accountName_) ) {
			serverProfile_.clearFolderInfo();
		}

		accountName_ = login;
		serverProfile_.setProfileName(login);
		li.Password = WCstringToUtf8(GuiTools::GetDlgItemText(m_hWnd, IDC_PASSWORDEDIT));
		li.DoAuth = SendDlgItemMessage(IDC_DOAUTH, BM_GETCHECK) != FALSE;
		ServerSettingsStruct& ss = serverProfile_.serverSettings();
		ss.authData = li;
		plugin_ = iuPluginManager.getPlugin(m_UploadEngine->Name, m_UploadEngine->PluginName, ss);
		if ( !plugin_->supportsBeforehandAuthorization() ) {
			OnProcessFinished();
			return 0;
		}

		IU_ConfigureProxy(networkManager_);
		plugin_->setNetworkManager(&networkManager_);
		int res = plugin_->doLogin();
		if ( res ) {
			OnProcessFinished();
			MessageBox(TR("����������� ���� ��������� �������"));
			Accept();
			
			return 0;
		} else {
			doLoginLabel_.SetLabel( TR("�� ������� ��������������. ����������� ��� ���."));
		}
		
	}
	OnProcessFinished();
	return 0;
}

void CLoginDlg::OnProcessFinished()
{
	enableControls(true);
}

void CLoginDlg::Accept()
{
	LoginInfo li;
	TCHAR Buffer[256];

	GetDlgItemText(IDC_LOGINEDIT, Buffer, 256);
	li.Login = WCstringToUtf8(Buffer);

	if ( li.Login.empty() ) {
		MessageBox(TR("����� �� ����� ���� ������"),TR("������"), MB_ICONERROR);
		return;
	}
	std::string serverNameA = WCstringToUtf8(serverProfile_.serverName());
	if ( !ignoreExistingAccount_ &&  createNew_ && Settings.ServersSettings[serverNameA].find(li.Login ) != Settings.ServersSettings[serverNameA].end() ) {
		MessageBox(TR("������� ������ � ����� ������ ��� ����������."),TR("������"), MB_ICONERROR);
		return;
	}

	if ( li.Login != WCstringToUtf8(accountName_) ) {
		serverProfile_.clearFolderInfo();
	}

	accountName_ = Buffer;
	serverProfile_.setProfileName(Buffer);
	GetDlgItemText(IDC_PASSWORDEDIT, Buffer, 256);
	li.Password = WCstringToUtf8(Buffer);
	li.DoAuth = SendDlgItemMessage(IDC_DOAUTH, BM_GETCHECK) != FALSE;

	serverProfile_.serverSettings().authData = li;
	EndDialog(IDOK);
}

void CLoginDlg::enableControls(bool enable)
{
	wndAnimation_.ShowWindow(!enable ? SW_SHOW : SW_HIDE);
	GuiTools::EnableDialogItem(m_hWnd, IDC_LOGINEDIT, enable);
	GuiTools::EnableDialogItem(m_hWnd, IDC_PASSWORDEDIT, enable&&m_UploadEngine->NeedPassword);
	GuiTools::EnableDialogItem(m_hWnd, IDOK, enable);
	doLoginLabel_.EnableWindow(enable);
	deleteAccountLabel_.ShowWindow(enable?SW_SHOW : SW_HIDE);
}
