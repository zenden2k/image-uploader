// IShellContextMenu.h : Declaration of the CIShellContextMenu

#pragma once
#include "resource.h"       // main symbols
#include <atlapp.h>
#include <atlmisc.h>
#include <atlcoll.h>
#include "ExplorerIntegration.h"
#include <shobjidl.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

extern HINSTANCE hDllInstance;

// CIShellContextMenu

struct Shell_ContextMenuItem
{
	int cmd;
	CString text;
};
#define MENUITEM_UPLOADFILES 0
#define MENUITEM_UPLOADONLYIMAGES 1
#define MENUITEM_IMPORTVIDEO 2
#define MENUITEM_MEDIAINFO 3

CString GetDllFolder();

class ATL_NO_VTABLE CIShellContextMenu :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIShellContextMenu, &CLSID_IShellContextMenu>,
	public IContextMenu, public IShellExtInit,
	public IDispatchImpl<IIShellContextMenu, &IID_IIShellContextMenu, &LIBID_ExplorerIntegrationLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
	{
		public:
		CAtlArray<CString> m_FileList;
		CAtlMap<int, Shell_ContextMenuItem> m_nCommands;
		HBITMAP bmIULogo, bmUpArrow,bmInfo,bmMovie;
		CIShellContextMenu();

		DECLARE_REGISTRY_RESOURCEID(IDR_ISHELLCONTEXTMENU)

		BEGIN_COM_MAP(CIShellContextMenu)
			COM_INTERFACE_ENTRY(IIShellContextMenu)
			COM_INTERFACE_ENTRY(IDispatch)
			COM_INTERFACE_ENTRY(IContextMenu)
			COM_INTERFACE_ENTRY(IShellExtInit)
		END_COM_MAP()

		DECLARE_PROTECT_FINAL_CONSTRUCT()

		HRESULT FinalConstruct()
		{
			return S_OK;
		}

		void FinalRelease()
		{
			DeleteObject(bmIULogo);
			DeleteObject(bmUpArrow);
			DeleteObject(bmMovie);
			DeleteObject(bmInfo);
		}
		bool MyInsertMenu(HMENU hMenu, int pos, UINT id, int nInternalCommand, const LPTSTR szTitle, int firstCmd, HBITMAP bm=NULL);
		bool m_bMediaInfoInstalled;

	public:
		STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu,UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
		STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
		STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax);
  		STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, LPDATAOBJECT dataObject, HKEY hkeyProgID);
};

OBJECT_ENTRY_AUTO(__uuidof(IShellContextMenu), CIShellContextMenu)
