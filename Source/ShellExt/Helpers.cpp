#include "Helpers.h"

#include <algorithm>
#include <string>
#include <memory>

#include "Core/CommonDefs.h"
#include "../3rdpart/Registry.h"

namespace Helpers {

bool IsVistaOrLater() {
    static int isVista = -1;
    if (isVista == -1) {
        OSVERSIONINFO osver;
        osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        isVista = (::GetVersionEx(&osver) &&
            osver.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            (osver.dwMajorVersion >= 6));
    }
    return isVista != FALSE;
}

bool FileExists(LPCWSTR FileName) {
    return (FileName && GetFileAttributesW(FileName) != -1);
}

LPWSTR ExtractFilePath(LPCWSTR FileName, LPWSTR buf, size_t bufferSize) {
    int i, len = lstrlen(FileName);
    for (i = len; i >= 0; i--)
    {
        if (FileName[i] == _T('\\') || FileName[i] == _T('/'))
            break;
    }
    lstrcpynW(buf, FileName, (std::min)(i + 2, static_cast<int>(bufferSize) - 1));
    return buf;
}

//  Function doesn't allocate new string, it returns  pointer
//        to a part of source string
LPCWSTR GetFileExt(LPCWSTR szFileName)
{
    if (!szFileName) return 0;
    int nLen = lstrlenW(szFileName);

    LPCWSTR szReturn = szFileName + nLen;
    for (int i = nLen - 1; i >= 0; i--)
    {
        if (szFileName[i] == '.')
        {
            szReturn = szFileName + i + 1;
            break;
        }
        else if (szFileName[i] == '\\' || szFileName[i] == '/') break;
    }
    return szReturn;
}

bool IsImage(LPCWSTR szFileName) {
    if (!szFileName) {
        return false;
    }
    LPCWSTR szExt = GetFileExt(szFileName);
    if (lstrlen(szExt) <= 0) {
        return false;
    }
    std::wstring extensions[]{
        L"jpg", L"jpeg", L"png", L"bmp", L"gif", L"tif", L"tiff", L"webp"
    };
    for (const auto& ext: extensions) {
        if (!lstrcmpiW(ext.c_str(), szExt)) {
            return true;
        }
    }
    return false;
}

bool IsDirectory(LPCWSTR szFileName){
    DWORD res = GetFileAttributesW(szFileName);
    return (res & FILE_ATTRIBUTE_DIRECTORY) && (res != INVALID_FILE_ATTRIBUTES);
}

CString GetSystemSpecialPath(int csidl) {
    CString result;
    LPITEMIDLIST pidl;
    TCHAR szSendtoPath[MAX_PATH];
    LPMALLOC pMalloc;

    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, &pidl)))
    {
        if (SHGetPathFromIDList(pidl, szSendtoPath))
        {
            result = szSendtoPath;
        }

        if (SUCCEEDED(SHGetMalloc(&pMalloc)))
        {
            pMalloc->Free(pidl);
            pMalloc->Release();
        }
    }
    if (result.Right(1) != _T("\\"))
        result += _T("\\");
    return result;
}

CString GetApplicationDataPath()
{
    return GetSystemSpecialPath(CSIDL_APPDATA);
}

CString GetCommonApplicationDataPath()
{
    return GetSystemSpecialPath(CSIDL_COMMON_APPDATA);
}

CString GetModuleFullName(HMODULE module) {
    std::unique_ptr<WCHAR[]> buf;
    DWORD  bufLen = MAX_PATH;
    DWORD  retLen;
    CString res;

    while (32768 >= bufLen) {
        buf.reset(new WCHAR[bufLen]);

        if ((retLen = GetModuleFileNameW(module, buf.get(), bufLen)) == 0) {
            /* GetModuleFileName failed */
            break;
        }
        else if (bufLen > retLen) {
            /* Success */
            res = buf.get();
            break;
        }

        bufLen *= 2;
    }

    return res;
}

CString GetAppFolder() {
    CString szPath;
    CString szFileName = GetModuleFullName(hDllInstance);
    ExtractFilePath(szFileName, szPath.GetBuffer(szFileName.GetLength() + 1), szFileName.GetLength() + 1);
    szPath.ReleaseBuffer();
    return szPath;
}

CString FindDataFolder() {
    CString DataFolder;
    if (IsDirectory(GetAppFolder() + _T("Data"))) {
        DataFolder = GetAppFolder() + _T("Data\\");
        return DataFolder;
    }

    {
        CRegistry Reg;

        Reg.SetRootKey(HKEY_CURRENT_USER);
        if (Reg.SetKey(_T("Software\\Zenden.ws\\Image Uploader"), false))
        {
            CString dir = Reg.ReadString(_T("DataPath"));

            if (!dir.IsEmpty() && IsDirectory(dir))
            {
                DataFolder = dir;
                return DataFolder;
            }
        }
    }
    {
        CRegistry Reg;
        Reg.SetRootKey(HKEY_LOCAL_MACHINE);
        // Unable to use wow64 flag because of Registry Virtualization enabled in the explorer.exe process
        CString keyStr =
#ifdef _WIN64
            _T("Software\\Wow6432Node\\Zenden.ws\\Image Uploader");
#else
            _T("Software\\Zenden.ws\\Image Uploader");
#endif

        if (Reg.SetKey(keyStr, false))
        {
            CString dir = Reg.ReadString(_T("DataPath"));
            if (!dir.IsEmpty() && IsDirectory(dir))
            {
                DataFolder = dir;
                return DataFolder;
            }
        }
    }

    if (FileExists(GetCommonApplicationDataPath() + L"Settings.xml")) {
        DataFolder = GetCommonApplicationDataPath() + _T("Image Uploader\\");
    }
    else {
        DataFolder = GetApplicationDataPath() + _T("Image Uploader\\");
    }
    return DataFolder;
}

bool IsVideoFile(LPCWSTR szFileName){
    CString extension = GetFileExt(szFileName);
    extension.MakeLower();

    CString formats(IU_VIDEOFILES_EXTENSIONS);
    int nTokenPos = 0;
    CString separator(IU_VIDEOFILES_EXTENSIONS_SEPARATOR);
    CString strToken = formats.Tokenize(separator, nTokenPos);

    while (!strToken.IsEmpty()) {
        strToken = formats.Tokenize(separator, nTokenPos);
        if (extension == strToken) {
            return true;
        }
    }

    return false;
}

}
