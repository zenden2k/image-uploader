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
			Lang.SetDirectory(CString(szPath) + "Lang\\");
			Lang.LoadList();
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