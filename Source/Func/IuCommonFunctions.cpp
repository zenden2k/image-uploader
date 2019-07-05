#include "IuCommonFunctions.h"

#include "WinUtils.h"
#include "Core/Utils/CoreUtils.h"
#include "3rdpart/Registry.h"
#include "Core/Utils/CryptoUtils.h"
#include "Core/AppParams.h"

namespace IuCommonFunctions {

CString GetDataFolder()
{
    CString result = U2W(AppParams::instance()->dataDirectory());
/*#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)&& !defined(IU_TESTS)
    result= Settings.DataFolder;
#else 
    result= WinUtils::GetAppFolder()+"\\Data\\";
#endif
    */
    if (result.Right(1) != "\\" && result.Right(1) != "/") {
        result += "\\";
    }
    return result;
}

CString GetVersion()
{
    auto ver = AppParams::instance()->GetAppVersion();

    return CString(ver->FullVersion.c_str()) + _T(".") + CString(std::to_string(ver->Build).c_str());
}

BOOL CreateTempFolder(CString& IUCommonTempFolder, CString& IUTempFolder)
{
    TCHAR ShortPath[1024];
    GetTempPath(ARRAY_SIZE(ShortPath), ShortPath);
    TCHAR TempPath[1024];
    if (!GetLongPathName(ShortPath,TempPath, ARRAY_SIZE(TempPath)) ) {
        lstrcpy(TempPath, ShortPath);
    }
    DWORD pid = GetCurrentProcessId() ^ 0xa1234568;
    IUCommonTempFolder.Format(_T("%stmd_iu_temp"), static_cast<LPCTSTR>(TempPath));

    if (!CreateDirectory(IUCommonTempFolder, 0)) {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_ALREADY_EXISTS) {
            LOG(ERROR) << "Unable to create temp folder: " << std::endl 
                << IUCommonTempFolder << std::endl << WinUtils::ErrorCodeToString(errorCode);
            return false;
        }
    }
    IUTempFolder.Format(_T("%s\\iu_temp_%x"), static_cast<LPCTSTR>(IUCommonTempFolder), pid);

    if (!CreateDirectory(IUTempFolder, 0) ) {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_ALREADY_EXISTS) {
            LOG(ERROR) << "Unable to create temp folder: " << std::endl 
                << IUTempFolder << std::endl  << WinUtils::ErrorCodeToString(errorCode);
            return false;
        }
    }

    IUTempFolder += _T("\\");
    return TRUE;
}

WIN32_FIND_DATA wfd;
HANDLE findfile = 0;

int GetNextImgFile(LPCTSTR folder, CString& szBuffer)
{
    CString buffer2 = folder + CString(_T("*.*"));

    if (!findfile)
    {
        findfile = FindFirstFile(buffer2, &wfd);
        if (!findfile)
            goto error;
    }
    else
    {
        if (!FindNextFile(findfile, &wfd))
            goto error;
    }
    if (lstrlen(wfd.cFileName) < 1)
        goto error;

    szBuffer = wfd.cFileName;

    return TRUE;

error:
    if (findfile)
        FindClose(findfile);
    return FALSE;

}
void ClearTempFolder(CString folder)
{
    if (folder.IsEmpty()) {
        return;
    }
    int lastCharPos = folder.GetLength() - 1;
    if (folder[lastCharPos] != _T('\\') && folder[lastCharPos] != _T('/')) {
        folder += _T("\\");
    }
    CString buffer;
    CString buffer2;

    findfile = 0;
    while (GetNextImgFile(folder, buffer))
    {
#ifdef DEBUG
        if (buffer == _T("log.txt")) {
            continue;
        }
#endif
        if (buffer == _T(".") || buffer == _T("..")) {
            continue;
        }
        buffer2 = CString(folder) + buffer;
        DeleteFile(buffer2);
    }
    if (!RemoveDirectory(folder))
    {
        WinUtils::DeleteDir2(folder);
    }
}

CString FindDataFolder()
{
    CString DataFolder;
    if (WinUtils::IsDirectory(WinUtils::GetAppFolder() + _T("Data"))) {
        DataFolder     = WinUtils::GetAppFolder() + _T("Data\\");
        return DataFolder;
    }

#if !defined(IU_SERVERLISTTOOL) && !defined  (IU_CLI)
    {
        CRegistry Reg;

        Reg.SetRootKey(HKEY_CURRENT_USER);
        if (Reg.SetKey(_T("Software\\Zenden.ws\\Image Uploader"), false))
        {
            CString dir = Reg.ReadString(_T("DataPath"));

            if (!dir.IsEmpty() && WinUtils::IsDirectory(dir))
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
            if (!dir.IsEmpty() && WinUtils::IsDirectory(dir))
            {
                DataFolder = dir;
                return DataFolder;
            }
        }
    }

    if (WinUtils::FileExists(WinUtils::GetCommonApplicationDataPath() + L"Settings.xml")) {
        DataFolder = WinUtils::GetCommonApplicationDataPath() + _T("Image Uploader\\");
    }
    else 
#endif

    {
        DataFolder = WinUtils::GetApplicationDataPath() + _T("Image Uploader\\");
    }
    return DataFolder;
}

CString GenerateFileName(const CString& templateStr, int index, const CPoint& size, const CString& originalName)
{
    CString result = templateStr;
    time_t t = time(0);
    tm* timeinfo = localtime(&t);
    CString indexStr;
    CString day, month, year;
    CString hours, seconds, minutes;
    indexStr.Format(_T("%03d"), index);
    std::thread::id threadId = std::this_thread::get_id();
    CString md5 = Utf8ToWstring(IuCoreUtils::CryptoUtils::CalcMD5HashFromString(IuCoreUtils::ThreadIdToString(threadId) + IuCoreUtils::int64_tToString(GetTickCount() + rand() % (100)))).c_str();
    result.Replace(_T("%md5"), (LPCTSTR)md5);
    result.Replace(_T("%width%"), WinUtils::IntToStr(size.x));
    result.Replace(_T("%height%"), WinUtils::IntToStr(size.y));
    year.Format(_T("%04d"), 1900 + timeinfo->tm_year);
    month.Format(_T("%02d"), timeinfo->tm_mon + 1);
    day.Format(_T("%02d"), timeinfo->tm_mday);
    hours.Format(_T("%02d"), timeinfo->tm_hour);
    seconds.Format(_T("%02d"), timeinfo->tm_sec);
    minutes.Format(_T("%02d"), timeinfo->tm_min);
    result.Replace(_T("%y"), year);
    result.Replace(_T("%m"), month);
    result.Replace(_T("%d"), day);
    result.Replace(_T("%h"), hours);
    result.Replace(_T("%n"), minutes);
    result.Replace(_T("%s"), seconds);
    result.Replace(_T("%i"), indexStr);
    return result;
}


bool IsImage(LPCTSTR szFileName)
{
    LPCTSTR szExt = WinUtils::GetFileExt(szFileName);
    if (lstrlen(szExt)<1) return false;
    return WinUtils::IsStrInList(szExt, _T("jpg\0jpeg\0png\0bmp\0gif\0tif\0tiff\0webp\0\0"));
}

};