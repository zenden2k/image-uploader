#ifndef FUNC_WINUTILS_H
#define FUNC_WINUTILS_H

#include "atlheaders.h"
#include <vector>

namespace Gdiplus {
    class Bitmap;
}

const std::wstring Utf8ToWstring(const std::string &str);
#define MYRGB(a,color) Color(a,GetRValue(color),GetGValue(color),GetBValue(color))
#define WstrToUtf8(str) IuCoreUtils::WstringToUtf8(str)
//wstostr(str, CP_UTF8)
#define WCstringToUtf8(str) WinUtils::wstostr(((LPCTSTR)(str)), CP_UTF8)
#define Utf8ToWCstring(str) CString(Utf8ToWstring(str).c_str())

namespace WinUtils {
    bool IsWinXP();
    bool IsWinXPOrLater();
    bool IsDirectory(LPCTSTR szFileName);
    bool IsVista();
    bool IsWindows64Bit();
    bool IsWindows8orLater();
    bool IsWine();
    // Function that gets path to SendTo folder
    CString GetSendToPath();
    CString GetSystemSpecialPath(int csidl);
    CString GetApplicationDataPath();
    CString GetCommonApplicationDataPath();
    bool    CopyTextToClipboard(const CString& text);
    bool    GetClipboardText(CString &text, HWND hwnd = NULL, bool raiseError = false);
    bool CopyHtmlToClipboard(const CString& text);
    bool GetClipboardHtml(CString& text, CString& outSourceUrl);
    DWORD MsgWaitForSingleObject(HANDLE pHandle, DWORD dwMilliseconds);

    bool CreateShortCut( 
        LPCWSTR pwzShortCutFileName, 
        LPCTSTR pszPathAndFileName, 
        LPCTSTR pszWorkingDirectory, 
        LPCTSTR pszArguments, 
        WORD wHotKey, 
        int iCmdShow, 
        LPCTSTR pszIconFileName, 
        int iIconIndex) ;

    bool CreateFolder(LPCTSTR szFolder);
    bool CreateFilePath(LPCTSTR szFilePath);
    HICON GetAssociatedIcon (LPCTSTR filename, bool Small);

    // File path functions
    LPTSTR ExtractFilePath(LPCTSTR FileName,LPTSTR buf);
    CString myExtractFileName(const CString & FileName);
    LPCTSTR GetFileExt(LPCTSTR szFileName);
    CString GetOnlyFileName(const CString& szFilename);
    bool FileExists(LPCTSTR FileName);
    const CString TrimString(const CString& source, int nMaxLen);
    bool IsStrInList(LPCTSTR szExt,LPCTSTR szList);
    bool NewBytesToString(__int64 nBytes, LPTSTR szBuffer, int nBufSize);
    bool ShowFilePropertiesDialog(HWND hWnd, const CString& fileName);
    CString IntToStr(int n);
    CString GetModuleFullName(HMODULE module = NULL);
    CString GetAppFolder();
    CString GetAppFileName();
    CString FormatWindowsErrorMessage(int idCode);
    bool IsElevated();
    void DeleteDir2(LPCTSTR Dir);
    CString GetUniqFileName(const CString& filePath);
    size_t GetFolderFileList(std::vector<CStringT<wchar_t, StrTraitATL<wchar_t, ChTraitsCRT<wchar_t>>>>& list, CStringT<wchar_t, StrTraitATL<wchar_t, ChTraitsCRT<wchar_t>>> folder, CStringT<wchar_t, StrTraitATL<wchar_t, ChTraitsCRT<wchar_t>>> mask);

    inline COLORREF RGB2COLORREF(unsigned int color) {
        return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
    }

    inline unsigned int COLORREF2RGB( COLORREF color){
        return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
    }

    bool FontToString(LOGFONT const * lFont, CString &Result);
    bool StringToFont(LPCTSTR szBuffer,LPLOGFONT lFont);
    RECT AutoSizeStaticControl(HWND control);

    bool ExtractStrFromList(
        LPCTSTR szString /* Source string */,
        int nIndex, /* Zero based item index */
        LPTSTR szBuffer /* Destination buffer */,
        LONG nSize ,/* Length in characters of destination buffer */
        LPCTSTR szDefString = NULL,
        TCHAR cSeparator = _T(',') /* Character to be separator in list */);
    const CString StringSection(const CString& str,TCHAR sep, int index);
    bool IsWindows64Bit();

    void DeleteDir2(LPCTSTR Dir);
    CString GetAppFolder();
    float GetMonitorScaleFactor();
    CString GetLastErrorAsString();
    BOOL MakeDirectoryWritable(LPCTSTR lpPath);
    int GetInternetExplorerMajorVersion();
    void RemoveBrowserKey();
    void UseLatestInternetExplorerVersion(bool IgnoreIDocDirective = false);
    void TimerWait(int Delay);
    CString ConvertRelativePathToAbsolute(const CString& fileName);
    bool IsProcessRunning(DWORD pid);
//#ifndef IU_SHELLEXT
    std::wstring strtows(const std::string &str, UINT codePage);
    std::string wstostr(const std::wstring &ws, UINT codePage);
    const std::string AnsiToUtf8(const std::string &str, int codepage);
    const std::string Utf8ToAnsi(const std::string &str, int codepage);
    CString GetProcessName(DWORD pid);
    CString ErrorCodeToString(DWORD idCode);
    CString ExpandEnvironmentStrings(const CString& s);
    void ArgvQuote(const std::wstring& Argument, std::wstring& CommandLine, bool Force);
    bool GetProxyInfo(CString& proxy_address, CString& proxy_bypass);
    std::string TextToClipboardHtmlFormat(const char* html, int length, const std::string& base_url = std::string());
//#endif
};


#endif