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

#pragma once
#include "../../Func/pluginloader.h"
#include "../../resource.h"       // main symbols
#include "../../3rdpart/thread.h"
#include "../../Func/common.h"
#include <atlcrack.h>
#include "../Controls/PictureExWnd.h"
#include "../../Core/Upload/UploadEngine.h"
typedef enum FolderOperationType            /* Defines an enumeration type    */
{
   foGetFolders = 0, foCreateFolder, foModifyFolder 
};

// CServerFolderSelect
#define ID_EDITFOLDER 10001
#define ID_OPENINBROWSER 10002
#define ID_CREATENESTEDFOLDER 10003
class CServerFolderSelect : 
	public CDialogImpl<CServerFolderSelect>	, public CThreadImpl<CServerFolderSelect>, public CDialogResize<CServerFolderSelect>	
{
public:
	CServerFolderSelect(CUploadEngineData* uploadEngine);
	~CServerFolderSelect();
	enum { IDD = IDD_SERVERFOLDERSELECT};

    BEGIN_MSG_MAP(CServerFolderSelect)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		  COMMAND_HANDLER(ID_OPENINBROWSER, BN_CLICKED, OnOpenInBrowser)
		  COMMAND_HANDLER(ID_CREATENESTEDFOLDER, BN_CLICKED, OnCreateNestedFolder)
		  
		  COMMAND_HANDLER(IDC_NEWFOLDERBUTTON, BN_CLICKED, OnBnClickedNewfolderbutton)
		COMMAND_ID_HANDLER(ID_EDITFOLDER, OnEditFolder)
		CHAIN_MSG_MAP(CDialogResize<CServerFolderSelect>)
	 END_MSG_MAP()

	 	BEGIN_DLGRESIZE_MAP(CServerFolderSelect)
		DLGRESIZE_CONTROL(IDC_FOLDERLISTLABEL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_FOLDERTREE, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X| DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_ANIMATIONSTATIC, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_NEWFOLDERBUTTON, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCreateNestedFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	DWORD Run();
 void OnLoadFinished();
 CImageList m_PlaceSelectorImageList;
 CFolderItem m_SelectedFolder;
protected:
	CPictureExWnd m_wndAnimation;
	CScriptUploadEngine *m_pluginLoader;
	CUploadEngineData *m_UploadEngine;
	CFolderList m_FolderList;
	CFolderItem m_newFolder;
	std::vector<std::string> m_accessTypeList;
	std::map<std::wstring,HTREEITEM> m_FolderMap;
	CTreeViewCtrl m_FolderTree;
	//IU_PLUGIN_FolderItem * m_folderItems;
	FolderOperationType m_FolderOperationType;
	NetworkManager m_NetworkManager;
	void BlockWindow(bool Block);
	void NewFolder(const CString& parentFolderId);
	//CString m_sNewFolderName, m_sNewFolderDescription;
public:
	
	LRESULT OnBnClickedNewfolderbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFolderlistLbnDblclk(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void CreateLoadingThread();
	void BuildFolderTree(std::vector<CFolderItem> &list,const CString& parentFolderId);
};


