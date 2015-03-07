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
// IShellContextMenu.h : Declaration of the CIShellContextMenu

#pragma once
#include "resource.h"       // main symbols
#include <atlapp.h>
#include <atlmisc.h>
#include <atlcoll.h>
#include "Generated/ExplorerIntegration.h"
#include "IconBitmapUtils.h"
#include <shobjidl.h>
#include <map>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

extern HINSTANCE hDllInstance;

// CIShellContextMenu

struct Shell_ContextMenuItem
{
	int cmd;
	CString text;
	CString commandArgument;
	WORD icon;
	int id;
};
#define MENUITEM_UPLOADFILES 0
#define MENUITEM_UPLOADONLYIMAGES 1
#define MENUITEM_IMPORTVIDEO 2
#define MENUITEM_MEDIAINFO 3
#define MENUITEM_SERVER_PROFILE 4

CString GetDllFolder();

class ATL_NO_VTABLE CIShellContextMenu :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIShellContextMenu, &CLSID_IShellContextMenu>,
	public IContextMenu3, public IShellExtInit,
	public IDispatchImpl<IIShellContextMenu, &IID_IIShellContextMenu, &LIBID_ExplorerIntegrationLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
	{
		public:
		CAtlArray<CString> m_FileList;
		std::map<int, Shell_ContextMenuItem> m_nCommands;
		std::map<UINT, HBITMAP>		bitmaps;
		CIShellContextMenu();

		DECLARE_REGISTRY_RESOURCEID(IDR_ISHELLCONTEXTMENU)

		BEGIN_COM_MAP(CIShellContextMenu)
			COM_INTERFACE_ENTRY(IIShellContextMenu)
			COM_INTERFACE_ENTRY(IDispatch)
			COM_INTERFACE_ENTRY(IContextMenu)
			COM_INTERFACE_ENTRY(IContextMenu2)
			COM_INTERFACE_ENTRY(IContextMenu3)
			COM_INTERFACE_ENTRY(IShellExtInit)
		END_COM_MAP()

		DECLARE_PROTECT_FINAL_CONSTRUCT()

		HRESULT FinalConstruct()
		{
			return S_OK;
		}

		void FinalRelease()
		{
		}
		bool MyInsertMenu(HMENU hMenu, int pos, UINT id, int nInternalCommand, const LPCTSTR szTitle, int firstCmd, CString commandArgument = CString(), bool UseBitmaps = true, HBITMAP bm=0,WORD resid=0,HICON ico = 0);
		bool MyInsertMenuSeparator(HMENU hMenu, int pos, UINT id);

		bool m_bMediaInfoInstalled;
		IconBitmapUtils m_IconBitmapUtils;
	HBITMAP IconToBitmap(HINSTANCE hInst, UINT uIcon);
	public:
		STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);
		STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	
		STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu,UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
		STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
		STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax);
  		STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, LPDATAOBJECT dataObject, HKEY hkeyProgID);
};

OBJECT_ENTRY_AUTO(__uuidof(IShellContextMenu), CIShellContextMenu)
