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
	createNew_ = createNew;
}

CLoginDlg::~CLoginDlg()
{
	
}

LRESULT CLoginDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	LoginInfo li = serverProfile_.serverSettings().authData;

	SetWindowText(TR("Параметры авторизации"));
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_DOAUTH, "Выполнять авторизацию");
	TRC(IDCANCEL, "Отмена");
	CString deleteAccountLabelText;
	accountName_ = Utf8ToWCstring(li.Login);
	deleteAccountLabelText.Format(TR( "Удалить учетную запись \"%s\" из списка"), (LPCTSTR)accountName_);

	SetDlgItemText(IDC_LOGINEDIT, accountName_);
	SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));
	SetDlgItemText(IDC_LOGINFRAME, Utf8ToWCstring(m_UploadEngine->Name));
	SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, /*((li.DoAuth||createNew_)?BST_CHECKED:BST_UNCHECKED)*/true);
	::EnableWindow(GetDlgItem(IDC_DOAUTH),false);
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT),m_UploadEngine->NeedPassword);
	::EnableWindow(GetDlgItem(IDC_PASSWORDLABEL),m_UploadEngine->NeedPassword);

	deleteAccountLabel_.SubclassWindow(GetDlgItem(IDC_DELETEACCOUNTLABEL));
	deleteAccountLabel_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
	deleteAccountLabel_.SetLabel(deleteAccountLabelText);
	deleteAccountLabel_.m_clrLink = CSettings::DefaultLinkColor;


	deleteAccountLabel_.ShowWindow((createNew_ || accountName_.IsEmpty()) ? SW_HIDE : SW_SHOW);

	
	OnClickedUseIeCookies(0, 0, 0, bHandled);
	::SetFocus(GetDlgItem(IDC_LOGINEDIT));
	return 0;  // Разрешаем системе самостоятельно установить фокус ввода
}

LRESULT CLoginDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	LoginInfo li;
	TCHAR Buffer[256];

	GetDlgItemText(IDC_LOGINEDIT, Buffer, 256);
	li.Login = WCstringToUtf8(Buffer);
	std::string serverNameA = WCstringToUtf8(serverProfile_.serverName());
	if ( createNew_ && Settings.ServersSettings[serverNameA].find(li.Login ) != Settings.ServersSettings[serverNameA].end() ) {
		MessageBox(TR("Учетная запись с таким именем уже существует."),TR("Ошибка"), MB_ICONERROR);
		return 0;
	}
	accountName_ = Buffer;
	serverProfile_.setProfileName(Buffer);
	GetDlgItemText(IDC_PASSWORDEDIT, Buffer, 256);
	li.Password = WCstringToUtf8(Buffer);
	li.DoAuth = SendDlgItemMessage(IDC_DOAUTH, BM_GETCHECK) != FALSE;
	
	serverProfile_.serverSettings().authData = li;
	EndDialog(wID);
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
	Settings.ServersSettings[WCstringToUtf8(serverProfile_.serverName())].erase(WCstringToUtf8(accountName_));
	accountName_ = "";
	EndDialog(IDOK);
	return 0;
}

CString CLoginDlg::accountName()
{
	return accountName_;
}
