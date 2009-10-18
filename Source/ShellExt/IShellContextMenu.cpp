/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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
// IShellContextMenu.cpp : Implementation of CIShellContextMenu

#include "stdafx.h"
#include <atlbase.h>
#include <atlcoll.h>
#include "../Settings.h"
#include "../MyUtils.h"
#include "IShellContextMenu.h"
HINSTANCE hDllInstance;

CString GetDllFolder()
{
	TCHAR szFileName[256],szPath[256];
	GetModuleFileName(hDllInstance, szFileName, 1023);
	ExtractFilePath(szFileName, szPath);
	return szPath;
}

bool IsMediaInfoInstalled()
{
	CString MediaInfoDllPath;
	TCHAR Buffer[MAX_PATH];
	HKEY ExtKey;
	Buffer[0]=0;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\KLCodecPack"), 0,/* REG_OPTION_NON_VOLATILE, */KEY_QUERY_VALUE,  &ExtKey/* NULL*/);
	TCHAR ClassName[MAX_PATH]=_T("\0");
	DWORD BufSize = sizeof(ClassName)/sizeof(TCHAR);
	DWORD Type = REG_SZ;
   RegQueryValueEx(ExtKey,	 _T("installdir"), 0, &Type, (LPBYTE)&ClassName, &BufSize);
	RegCloseKey(ExtKey);
	CString MediaDll = GetDllFolder()+_T("\\Modules\\MediaInfo.dll");
	if(FileExists( MediaDll)) MediaInfoDllPath  = MediaDll;
	else
	{
		CString MediaDll2 =CString(ClassName)+_T("\\Tools\\MediaInfo.dll");
		if(FileExists( MediaDll2)) MediaInfoDllPath = MediaDll2;
	}
	return !MediaInfoDllPath.IsEmpty();
}

bool AreOnlyImages(CAtlArray<CString> & files)
{
	if(files.GetCount()>1000) return false;

	for(int i=0; i<files.GetCount();i++)
	{
		if(!IsImage(files[i])) return false;
	}
	return true;
}

bool CIShellContextMenu::MyInsertMenu(HMENU hMenu, int pos, UINT id, int nInternalCommand, const LPTSTR szTitle, int firstCmd, HBITMAP bm,WORD resid)
{
	Shell_ContextMenuItem InternalMenuItem;
	InternalMenuItem.cmd =  nInternalCommand;
	InternalMenuItem.text = szTitle;
	if(resid)
	InternalMenuItem.icon = resid;
	InternalMenuItem.id = id;
	m_nCommands[id-firstCmd]= InternalMenuItem;
	MENUITEMINFO MenuItem;
	
	MenuItem.cbSize = sizeof(MenuItem);
	MenuItem.fType = MFT_STRING;
	MenuItem.hbmpItem = IsVista() ? m_IconBitmapUtils.IconToBitmapPARGB32(hDllInstance, resid): HBMMENU_CALLBACK;;
	MenuItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_BITMAP | MIIM_STRING;
	
	MenuItem.wID = id;
	MenuItem.dwTypeData = szTitle;
	return InsertMenuItem(hMenu, pos, TRUE, &MenuItem);	
}

STDMETHODIMP CIShellContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res;
	return HandleMenuMsg2(uMsg, wParam, lParam, &res);
}

STDMETHODIMP CIShellContextMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	LRESULT res;
	if (pResult == NULL)
		pResult = &res;
	*pResult = FALSE;
	
	switch (uMsg)
	{
	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
			if (lpmis==NULL)
				break;
			lpmis->itemWidth += 2;
			if (lpmis->itemHeight < 16)
				lpmis->itemHeight = 16;
			*pResult = TRUE;
		}
		break;
	case WM_DRAWITEM:
		{
			LPCTSTR resource;
			HICON hIcon=0;
			DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
			if ((lpdis==NULL)||(lpdis->CtlType != ODT_MENU))
				return S_OK;		//not for a menu
			int i =0;
			int iconID=0;

			std::map<int, Shell_ContextMenuItem>::iterator it;
			for(it=m_nCommands.begin(); it!=m_nCommands.end();it++)
			{
				if(it->second.id == lpdis->itemID) 
				{ 
					iconID= it->second.icon;
				}
			}
			if(iconID)
				hIcon = (HICON)LoadImage(hDllInstance, MAKEINTRESOURCE(iconID), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);//m_nCommands[ LOWORD( lpdis->itemID )].icon;//(HICON)LoadImage(g_hResInst, resource, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			if (hIcon == NULL)
				return S_OK;
			DrawIconEx(lpdis->hDC,
				lpdis->rcItem.left - 16,
				lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top - 16) / 2,
				hIcon, 16, 16,
				0, NULL, DI_NORMAL);
			*pResult = TRUE;
			DeleteObject(hIcon);
		}
		break;
	default:
		return S_OK;
	}

	return S_OK;
}

// CIShellContextMenu
HRESULT CIShellContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	UINT currentCommandID = idCmdFirst;
	if ((uFlags & 0x000F) != CMF_NORMAL  && (uFlags & CMF_VERBSONLY) == 0 && (uFlags & CMF_EXPLORE) == 0)
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, currentCommandID);

	HMENU PopupMenu;
	UINT subIndex = indexMenu;
	if(Settings.ExplorerCascadedMenu)
		PopupMenu = CreatePopupMenu();
	else PopupMenu = hmenu;


	if(AreOnlyImages(m_FileList))
		MyInsertMenu(PopupMenu, subIndex++, currentCommandID++, MENUITEM_UPLOADONLYIMAGES, TR("Загрузить изображения"),idCmdFirst,bmUpArrow,IDI_ICONUPLOAD);
	else
	{
		MyInsertMenu(PopupMenu, subIndex++, currentCommandID++, MENUITEM_UPLOADFILES, TR("Загрузить файлы"),idCmdFirst,bmUpArrow,IDI_ICONUPLOAD);
		MyInsertMenu(PopupMenu, subIndex++, currentCommandID++, MENUITEM_UPLOADONLYIMAGES,TR("Загрузить только изображения"),idCmdFirst,bmUpArrow,IDI_ICONUPLOAD);
	}
	if(Settings.ExplorerVideoContextMenu&&  m_FileList.GetCount()==1 &&IsVideoFile( m_FileList[0]))
	{
		MyInsertMenu(PopupMenu, subIndex++, currentCommandID++,MENUITEM_IMPORTVIDEO,  TR("Импорт видео"),idCmdFirst,Settings.ExplorerCascadedMenu?0:bmMovie,Settings.ExplorerCascadedMenu?0:IDI_ICONMOVIE);
		if(m_bMediaInfoInstalled)
			MyInsertMenu(PopupMenu, subIndex++, currentCommandID++,MENUITEM_MEDIAINFO, TR("Информация о файле"),idCmdFirst,Settings.ExplorerCascadedMenu?0:bmInfo,Settings.ExplorerCascadedMenu?0:IDI_ICONINFO);
	}
	if(Settings.ExplorerCascadedMenu)
	{
		MENUITEMINFO MenuItem;
		MenuItem.cbSize = sizeof(MenuItem);
		MenuItem.hbmpChecked = bmIULogo;
		MenuItem.hbmpUnchecked =  bmIULogo;
		MenuItem.fType = MFT_STRING;
		MenuItem.fMask =MIIM_SUBMENU| MIIM_FTYPE | MIIM_ID | MIIM_BITMAP | MIIM_STRING;//MIIM_SUBMENU | MIIM_TYPE | MIIM_DATA|MIIM_ID|MIIM_BITMAP;//|MIIM_CHECKMARKS;
		MenuItem.wID = currentCommandID++;
		MenuItem.dwTypeData = _T("Image Uploader");
		MenuItem.hSubMenu = PopupMenu;

		// Inserting item in our internal list
		Shell_ContextMenuItem InternalMenuItem;
		InternalMenuItem.cmd =  -1;
		InternalMenuItem.text = MenuItem.dwTypeData;
		
		InternalMenuItem.icon= IDI_ICONMAIN;
		MenuItem.hbmpItem = IsVista() ? m_IconBitmapUtils.IconToBitmapPARGB32(hDllInstance, IDI_ICONMAIN): HBMMENU_CALLBACK;
		
		InternalMenuItem.id = MenuItem.wID;
		m_nCommands[InternalMenuItem.id -idCmdFirst]= InternalMenuItem;
		InsertMenuItem(hmenu, indexMenu, true, &MenuItem);	
	}

	if (IsVista())
	{
		MENUINFO MenuInfo;

		memset(&MenuInfo, 0, sizeof(MenuInfo));

		MenuInfo.cbSize  = sizeof(MenuInfo);
		MenuInfo.fMask   = MIM_STYLE | MIM_APPLYTOSUBMENUS;
		MenuInfo.dwStyle = MNS_CHECKORBMP;

		SetMenuInfo(hmenu, &MenuInfo);
	}
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, currentCommandID - idCmdFirst);
}
      
bool IULaunchCopy(CAtlArray<CString> & CmdLine,const CString params=_T(""))
{
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	
	ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);				 
   ZeroMemory(&pi, sizeof(pi));

	CString TempCmdLine = CString(_T("\""))+GetDllFolder()+_T("Image Uploader.exe")+CString(_T("\""));
	if(!params.IsEmpty()) TempCmdLine+=_T(" ")+params+_T(" ");
	for(int i=0;i <CmdLine.GetCount(); i++)
		{
			if(!lstrcmpi(CmdLine[i], _T("-Embedding"))) continue;
			TempCmdLine = TempCmdLine + " \"" + CmdLine[i] + "\""; 
		}

    // Start the child process.
    if( !CreateProcess(
		NULL,                   // No module name (use command line). 
        (LPWSTR)(LPCTSTR)TempCmdLine, // Command line. 
        NULL,                   // Process handle not inheritable. 
        NULL,                   // Thread handle not inheritable. 
        FALSE,                  // Set handle inheritance to FALSE. 
        0,                      // No creation flags. 
        NULL,                   // Use parent's environment block. 
        NULL,                   // Use parent's starting directory. 
        &si,                    // Pointer to STARTUPINFO structure.
        &pi )                   // Pointer to PROCESS_INFORMATION structure.
    ) 
    
        return false;

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	return true;
}

HRESULT CIShellContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	switch ( m_nCommands[ LOWORD( lpici->lpVerb )].cmd)
	{
	case MENUITEM_UPLOADFILES:
		{
			IULaunchCopy(m_FileList,_T("/upload"));

			return S_OK;
		}
		break;
	case MENUITEM_UPLOADONLYIMAGES:
		{
			IULaunchCopy(m_FileList,_T("/upload /imagesonly"));

			return S_OK;
		}
		break;
	case MENUITEM_IMPORTVIDEO:
		{
			IULaunchCopy(m_FileList);

			return S_OK;
		}
		break;
	case  MENUITEM_MEDIAINFO:
		{
			IULaunchCopy(m_FileList,_T("/mediainfo"));

			return S_OK;
		}
		break;

	default:
		return E_INVALIDARG;
		break;
	}
	return S_OK;
}


HRESULT CIShellContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
	return S_OK;
}

CIShellContextMenu::CIShellContextMenu()
{
	m_bMediaInfoInstalled = IsMediaInfoInstalled();
	bmIULogo = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_IULOGO));
	bmUpArrow = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_UPARROW));
	bmMovie = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_MOVIE));
	bmInfo = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_INFO));
}

HRESULT CIShellContextMenu::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT dataObject, HKEY hkeyProgID)
{
	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };
	HDROP     hDrop;

	// Look for CF_HDROP data in the data object.
	if ( FAILED(dataObject->GetData ( &fmt, &stg )))
	{
		// Nope! Return an "invalid argument" error back to Explorer.
		return E_INVALIDARG;
	}

	// Get a pointer to the actual data.
	hDrop = (HDROP) GlobalLock ( stg.hGlobal );

	// Make sure it worked.
	if ( NULL == hDrop )
	{
		return E_INVALIDARG;
	}

	// Sanity check – make sure there is at least one filename.
	UINT uNumFiles = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );

	if ( 0 == uNumFiles )
	{
		GlobalUnlock ( stg.hGlobal );
		ReleaseStgMedium ( &stg );
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;
	TCHAR FileName[MAX_PATH+1];
	int i =0;

	while( DragQueryFile ( hDrop, i++, FileName, MAX_PATH ))
	{

		m_FileList.Add(FileName);
		// hr = E_INVALIDARG;
	}

	GlobalUnlock ( stg.hGlobal );
	ReleaseStgMedium ( &stg );

	return hr;
}


