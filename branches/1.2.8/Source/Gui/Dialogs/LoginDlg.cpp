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
#include <Func/Myutils.h>

// CLoginDlg
CLoginDlg::CLoginDlg(CUploadEngineData *ue)
{
	m_UploadEngine = ue;
}

CLoginDlg::~CLoginDlg()
{
	
}

LRESULT CLoginDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	LoginInfo li = Settings.ServerByUtf8Name(m_UploadEngine->Name).authData;

	SetWindowText(TR("Параметры авторизации"));
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_DOAUTH, "Выполнять авторизацию");
	TRC(IDCANCEL, "Отмена");
	
	SetDlgItemText(IDC_LOGINEDIT, Utf8ToWCstring(li.Login));
	SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));
	SetDlgItemText(IDC_LOGINFRAME, Utf8ToWCstring(m_UploadEngine->Name));
	SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, (li.DoAuth?BST_CHECKED:BST_UNCHECKED));
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
	GetDlgItemText(IDC_PASSWORDEDIT, Buffer, 256);
	li.Password = WCstringToUtf8(Buffer);
	li.DoAuth = SendDlgItemMessage(IDC_DOAUTH, BM_GETCHECK) != FALSE;
	
	Settings.ServerByUtf8Name(m_UploadEngine->Name).authData = li;
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
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), IS_CHECKED(IDC_DOAUTH));
	return 0;
}
