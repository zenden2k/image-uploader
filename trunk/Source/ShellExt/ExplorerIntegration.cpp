// ExplorerIntegration.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "ExplorerIntegration.h"
#include "ishellcontextmenu.h"

#include "../LangClass.h"
#include "../Settings.h"
#include "../MyUtils.h"

class CExplorerIntegrationModule : public CAtlDllModuleT< CExplorerIntegrationModule >
{
public :
	DECLARE_LIBID(LIBID_ExplorerIntegrationLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_EXPLORERINTEGRATION, "{B3E19CB7-7EF2-4C97-A1DC-324F6A6CF2E5}")
};

CExplorerIntegrationModule _AtlModule;

	BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_IShellContextMenu, CIShellContextMenu)
	END_OBJECT_MAP()


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
	{
		hDllInstance=hInstance;

		if(dwReason == DLL_PROCESS_ATTACH)
		{
			OleInitialize(NULL);
			TCHAR szFileName[256], szPath[256];
			GetModuleFileName(hInstance, szFileName, 1023);
			ExtractFilePath(szFileName, szPath);
			//CLang Lang;
			Lang.SetDirectory(CString(szPath) + "Lang\\");
			Lang.LoadList();

			CSettings settings;
			Settings.LoadSettings(GetDllFolder());
			
			Lang.LoadLanguage(Settings.Language);
		}

		return _AtlModule.DllMain(dwReason, lpReserved); 
	}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}