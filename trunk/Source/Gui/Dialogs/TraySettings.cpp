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

#include "../../atlheaders.h"
#include "TraySettings.h"
#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}
 
CTrayActions::CTrayActions()
	{
		/*AddTrayAction(TR("��� ��������"), 0);
		AddTrayAction(TR("����������� ���� ������"), IDM_CONTEXTMENU);
		AddTrayAction(TR("��������� �����"), IDM_UPLOADFILES);
		AddTrayAction(TR("��������� �����"), IDM_ADDFOLDER);
		AddTrayAction(TR("������ �����"),IDM_IMPORTVIDEO);
		AddTrayAction(TR("��������"),IDM_SCREENSHOTDLG);
		AddTrayAction(TR("�������� ���������� �������"),IDM_REGIONSCREENSHOT);
		AddTrayAction(TR("�������� ����� ������"),IDM_FULLSCREENSHOT);
		AddTrayAction(TR("�������� �������� ����"),IDM_WINDOWSCREENSHOT);
		AddTrayAction(TR("�������� ���� ���������"), IDM_SHOWAPPWINDOW);
		AddTrayAction(TR("���������"), IDM_SETTINGS);
		
		AddTrayAction(TR("�����"), IDM_EXIT);*/
	
	}

// CTraySettingsPage
CTraySettingsPage::CTraySettingsPage()
{

}

CTraySettingsPage::~CTraySettingsPage()
{
}

LRESULT CTraySettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRC(IDC_SHOWTRAYICON,"���������� ������ � ����");
	TRC(IDC_MOUSEREACTIONGROUP,"������� �� �������� ����");
	TRC(IDC_LEFTBUTTONDOUBLECLICKLABEL,"������� ���� ����� �������");
	TRC(IDC_LEFTBUTTONCLICKLABEL,"��������� ���� ����� �������");
	TRC(IDC_MIDDLEBUTTONCLICKLABEL,"���� ������� �������");
	TRC(IDC_RIGHTBUTTONCLICKLABEL,"���� ������ �������");
	TRC(IDC_ONEINSTANCE,"�� ��������� ����� ����� ��������� �� ����");
	TRC(IDC_AUTOSTARTUP, "������ ��������� ��� ������ Windows");
	//CTrayActions trayActions;
	for(size_t i=0; i< Settings.Hotkeys/*trayActions*/.GetCount(); i++)
	{
		SendDlgItemMessage(IDC_LEFTBUTTONDOUBLECLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
		SendDlgItemMessage(IDC_LEFTBUTTONCLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
		SendDlgItemMessage(IDC_MIDDLEBUTTONCLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
	
		SendDlgItemMessage(IDC_RIGHTBUTTONCLICKCOMBO, CB_ADDSTRING,0, (LPARAM)(LPCTSTR)Settings.Hotkeys[i].GetDisplayName());
	
	}

	SendDlgItemMessage(IDC_SHOWTRAYICON, BM_SETCHECK,Settings.ShowTrayIcon);
	SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK,Settings.AutoStartup);

	SendDlgItemMessage(IDC_ONEINSTANCE, BM_SETCHECK,Settings.TrayIconSettings.DontLaunchCopy);
	OnShowTrayIconBnClicked(BN_CLICKED, IDC_SHOWTRAYICON, 0);

	SendDlgItemMessage(IDC_LEFTBUTTONDOUBLECLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.LeftDoubleClickCommand);
	SendDlgItemMessage(IDC_LEFTBUTTONCLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.LeftClickCommand);
	SendDlgItemMessage(IDC_MIDDLEBUTTONCLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.MiddleClickCommand);
	SendDlgItemMessage(IDC_RIGHTBUTTONCLICKCOMBO, CB_SETCURSEL,Settings.TrayIconSettings.RightClickCommand);
	
	return 1;  // Let the system set the focus
}

LRESULT CTraySettingsPage::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CTraySettingsPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
bool CTraySettingsPage::Apply()
{
	Settings.ShowTrayIcon_changed = Settings.ShowTrayIcon;
	Settings.ShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK)==BST_CHECKED;
	Settings.ShowTrayIcon_changed ^= Settings.ShowTrayIcon;


	Settings.AutoStartup_changed = Settings.AutoStartup;
	Settings.AutoStartup = SendDlgItemMessage(IDC_AUTOSTARTUP, BM_GETCHECK)==BST_CHECKED;
	Settings.AutoStartup_changed ^= Settings.AutoStartup;

	//Settings.ExplorerContextMenu_changed = true;
	Settings.TrayIconSettings.LeftDoubleClickCommand = SendDlgItemMessage(IDC_LEFTBUTTONDOUBLECLICKCOMBO, CB_GETCURSEL);
	Settings.TrayIconSettings.LeftClickCommand = SendDlgItemMessage(IDC_LEFTBUTTONCLICKCOMBO, CB_GETCURSEL);
	Settings.TrayIconSettings.MiddleClickCommand = SendDlgItemMessage(IDC_MIDDLEBUTTONCLICKCOMBO, CB_GETCURSEL);
	Settings.TrayIconSettings.RightClickCommand = SendDlgItemMessage(IDC_RIGHTBUTTONCLICKCOMBO, CB_GETCURSEL);
	
	Settings.TrayIconSettings.DontLaunchCopy = SendDlgItemMessage(IDC_ONEINSTANCE, BM_GETCHECK);
	
	return true;
}
LRESULT CTraySettingsPage::OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	bool bShowTrayIcon = SendDlgItemMessage(IDC_SHOWTRAYICON, BM_GETCHECK) == BST_CHECKED;
	SendDlgItemMessage(IDC_AUTOSTARTUP, BM_SETCHECK, FALSE);
	EnableNextN(GetDlgItem(wID),11,bShowTrayIcon);
	return 0;
}
