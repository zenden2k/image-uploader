/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "Common.h"

#include "atlheaders.h"
#include "Func/CmdLine.h"
#include "Func/MyUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Func/WinUtils.h"

CString IUCommonTempFolder;

CString IU_md5_file(const CString& filename)
{
    return U2W(IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(W2U(filename)));
}

bool IULaunchCopy(CString additionalParams)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    TCHAR Buffer[MAX_PATH * 40];
    GetModuleFileName(0, Buffer, sizeof(Buffer) / sizeof(TCHAR));

    CString TempCmdLine = CString(_T("\"")) + CmdLine[0] + CString(_T("\""));
    for (size_t i = 1; i < CmdLine.GetCount(); i++)
    {
        if (!lstrcmpi(CmdLine[i], _T("-Embedding")))
            continue;
        TempCmdLine = TempCmdLine + " \"" + CmdLine[i] + "\"";
    }

    TempCmdLine += _T(" ") + additionalParams;
    // Start the child process.
    if ( !CreateProcess(
            NULL,              // No module name (use command line).
            (LPWSTR)(LPCTSTR)TempCmdLine, // Command line.
            NULL,                // Process handle not inheritable.
            NULL,                // Thread handle not inheritable.
            FALSE,               // Set handle inheritance to FALSE.
            0,                   // No creation flags.
            NULL,                // Use parent's environment block.
            NULL,                // Use parent's starting directory.
            &si,                 // Pointer to STARTUPINFO structure.
            &pi )                // Pointer to PROCESS_INFORMATION structure.
         )

        return false;
    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    return true;
}

#define HOTKEY(modifier, key) ((((modifier) & 0xff) << 8) | ((key) & 0xff))

// Создание ярлыка
// Входные параметры:
//  pwzShortCutFileName - путь и имя ярлыка, например, "C:\\Блокнот.lnk"
//  Если не указан путь, ярлык будет создан в папке, указанной в следующем параметре.
//  Прим.: Windows сама НЕ добавляет к имени расширение .lnk
//  pszPathAndFileName  - путь и имя exe-файла, например, "C:\\Windows\\NotePad.Exe"
//  pszWorkingDirectory - рабочий каталог, например, "C:\\Windows"
//  pszArguments        - аргументы командной строки, например, "C:\\Doc\\Text.Txt"
//  wHotKey             - горячая клавиша, например, для Ctrl+Alt+A     HOTKEY(HOTKEYF_ALT|HOTKEYF_CONTROL,'A')
//  iCmdShow            - начальный вид, например, SW_SHOWNORMAL
//  pszIconFileName     - путь и имя файла, содержащего иконку, например, "C:\\Windows\\NotePad.Exe"
//  int iIconIndex      - индекс иконки в файле, нумеруется с 0
bool __fastcall CreateShortCut(
   LPCWSTR pwzShortCutFileName,
   LPCTSTR pszPathAndFileName,
   LPCTSTR pszWorkingDirectory,
   LPCTSTR pszArguments,
   WORD wHotKey,
   int iCmdShow,
   LPCTSTR pszIconFileName,
   int iIconIndex)
{
    IShellLink* pSL;
    IPersistFile* pPF;
    HRESULT hRes;
    CoInitialize(NULL);
    // return false;
    // Получение экземпляра компонента "Ярлык"
    hRes = CoCreateInstance(CLSID_ShellLink, 0,  CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pSL);

    if ( SUCCEEDED(hRes) )
    {
        hRes = pSL->SetPath(pszPathAndFileName);
        if ( SUCCEEDED(hRes) )
        {
            hRes = pSL->SetArguments(pszArguments);
            // if( SUCCEEDED(hRes) )
            {
                hRes = pSL->SetWorkingDirectory(pszWorkingDirectory);
                if ( SUCCEEDED(hRes) )
                {
                    hRes = pSL->SetIconLocation(pszIconFileName, iIconIndex);
                    if ( SUCCEEDED(hRes) )
                    {
                        //    hRes = pSL->SetHotkey(wHotKey);
                        //    if( SUCCEEDED(hRes) )
                        {
                            hRes = pSL->SetShowCmd(iCmdShow);
                            if ( SUCCEEDED(hRes) )
                            {
                                // Получение компонента хранилища параметров
                                hRes = pSL->QueryInterface(IID_IPersistFile, (LPVOID*)&pPF);
                                if ( SUCCEEDED(hRes) )
                                {
                                    // Сохранение созданного ярлыка
                                    hRes = pPF->Save(pwzShortCutFileName, TRUE);
                                    pPF->Release();
                                }
                            }
                        }
                    }
                }
            }
        }
        pSL->Release();
    }
    return SUCCEEDED(hRes);
}

bool IULaunchCopy(CString params, const CAtlArray<CString>& files)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    TCHAR Buffer[MAX_PATH * 40];
    GetModuleFileName(0, Buffer, sizeof(Buffer) / sizeof(TCHAR));

    CString TempCmdLine = CString(_T("\"")) + Buffer + CString(_T("\""));
    if (!params.IsEmpty())
        TempCmdLine += _T(" ") + params + _T(" ");

    for (size_t i = 0; i < files.GetCount(); i++)
    {
        TempCmdLine = TempCmdLine + " \"" + files[i] + "\"";
    }

    // Start the child process.
    if ( !CreateProcess(
            NULL,                        // No module name (use command line).
            (LPWSTR)(LPCTSTR)TempCmdLine, // Command line.
            NULL,                // Process handle not inheritable.
            NULL,                // Thread handle not inheritable.
            FALSE,               // Set handle inheritance to FALSE.
            0,                   // No creation flags.
            NULL,                // Use parent's environment block.
            NULL,                // Use parent's starting directory.
            &si,                 // Pointer to STARTUPINFO structure.
            &pi )                // Pointer to PROCESS_INFORMATION structure.
         )

        return false;

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    return true;
}

void IU_RunElevated(CString params)
{
    SHELLEXECUTEINFO TempInfo = {0};
    CString appDir = WinUtils::GetAppFolder();
    CString Command = CmdLine[0];
    CString parameters = _T(" ") + params;
    TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    TempInfo.fMask = 0;
    TempInfo.hwnd = NULL;
    if (WinUtils::IsVista())
        TempInfo.lpVerb = _T("runas");
    else
        TempInfo.lpVerb = _T("open");
    TempInfo.lpFile = Command;
    TempInfo.lpParameters = parameters;
    TempInfo.lpDirectory = appDir;
    TempInfo.nShow = SW_NORMAL;

    ::ShellExecuteEx(&TempInfo);
}

DWORD MsgWaitForSingleObject(HANDLE pHandle, DWORD dwMilliseconds)
{
    while ((MsgWaitForMultipleObjects(1, &pHandle, FALSE, dwMilliseconds,
                                      /*QS_ALLEVENTS*/ QS_SENDMESSAGE)) != WAIT_OBJECT_0)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 1;
}

CString GetUniqFileName(const CString& filePath)
{
    if (!WinUtils::FileExists(filePath))
        return filePath;
    CString path = U2W(IuCoreUtils::ExtractFilePath(W2U(filePath)));

    CString name;
    name = WinUtils::GetOnlyFileName(filePath);

    CString extension = GetFileExt(filePath);
    CString result;
    for (int i = 2;; i++)
    {
        result = path + name + WinUtils::IntToStr(i) + (extension.IsEmpty() ? _T("") : _T(".") + extension);
        if (!WinUtils::FileExists(result))
            break;
    }
    return result;
}

BOOL IU_CreateFolder(LPCTSTR szFolder)
{
    if (!szFolder || !lstrlen(szFolder))
        return FALSE;

    DWORD dwAttrib = GetFileAttributes(szFolder);

    // already exists ?
    if (dwAttrib != 0xffffffff)
        return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

    // recursively create from the top down
    TCHAR* szPath = _tcsdup(szFolder);
    TCHAR* p = _tcsrchr(szPath, '\\');

    if (p)
    {
        // The parent is a dir, not a drive
        *p = '\0';

        // if can't create parent
        if (!IU_CreateFolder(szPath))
        {
            free(szPath);
            return FALSE;
        }
        free(szPath);

        if (!::CreateDirectory(szFolder, NULL))
            return FALSE;
    }

    return TRUE;
}

CMyEngineList* _EngineList;

void DecodeString(LPCTSTR szSource, CString& Result, LPSTR code)
{
    TCHAR szDestination[1024];
    int br = strlen(code);
    int n = lstrlen(szSource) / 2;
    int j = 0;
    ZeroMemory(szDestination, n * 2);

    int i;
    PBYTE data = (PBYTE)szDestination;
    *szDestination = 0;

    for (i = 0; i < n; i++)
    {
        if (j >= br)
            j = 0;

        BYTE b;
        b = static_cast<BYTE>((szSource[i * 2] - _T('A')) * 16 + (szSource[i * 2 + 1] - _T('A')));
        b = b ^ code[j];
        data[i] = b;
        j++;
    }
    data[i] = 0;
    Result = szDestination;
}

void EncodeString(LPCTSTR szSource, CString& Result, LPSTR code)
{
    TCHAR szDestination[1024];
    int br = strlen(code);
    int n = lstrlen(szSource) * 2;
    int j = 0;

    PBYTE data = (PBYTE)szSource;
    *szDestination = 0;
    for (int i = 0; i < n; i++)
    {
        if (j >= br)
            j = 0;

        BYTE b;
        b = data[i] ^ code[j];
        TCHAR bb[2] = {0, 0};
        bb[0] = _T('A') + b / 16;
        lstrcat(szDestination, bb);
        bb[0] = _T('A') + b % 16;
        lstrcat(szDestination, bb);
        j++;
    }
    Result = szDestination;
}

BOOL IU_CreateFilePath(LPCTSTR szFilePath)
{
    TCHAR* szPath = _tcsdup(szFilePath);
    TCHAR* p = _tcsrchr(szPath, '\\');

    BOOL bRes = FALSE;

    if (p)
    {
        *p = '\0';

        bRes = IU_CreateFolder(szPath);
    }

    free(szPath);

    return bRes;
}

HICON GetAssociatedIcon (LPCTSTR filename, bool Small)
{
    SHFILEINFO Info;
    DWORD Flags;

    if (Small)
        Flags = SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;
    else
        Flags = SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES | SHGFI_ADDOVERLAYS;
    SHGetFileInfo (filename, FILE_ATTRIBUTE_NORMAL, &Info, sizeof(Info), Flags);
    return Info.hIcon;
}

int ScreenBPP()
{
// Возвращает количество битов на точку в данном режиме
    int iRet = 0;
    HDC hdc = GetDC(NULL);
    if (hdc != NULL)
    {
        iRet = GetDeviceCaps(hdc, BITSPIXEL);
        ReleaseDC(NULL, hdc);
    }
    return iRet;
}

BOOL Is32BPP()
{
    return (WinUtils::IsWinXP() & (ScreenBPP() >= 32));
}

CString GetSystemSpecialPath(int csidl)
{
    CString result;
    LPITEMIDLIST pidl;
    TCHAR szSendtoPath [MAX_PATH];
    LPMALLOC pMalloc;

    if (SUCCEEDED( SHGetSpecialFolderLocation ( NULL, csidl, &pidl )))
    {
        if (SHGetPathFromIDList(pidl, szSendtoPath))
        {
            result = szSendtoPath;
        }

        if (SUCCEEDED(SHGetMalloc(&pMalloc)))
        {
            pMalloc->Free ( pidl );
            pMalloc->Release();
        }
    }
    if (result.Right(1) != _T("\\"))
        result += _T("\\");
    return result;
}

const CString GetApplicationDataPath()
{
    return GetSystemSpecialPath(CSIDL_APPDATA);
}

const CString GetCommonApplicationDataPath()
{
    return GetSystemSpecialPath(CSIDL_COMMON_APPDATA);
}

HRESULT IsElevated( __out_opt BOOL* pbElevated )  // = NULL )
{
    //ATLASSERT( IsVista() );

    HRESULT hResult = E_FAIL; // assume an error occured
    HANDLE hToken  = NULL;

    if ( !::OpenProcessToken(
            ::GetCurrentProcess(),
            TOKEN_QUERY,
            &hToken ) )
    {
        ATLASSERT( FALSE );
        return hResult;
    }

    TOKEN_ELEVATION te = { 0 };
    DWORD dwReturnLength = 0;

    if ( !::GetTokenInformation(
            hToken,
            (TOKEN_INFORMATION_CLASS) TokenElevation,
            &te,
            sizeof(te),
            &dwReturnLength ) )
    {
        //ATLASSERT( FALSE );
    }
    else
    {
        ATLASSERT( dwReturnLength == sizeof(te) );

        hResult = te.TokenIsElevated ? S_OK : S_FALSE;

        if ( pbElevated)
            *pbElevated = (te.TokenIsElevated != 0);
    }

    ::CloseHandle( hToken );

    return hResult;
}

// Function that gets path to SendTo folder
CString GetSendToPath()
{
    CString result;
    LPITEMIDLIST pidl;
    TCHAR szSendtoPath [MAX_PATH];
    LPMALLOC pMalloc;

    if (SUCCEEDED( SHGetSpecialFolderLocation ( NULL, CSIDL_SENDTO, &pidl )))
    {
        if (SHGetPathFromIDList(pidl, szSendtoPath))
        {
            result = szSendtoPath;
        }

        if (SUCCEEDED(SHGetMalloc(&pMalloc)))
        {
            pMalloc->Free ( pidl );
            pMalloc->Release();
        }
    }
    return result;
}
