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

#include "IntegrationSettings.h"

#include <uxtheme.h>
#include "Func/common.h"
#include "Func/Settings.h"
#include "LogWindow.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
#include "ContextMenuItemDlg.h"
#include "3rdpart/Registry.h"
#include <Core/Utils/CryptoUtils.h>

// CIntegrationSettings
CIntegrationSettings::CIntegrationSettings()
{
}

CIntegrationSettings::~CIntegrationSettings()
{

}
	

LRESULT CIntegrationSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	serverProfiles_ = Settings.ServerProfiles;
	menuItemsChanged_ = false;
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
	TRC(IDC_CONTEXTMENUITEMSLABEL, "Пользовательские элементы меню:");

	menuItemsListBox_.m_hWnd = GetDlgItem(IDC_CONTEXTMENUITEMSLIST);

	TCHAR buf[MAX_PATH];
	CString buf2;

	SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerContextMenu);
	
	bool shellIntegrationAvailable = FileExists(Settings.getShellExtensionFileName())!=0;

	SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerVideoContextMenu);
	SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_SETCHECK, Settings.SendToContextMenu);
	SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_SETCHECK, Settings.ExplorerCascadedMenu);
	
	SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_SETCHECK, Settings.QuickUpload);

	HICON ico = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONADDITEM), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage(IDC_ADDITEM, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)ico);
	addItemButton_.SubclassWindow(GetDlgItem(IDC_ADDITEM));

	HICON icon2 = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONDELETEITEM), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage(IDC_DELETEITEM, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)icon2);
	deleteItemButton_.SubclassWindow(GetDlgItem(IDC_DELETEITEM));

	HICON icon3 = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONUP), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage(IDC_UPBUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)icon3);
	upButton_.SubclassWindow(GetDlgItem(IDC_UPBUTTON));

	HICON icon4 = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONDOWN), IMAGE_ICON	, 16,16,0);
	SendDlgItemMessage(IDC_DOWNBUTTON, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(HICON)icon4);
	downButton_.SubclassWindow(GetDlgItem(IDC_DOWNBUTTON));


	CRegistry Reg;
	Reg.SetRootKey( HKEY_CURRENT_USER );

	serverProfiles_.clear();
	std::vector<CString> keyNames;
	CString keyPath = "Software\\Zenden.ws\\Image Uploader\\ContextMenuItems";
	Reg.GetChildKeysNames(keyPath,keyNames);
	for(int i =0; i < keyNames.size() ; i++ ) {
		if ( Reg.SetKey(keyPath + _T("\\") + keyNames[i], false) ) {
			CString title = Reg.ReadString("Name");
			CString displayTitle = title;
			ListItemData* lid = new	ListItemData();
			if ( Settings.ServerProfiles.find(keyNames[i])==  Settings.ServerProfiles.end()) {
				displayTitle = _T("[invalid] ") + displayTitle;
				lid->invalid = true;
			} else {
				lid->serverProfile = Settings.ServerProfiles[keyNames[i]];
			}
			lid->name  = title;
			int newIndex = menuItemsListBox_.AddString(displayTitle);
			menuItemsListBox_.SetItemData(newIndex, (DWORD_PTR) lid);
		}
	}


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
	
	if ( menuItemsChanged_ ) {
		int menuItemCount = menuItemsListBox_.GetCount();
		CRegistry Reg;
		Reg.SetRootKey( HKEY_CURRENT_USER );

		serverProfiles_.clear();
		Reg.DeleteWithSubkeys("Software\\Zenden.ws\\Image Uploader\\ContextMenuItems");
		CString itemId;
		if ( Reg.SetKey( "Software\\Zenden.ws\\Image Uploader\\ContextMenuItems", true ) ) {

				for( int i =0; i< menuItemCount; i++ ){
					ListItemData* lid = (	ListItemData*)menuItemsListBox_.GetItemData(i);
					if ( lid->invalid ) {
						continue;
					}
					CRegistry Reg2 = Reg;
					CString itemNumber;
					itemNumber.Format(_T("%04d"), i);
					itemId = itemNumber+_T("_")+lid->serverProfile.serverName() + L"_" + IuCoreUtils::CryptoUtils::CalcMD5HashFromString(IuCoreUtils::int64_tToString(rand() % 999999)).c_str();
					itemId.Replace(L" ",L"_");
					itemId.Replace(L":",L"_");
					itemId.Replace(L"\\",L"_");
					itemId.Replace(L"//",L"_");
					//MessageBox(itemId);
					if ( Reg2.SetKey("Software\\Zenden.ws\\Image Uploader\\ContextMenuItems\\" + itemId, true) ) {
						Reg2.WriteString( "Name", lid->name );
						Reg2.WriteString( "ServerName", lid->serverProfile.serverName() );
						Reg2.WriteString( "ProfileName", lid->serverProfile.profileName() );
						Reg2.WriteString( "FolderId", Utf8ToWCstring(lid->serverProfile.folderId() ) );
						Reg2.WriteString( "FolderTitle", Utf8ToWCstring(lid->serverProfile.folderTitle()) );
						Reg2.WriteString( "FolderUrl", Utf8ToWCstring(lid->serverProfile.folderUrl()) );
						CString icon = _EngineList->getIconNameForServer(WCstringToUtf8(lid->serverProfile.serverName()));
						CUploadEngineData * ued = lid->serverProfile.uploadEngineData();
						if ( ued ) {
							Reg2.WriteDword( "ServerType", (unsigned int) ued->Type );
						}
						Reg2.WriteString( "Icon", icon);
						
						serverProfiles_[itemId] = lid->serverProfile;
					}
				}
			}
		Settings.ServerProfiles = serverProfiles_;
	}
	
	//MessageBoxA(0,Settings.ServerProfiles[itemId].folderTitle().c_str(),0,0);
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
	HWND contextMenuItemsLabel = GetDlgItem(IDC_CONTEXTMENUITEMSLABEL);
	::EnableWindow(contextMenuItemsLabel, checked);
	GuiTools::EnableNextN(contextMenuItemsLabel, 5, checked);
}
LRESULT CIntegrationSettings::OnBnClickedAdditem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CContextMenuItemDlg dlg;
	if ( dlg.DoModal(m_hWnd) == IDOK ) {
		int newIndex = menuItemsListBox_.AddString(dlg.menuItemTitle());
		ListItemData* lid = new ListItemData();
		lid->name = dlg.menuItemTitle();
		lid->serverProfile = dlg.serverProfile();
		menuItemsListBox_.SetItemData(newIndex, (DWORD_PTR) lid);
		menuItemsChanged_ = true; 
	}
	// TODO: Add your control notification handler code here

	return 0;
}

LRESULT CIntegrationSettings::OnBnClickedDeleteitem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int currentIndex = menuItemsListBox_.GetCurSel();
	if ( currentIndex != -1 ) {
		ListItemData* lid = (ListItemData*) menuItemsListBox_.GetItemData(currentIndex);
		menuItemsListBox_.DeleteString(currentIndex);
		delete lid;
		menuItemsChanged_ = true;
	}
	// TODO: Add your control notification handler code here

	return 0;
}

LRESULT CIntegrationSettings::OnBnClickedDownbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int itemIndex = menuItemsListBox_.GetCurSel();
	if(itemIndex == -1) 
		return 0;

	if(itemIndex < menuItemsListBox_.GetCount() - 1)
	{
		TCHAR* name = new TCHAR[menuItemsListBox_.GetTextLen(itemIndex)+1];
	
		menuItemsListBox_.GetText(itemIndex,name);
		DWORD_PTR data = menuItemsListBox_.GetItemData(itemIndex);
		menuItemsListBox_.DeleteString(itemIndex);
		menuItemsListBox_.InsertString(itemIndex + 1,name);
		menuItemsListBox_.SetItemData(itemIndex + 1,data);
		menuItemsListBox_.SetCurSel(itemIndex+1);
		delete[] name;
		menuItemsChanged_ = true; 
	}
	return 0;
}

LRESULT CIntegrationSettings::OnBnClickedUpbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int itemIndex = menuItemsListBox_.GetCurSel();
	if(itemIndex == -1) 
		return 0;

	if(itemIndex > 0)
	{
		TCHAR* name = new TCHAR[menuItemsListBox_.GetTextLen(itemIndex)+1];

		menuItemsListBox_.GetText(itemIndex,name);
		DWORD_PTR data = menuItemsListBox_.GetItemData(itemIndex);
		menuItemsListBox_.DeleteString(itemIndex);
		menuItemsListBox_.InsertString(itemIndex - 1,name);
		menuItemsListBox_.SetItemData(itemIndex - 1,data);
		menuItemsListBox_.SetCurSel(itemIndex-1);
		delete[] name;
		menuItemsChanged_ = true; 
	}

	return 0;
}

LRESULT CIntegrationSettings::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int itemCount = menuItemsListBox_.GetCount();
	for( int i =0; i < itemCount; i++ ){
		ListItemData* lid = reinterpret_cast<ListItemData*>(menuItemsListBox_.GetItemData(i));
		delete lid;
	}
	menuItemsListBox_.ResetContent();
	return 0;
}
