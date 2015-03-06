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

#include "IntegrationSettings.h"

#include <uxtheme.h>
#include "Func/common.h"
#include "Func/Settings.h"
#include "LogWindow.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
// CIntegrationSettings
CIntegrationSettings::CIntegrationSettings()
{
}

CIntegrationSettings::~CIntegrationSettings()
{

}
	

LRESULT CIntegrationSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
	// Translating controls
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_INTEGRATIONGROUP, "Интеграция с проводником Windows");
	TRC(IDC_SHELLINTEGRATION, "Интеграция в контекстное меню оболочки");
	//TRC(IDC_SHELLIMGCONTEXTMENUITEM, "Пункт в контекстное меню файлов изображений");
	TRC(IDC_STARTUPLOADINGFROMSHELL, "Сразу начинать загрузку на сервер");
	TRC(IDC_SHELLVIDEOCONTEXTMENUITEM, "Пункт в контекстном меню видеофайлов");
	TRC(IDC_CASCADEDCONTEXTMENU, "Вложенное контекстное меню");
	TRC(IDC_SHELLSENDTOITEM, "Добавить Image Uploader в меню \"Отправить\"");
	

	TCHAR buf[MAX_PATH];
	CString buf2;

	SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerContextMenu);
	
	bool shellIntegrationAvailable = FileExists(Settings.getShellExtensionFileName())!=0;

	SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerVideoContextMenu);
	SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_SETCHECK, Settings.SendToContextMenu);
	SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_SETCHECK, Settings.ExplorerCascadedMenu);
	
	SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_SETCHECK, Settings.QuickUpload);

	CIcon ico = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONADDITEM), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage(IDC_ADDITEM, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)ico);

	CIcon icon2 = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONDELETEITEM), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage(IDC_DELETEITEM, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)ico2);

	BOOL b;
	OnClickedQuickUpload(0, IDC_STARTUPLOADINGFROMSHELL,0, b);
	::EnableWindow(GetDlgItem(IDC_SHELLVIDEOCONTEXTMENUITEM), shellIntegrationAvailable);
	::EnableWindow(GetDlgItem(IDC_CASCADEDCONTEXTMENU), shellIntegrationAvailable);
	::EnableWindow(GetDlgItem(IDC_SHELLIMGCONTEXTMENUITEM), shellIntegrationAvailable);
	ShellIntegrationChanged();
	
	return 1;  // Let the system set the focus
}

	
bool CIntegrationSettings::Apply()
{
	Settings.ExplorerContextMenu_changed = Settings.ExplorerContextMenu; 
	Settings.ExplorerContextMenu = SendDlgItemMessage(IDC_SHELLINTEGRATION, BM_GETCHECK)==BST_CHECKED;
	Settings.ExplorerContextMenu_changed ^= (Settings.ExplorerContextMenu);
	
	bool Temp = Settings.ExplorerVideoContextMenu;
	Settings.ExplorerVideoContextMenu = SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_GETCHECK)==BST_CHECKED;
	Temp ^= Settings.ExplorerVideoContextMenu;

	Temp = Settings.ExplorerCascadedMenu;
	Settings.ExplorerCascadedMenu = SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_GETCHECK)==BST_CHECKED;
	Temp ^= Settings.ExplorerCascadedMenu;

	Settings.SendToContextMenu_changed = Settings.SendToContextMenu;
	Settings.SendToContextMenu = SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_GETCHECK)==BST_CHECKED;
	Settings.SendToContextMenu_changed ^= Settings.SendToContextMenu;

	Settings.QuickUpload = SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_GETCHECK)==BST_CHECKED;
	//FIXME
	Settings.setQuickServerID(SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0, 0));
	
	return true;
}

LRESULT CIntegrationSettings::OnClickedQuickUpload(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}



LRESULT CIntegrationSettings::OnShellIntegrationCheckboxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShellIntegrationChanged();
	return 0;
}
	
void CIntegrationSettings::ShellIntegrationChanged()
{
		bool shellIntegrationAvailable = FileExists(Settings.getShellExtensionFileName())!=0;
	bool checked = SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_GETCHECK)==BST_CHECKED && shellIntegrationAvailable;
	GuiTools::EnableNextN(GetDlgItem(IDC_SHELLINTEGRATION), 2, checked);
}