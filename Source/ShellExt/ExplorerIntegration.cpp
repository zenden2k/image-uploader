/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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
// ExplorerIntegration.cpp : Implementation of DLL Exports.

#include "resource.h"
#include <atlstr.h>
#include <atlbase.h>
#include <atlapp.h>
#include "Generated/ExplorerIntegration.h"
#include "ishellcontextmenu.h"
//#include "../Func/LangClass.h"
#include "../3rdpart/Registry.h"
#include "Helpers.h"

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

//CLang Lang;
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
    {
        hDllInstance=hInstance;

        if(dwReason == DLL_PROCESS_ATTACH)
        {
            OleInitialize(NULL);
            /*TCHAR szFileName[1024], szPath[256];
            GetModuleFileName(hInstance, szFileName, 1023);
            Helpers::ExtractFilePath(szFileName, szPath, 256);
            Lang.SetDirectory(CString(szPath) + _T("Lang\\"));

            CRegistry Reg;
            CString lang;
            Reg.SetRootKey(HKEY_CURRENT_USER);
            if (Reg.SetKey(_T("Software\\Zenden.ws\\Image Uploader"), false))
            {
                lang = Reg.ReadString(_T("Language"));
            }
            else
            {
                #ifdef _WIN64
                    Reg.SetWOW64Flag(KEY_WOW64_32KEY);
                    Reg.SetRootKey(HKEY_CURRENT_USER);
                    if (Reg.SetKey(_T("Software\\Zenden.ws\\Image Uploader"), false))
                    {
                        lang = Reg.ReadString(_T("Language"));
                    }
                #endif
            }


            Lang.LoadLanguage(lang);*/
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
