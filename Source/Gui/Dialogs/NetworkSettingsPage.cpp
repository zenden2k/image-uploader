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

#include "NetworkSettingsPage.h"
#include <uxtheme.h>

#include "wizarddlg.h"
#include "Gui/GuiTools.h"

// CNetworkSettingsPage
CNetworkSettingsPage::CNetworkSettingsPage()
{
		
}

CNetworkSettingsPage::~CNetworkSettingsPage()
{
}

void CNetworkSettingsPage::TranslateUI()
{
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_CONNECTIONSETTINGS, "Параметры подключения");
	TRC(IDC_USEPROXYSERVER, "Использовать прокси-сервер");
	TRC(IDC_ADDRESSLABEL, "Адрес:");
	TRC(IDC_PORTLABEL, "Порт:");
	TRC(IDC_SERVERTYPE, "Тип сервера:");
	TRC(IDC_NEEDSAUTH, "Необходима авторизация");
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_AUTOCOPYTOCLIPBOARD, "Автоматически копировать результаты в буфер обмена");
	TRC(IDC_UPLOADBUFFERLABEL, "Размер буфера отдачи:");
}
	
LRESULT CNetworkSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);
	TranslateUI();

	BOOL temp;
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("HTTP"));

	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS4"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS4A"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS5"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS5(DNS)"));

	// ---- Инициализация элементов (заполнение) ----
	
	// ---- заполнение connection settings -----
	SetDlgItemText(IDC_ADDRESSEDIT, Settings.ConnectionSettings.ServerAddress);
	SendDlgItemMessage(IDC_NEEDSAUTH, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.NeedsAuth);
	SendDlgItemMessage(IDC_AUTOCOPYTOCLIPBOARD, BM_SETCHECK, (WPARAM) Settings.AutoCopyToClipboard);
	
		SendDlgItemMessage(IDC_USEPROXYSERVER, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.UseProxy);
	SetDlgItemText(IDC_PROXYLOGINEDIT, Settings.ConnectionSettings.ProxyUser);
	SetDlgItemText(IDC_PROXYPASSWORDEDIT, Settings.ConnectionSettings.ProxyPassword);
	SetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT,Settings.UploadBufferSize/1024);
	if(Settings.ConnectionSettings.ProxyPort) // Только если порт не равен нулю
		SetDlgItemInt(IDC_PORTEDIT, Settings.ConnectionSettings.ProxyPort);


	SendDlgItemMessage(IDC_SERVERTYPECOMBO, CB_SETCURSEL, Settings.ConnectionSettings.ProxyType);
	SendDlgItemMessage(IDC_NEEDSAUTH, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.NeedsAuth);
	
	// Уведомление элементов
	OnClickedUseProxy(BN_CLICKED, IDC_USEPROXYSERVER, 0, temp);

	return 1;  // Let the system set the focus
}

LRESULT CNetworkSettingsPage::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CNetworkSettingsPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CNetworkSettingsPage::OnClickedUseProxy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	bool Checked = SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK)!=0;
	GuiTools::EnableNextN(GetDlgItem(wID),Checked? 8: 11, Checked);

	if(Checked)
		OnClickedUseProxyAuth(BN_CLICKED, IDC_NEEDSAUTH, 0, bHandled);

	//::EnableWindow(GetDlgItem(IDC_ADDRESSEDIT), Checked);
	return 0;
}
	
LRESULT CNetworkSettingsPage::OnClickedUseProxyAuth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool Checked = SendDlgItemMessage(wID, BM_GETCHECK)!=0;
	GuiTools::EnableNextN(GetDlgItem(wID), 4, Checked);
	return 0;
}

bool CNetworkSettingsPage::Apply()
{
	Settings.ConnectionSettings.UseProxy = SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK)!=0;
	Settings.ConnectionSettings.NeedsAuth = SendDlgItemMessage(IDC_NEEDSAUTH, BM_GETCHECK);
	TCHAR Buffer[128];

	GetDlgItemText(IDC_ADDRESSEDIT,Buffer, 128);
	Settings.ConnectionSettings.ServerAddress = Buffer;
	Settings.ConnectionSettings.ProxyPort = GetDlgItemInt(IDC_PORTEDIT);
	
	GetDlgItemText(IDC_PROXYLOGINEDIT, Buffer, 128);
	Settings.ConnectionSettings.ProxyUser = Buffer;
	GetDlgItemText(IDC_PROXYPASSWORDEDIT, Buffer, 128);
	Settings.ConnectionSettings.ProxyPassword = Buffer;
	Settings.ConnectionSettings.ProxyType = SendDlgItemMessage(IDC_SERVERTYPECOMBO, CB_GETCURSEL);
	Settings.UploadBufferSize = GetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT)*1024;
	if(!Settings.UploadBufferSize) Settings.UploadBufferSize = 65536;
	return true;
}