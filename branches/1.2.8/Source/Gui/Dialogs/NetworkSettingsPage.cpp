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
CNetworkSettingsPage::CNetworkSettingsPage(){		
}

CNetworkSettingsPage::~CNetworkSettingsPage(){
}

void CNetworkSettingsPage::TranslateUI() {
	TRC(IDC_CONNECTIONSETTINGS, "��������� �����������");
	TRC(IDC_USEPROXYSERVER, "������������ ������-������");
	TRC(IDC_ADDRESSLABEL, "�����:");
	TRC(IDC_PORTLABEL, "����:");
	TRC(IDC_SERVERTYPE, "��� �������:");
	TRC(IDC_NEEDSAUTH, "���������� �����������");
	TRC(IDC_LOGINLABEL, "�����:");
	TRC(IDC_PASSWORDLABEL, "������:");
	TRC(IDC_UPLOADBUFFERLABEL, "������ ������ ������:");
}
	
LRESULT CNetworkSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	TabBackgroundFix(m_hWnd);
	TranslateUI();

	BOOL temp;
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("HTTP"));

	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS4"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS4A"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS5"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS5(DNS)"));

	// ---- ���������� connection settings -----
	SetDlgItemText(IDC_ADDRESSEDIT, Settings.ConnectionSettings.ServerAddress);

	GuiTools::SetCheck(m_hWnd, IDC_NEEDSAUTH,  Settings.ConnectionSettings.NeedsAuth );
	GuiTools::SetCheck(m_hWnd, IDC_USEPROXYSERVER, Settings.ConnectionSettings.UseProxy);

	SetDlgItemText(IDC_PROXYLOGINEDIT, Settings.ConnectionSettings.ProxyUser);
	SetDlgItemText(IDC_PROXYPASSWORDEDIT, Settings.ConnectionSettings.ProxyPassword);
	SetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT, Settings.UploadBufferSize / 1024);

	if ( Settings.ConnectionSettings.ProxyPort != 0 ) {
		SetDlgItemInt(IDC_PORTEDIT, Settings.ConnectionSettings.ProxyPort);
	}

	GuiTools::SetCheck(m_hWnd, IDC_NEEDSAUTH,  Settings.ConnectionSettings.NeedsAuth);
	SendDlgItemMessage(IDC_SERVERTYPECOMBO, CB_SETCURSEL, Settings.ConnectionSettings.ProxyType);
	
	// ����������� ���������
	OnClickedUseProxy(BN_CLICKED, IDC_USEPROXYSERVER, 0, temp);

	return 1;  // Let the system set the focus
}

LRESULT CNetworkSettingsPage::OnClickedUseProxy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
	bool Checked = GuiTools::GetCheck(m_hWnd, IDC_USEPROXYSERVER);

	GuiTools::EnableNextN(GetDlgItem(wID),Checked? 8: 11, Checked);

	if ( Checked ) {
		OnClickedUseProxyAuth(BN_CLICKED, IDC_NEEDSAUTH, 0, bHandled);
	}

	//::EnableWindow(GetDlgItem(IDC_ADDRESSEDIT), Checked);
	return 0;
}
	
LRESULT CNetworkSettingsPage::OnClickedUseProxyAuth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool Checked = SendDlgItemMessage(wID, BM_GETCHECK)!=0;
	GuiTools::EnableNextN(GetDlgItem(wID), 4, Checked);
	return 0;
}

bool CNetworkSettingsPage::Apply() {
	Settings.ConnectionSettings.UseProxy      = GuiTools::GetCheck(m_hWnd, IDC_USEPROXYSERVER);
	Settings.ConnectionSettings.NeedsAuth     = GuiTools::GetCheck(m_hWnd, IDC_NEEDSAUTH);
	Settings.ConnectionSettings.ServerAddress = GuiTools::GetDlgItemText(m_hWnd, IDC_ADDRESSEDIT);
	Settings.ConnectionSettings.ProxyPort     = GetDlgItemInt(IDC_PORTEDIT);
	Settings.ConnectionSettings.ProxyUser     = GuiTools::GetDlgItemText(m_hWnd, IDC_PROXYLOGINEDIT);
	Settings.ConnectionSettings.ProxyPassword = GuiTools::GetDlgItemText(m_hWnd, IDC_PROXYPASSWORDEDIT);
	Settings.ConnectionSettings.ProxyType     = SendDlgItemMessage(IDC_SERVERTYPECOMBO, CB_GETCURSEL);
	Settings.UploadBufferSize                 = GetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT) * 1024;
	if ( !Settings.UploadBufferSize ) {
		Settings.UploadBufferSize = CSettings::DefaultUploadBufferSize;
	}
	return true;
}