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

#include "stdafx.h"
#include "ServerFolderSelect.h"
#include "pluginloader.h"
#include "common.h"
#include "NewFolderDlg.h"
#include "LogWindow.h"
#include "LangClass.h"
#include "Settings.h"
CServerFolderSelect::CServerFolderSelect(CUploadEngineData* uploadEngine)
{
	m_UploadEngine = uploadEngine;
}

CServerFolderSelect::~CServerFolderSelect()
{
}

LRESULT CServerFolderSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_FolderTree = GetDlgItem(IDC_FOLDERTREE);
	CenterWindow(GetParent());

	DlgResize_Init();

	
		HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
	if (hWnd)
	{
		m_wndAnimation.SubclassWindow(hWnd);
		if (m_wndAnimation.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF),_T("GIF")))
			m_wndAnimation.Draw();
		m_wndAnimation.ShowWindow(SW_HIDE);
	};

	// Internalization
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");
	TRC(IDC_NEWFOLDERBUTTON, "Создать папку");
	SetWindowText(TR("Список папок"));

	HBITMAP hBitmap;

	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);
	
	hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_SERVERTOOLBARBMP2));
	m_PlaceSelectorImageList.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	m_PlaceSelectorImageList.Add(hBitmap,RGB(255,0,255));

	m_FolderTree.SetImageList(m_PlaceSelectorImageList);
	m_FolderMap[L""] = 0;

	IU_ConfigureProxy(m_NetworkManager);
	
	m_FolderOperationType = foGetFolders;
	m_pluginLoader = iuPluginManager.getPlugin(m_UploadEngine->PluginName, Settings.ServerByUtf8Name(m_UploadEngine->Name));
	if(!m_pluginLoader) 
	{
		SetDlgItemText(IDC_FOLDERLISTLABEL, TR("Ошибка при загрузке squirrel скрипта."));
		return 0;
	}
	CString title;
	title.Format(TR("Список папок на сервере %s для учетной записи '%s':"), (LPCTSTR)Utf8ToWCstring(m_UploadEngine->Name), (LPCTSTR)Utf8ToWCstring(Settings.ServerByUtf8Name(m_UploadEngine->Name).authData.Login));
	SetDlgItemText(IDC_FOLDERLISTLABEL, title);
	m_pluginLoader->setNetworkManager(&m_NetworkManager);
	m_pluginLoader->getAccessTypeList(m_accessTypeList);
	if(m_pluginLoader)
	{
		CreateLoadingThread();
	}
	return 1;  // Let the system set the focus
}

LRESULT CServerFolderSelect::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HTREEITEM item = m_FolderTree.GetSelectedItem();

	if(!item) return 0;
	int nIndex = m_FolderTree.GetItemData(item);

	if(nIndex<0 || nIndex>m_FolderList.GetCount()-1) return 0;

	if(!::IsWindowEnabled(GetDlgItem(IDOK)))
		return 0;

	m_SelectedFolder = m_FolderList[nIndex];
	EndDialog(wID);

	return 0;
}

LRESULT CServerFolderSelect::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

 DWORD CServerFolderSelect::Run()
 {
	if(m_FolderOperationType == foGetFolders) 
	{
		int count=0;
		
		if(!m_pluginLoader) return -1;

		m_FolderList.Clear();
		m_FolderMap.clear();
		int retCode = m_pluginLoader->getFolderList(m_FolderList);
		if(retCode < 1) {
			if(retCode == -1)
				TRC(IDC_FOLDERLISTLABEL, "Данный сервер не поддерживает получение списка папок");
			else
				TRC(IDC_FOLDERLISTLABEL, "Не удалось загрузить список папок.");
				
			OnLoadFinished();
			return -1;
		}
		
		HTREEITEM toSelect = 0;
		int nNeedToBeSelected=-1;

		m_FolderTree.DeleteAllItems();
		BuildFolderTree(m_FolderList.m_folderItems, "");
		m_FolderTree.SelectItem(m_FolderMap[Utf8ToWstring(m_SelectedFolder.id)]);
		OnLoadFinished();
	}

	 else if(m_FolderOperationType == foCreateFolder)
	 {
			m_pluginLoader->createFolder(CFolderItem(),m_newFolder);
			m_FolderOperationType = foGetFolders;
			Run();
			m_FolderTree.SelectItem(m_FolderMap[Utf8ToWstring(m_newFolder.id)]);
	 }

	 else if(m_FolderOperationType == foModifyFolder) // Modifying an existing folder
	 {
			m_pluginLoader->modifyFolder(m_newFolder);
			m_FolderOperationType = foGetFolders;
			Run();
			m_FolderTree.SelectItem(m_FolderMap[Utf8ToWstring(m_newFolder.id)]);
	 }
	BlockWindow(false);
	return 0;
 }

 void CServerFolderSelect::OnLoadFinished()
  {
	  BlockWindow(false);
 }

void CServerFolderSelect::NewFolder(const CString& parentFolderId)
{
	CFolderItem newFolder;
	CNewFolderDlg dlg(newFolder, true, m_accessTypeList);
	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		m_newFolder = newFolder;
		m_newFolder.parentid = WCstringToUtf8(parentFolderId);
		m_FolderOperationType = foCreateFolder;
		if(!IsRunning())
		{
			CreateLoadingThread();
		}
	 }
}

 LRESULT CServerFolderSelect::OnBnClickedNewfolderbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
 {
	 NewFolder("");
	 return 0;
 }

 LRESULT CServerFolderSelect::OnFolderlistLbnDblclk(WORD wNotifyCode, WORD wID, HWND hWndCtl)
 {
	 SendDlgItemMessage(IDOK,BM_CLICK );
	 return 0;
 }

LRESULT CServerFolderSelect::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CMenu sub;	
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_TYPE|MIIM_ID;
	mi.fType = MFT_STRING;
	sub.CreatePopupMenu();
		
	POINT ClientPoint, ScreenPoint;
	HWND 	hwnd = (HWND) wParam;  
	if(hwnd != m_FolderTree.m_hWnd) return 0;

	if(lParam == -1) 
	{
		ClientPoint.x = 0;
		ClientPoint.y = 0;
		ScreenPoint = ClientPoint;
		::ClientToScreen(hwnd, &ScreenPoint);
	}
	else
	{
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}

	UINT      flags;
	HTREEITEM selectedItem = m_FolderTree.HitTest(ClientPoint, &flags);
	if(!selectedItem) return 0;

	m_FolderTree.SelectItem(selectedItem);
	
	mi.wID = ID_EDITFOLDER;
	mi.dwTypeData  = TR("Редактировать");
	sub.InsertMenuItem(1, true, &mi);

	mi.wID = ID_OPENINBROWSER;
 	mi.dwTypeData  = TR("Открыть в браузере");
	sub.InsertMenuItem(0, true, &mi);

	mi.wID = ID_CREATENESTEDFOLDER;
 	mi.dwTypeData  = TR("Создать вложенную папку");
	sub.InsertMenuItem(2, true, &mi);

	sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	return 0;
 }

LRESULT CServerFolderSelect::OnEditFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HTREEITEM item = m_FolderTree.GetSelectedItem();

	if(!item) return 0;
	int nIndex = m_FolderTree.GetItemData(item);
	
	CFolderItem &folder = m_FolderList[nIndex];

	CNewFolderDlg dlg(folder, false, m_accessTypeList);
	if(dlg.DoModal() == IDOK)
	{
		 m_FolderOperationType = foModifyFolder; // Editing an existing folder
		 m_newFolder = folder;
		 if(!IsRunning()){
			CreateLoadingThread();
			
		}
	}
	return 0;
}

void CServerFolderSelect::BlockWindow(bool Block)
{
	m_wndAnimation.ShowWindow(Block?  SW_SHOW: SW_HIDE);
	::EnableWindow(GetDlgItem(IDOK), !Block);
	::EnableWindow(GetDlgItem(IDCANCEL), !Block);
	::EnableWindow(GetDlgItem(IDC_NEWFOLDERBUTTON), !Block);
	m_FolderTree.EnableWindow(!Block);
}

LRESULT CServerFolderSelect::OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HTREEITEM item = m_FolderTree.GetSelectedItem();

	if(!item) return 0;
	int nIndex = m_FolderTree.GetItemData(item);
	
	CFolderItem &folder = m_FolderList[nIndex];

	CString str = folder.viewUrl.c_str();
	if(!str.IsEmpty())
	{
		ShellExecute(0,_T("open"), str, _T(""), 0, SW_SHOWNORMAL);
	}
	return 0;
}

void CServerFolderSelect::CreateLoadingThread()
{
	BlockWindow(true);
	Start();
}

LRESULT CServerFolderSelect::OnCreateNestedFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HTREEITEM item = m_FolderTree.GetSelectedItem();

	if(!item) return 0;
	int nIndex = m_FolderTree.GetItemData(item);
	
	
	CFolderItem &folder = m_FolderList[nIndex];
	NewFolder(folder.id.c_str());
	return 0;
}

void CServerFolderSelect::BuildFolderTree(std::vector<CFolderItem> &list,const CString& parentFolderId)
{
	for(size_t i=0; i<list.size(); i++)
	{
		CFolderItem& cur = list[i] ;
		if(cur.parentid.c_str() == parentFolderId)
		{
			CString title = Utf8ToWCstring(cur.title);
				if(cur.itemCount != -1)
					title+= _T(" (") + IntToStr(cur.itemCount) + _T(")");
				HTREEITEM res = m_FolderTree.InsertItem(title,1,1, m_FolderMap[Utf8ToWstring(cur.parentid)],TVI_SORT	);
				m_FolderTree.SetItemData(res, i);

			m_FolderMap[Utf8ToWstring(cur.id)] = res;
			if(cur.id != "")
				BuildFolderTree(list, cur.id.c_str());
			if(parentFolderId==_T(""))
			m_FolderTree.Expand(res);
		}
	}
}
	