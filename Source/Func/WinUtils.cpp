#include "WinUtils.h"

#include "Core/Utils/CoreUtils.h"
#include <sstream>
#include "Core/Utils/StringUtils.h"
#include "3rdpart/GdiplusH.h"
#include <Aclapi.h>
#include "3rdpart/Registry.h"
#include "TlHelp32.h"

namespace WinUtils {

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

bool IsWinXP()
{
    // Проверка операционной системы
    DWORD dwVersion = GetVersion();

    // Get major and minor version numbers of Windows
    DWORD dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    DWORD dwWindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Check for Windows XP
    if ((dwVersion < 0x80000000) &&                 // The OS is a NT family
        (dwWindowsMajorVersion >= 5) &&
        (dwWindowsMinorVersion >= 1))         // Windows NT 5.1 is an Windows XP version
        return TRUE;

    return FALSE;
}

bool IsWinXPOrLater()
{
    // Проверка операционной системы
    DWORD dwVersion = GetVersion();

    // Get major and minor version numbers of Windows
    DWORD dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    DWORD dwWindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Check for Windows XP
    if ((dwVersion < 0x80000000) &&                 // The OS is a NT family
        (dwWindowsMajorVersion > 5) ||
        (dwWindowsMajorVersion == 5 && dwWindowsMinorVersion >= 1))         // Windows NT 5.1 is an Windows XP version
        return TRUE;

    return FALSE;
}

// Function that gets path to SendTo folder
CString GetSendToPath() {
    return GetSystemSpecialPath(CSIDL_SENDTO);
}

CString GetApplicationDataPath()
{
    return GetSystemSpecialPath(CSIDL_APPDATA);
}

CString GetCommonApplicationDataPath()
{
    return GetSystemSpecialPath(CSIDL_COMMON_APPDATA);
}

CString MyGetWindowText(HWND wnd) {
    int len = GetWindowTextLength(wnd);
    CString buf;
    GetWindowText(wnd, buf.GetBuffer(len + 1), len + 1);
    buf.ReleaseBuffer(-1);
    return buf;
}

bool GetClipboardText(CString& text, HWND hwnd, bool raiseError)
{
    for (int i = 0; i < 4; i++) {
        if (OpenClipboard(hwnd)) {
            HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
            if (!hglb) {
                if (raiseError) {
                    LOG(ERROR) << "GetClipboardData call failed. ErrorCode=" << ::GetLastError();
                }
                
                CloseClipboard();
                return false;
            }
            LPCWSTR lpstr = reinterpret_cast<LPCWSTR>(GlobalLock(hglb));
            text = lpstr;
            GlobalUnlock(hglb);
            CloseClipboard();
            return true;
        }
        Sleep(50); // Clipboard may be owned by another application, wait and try again
    }
   
    if ( raiseError ) {
        DWORD lastError = ::GetLastError();
        CString message;
        HWND clipboardOwner = GetClipboardOwner();
        if ( clipboardOwner ) {
            CString windowTitle = MyGetWindowText(clipboardOwner);
            TCHAR windowClassName[256] = _T("");
            GetClassName(clipboardOwner, windowClassName, 255);
            DWORD proccessId;
            GetWindowThreadProcessId(clipboardOwner, &proccessId);
            message += _T("\r\n");
            message += _T("Clipboard is owned by window:\r\n");
            message += _T("Title: '") + windowTitle + _T("'\r\n");
            message += _T("Class: '") + CString(windowClassName) + _T("'\r\n");
            message += _T("Process: '") + WinUtils::GetProcessName(proccessId) + _T("' (PID=") + IntToStr(proccessId) + _T(")\r\n");
        }
        LOG(ERROR) << "OpenClipboard call failed. ErrorCode=" << lastError << message;
    }
    return false;
}

bool CopyTextToClipboard(const CString& text)
{
    LPTSTR lptstrCopy;
    HGLOBAL hglbCopy;
    int cch = text.GetLength();
    if (!OpenClipboard( NULL))
        return FALSE;
    EmptyClipboard();
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (cch + 1) * sizeof(TCHAR));
    if (hglbCopy == NULL)
    {
        CloseClipboard();
        return FALSE;
    }
    lptstrCopy = reinterpret_cast<LPTSTR>(GlobalLock(hglbCopy));
    memcpy(lptstrCopy, static_cast<LPCTSTR>(text), text.GetLength() * sizeof(TCHAR));
    lptstrCopy[cch] = 0;
    GlobalUnlock(hglbCopy);
    SetClipboardData(CF_UNICODETEXT, hglbCopy);
    CloseClipboard();
    return true;
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
bool CreateShortCut(
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
    hRes = CoCreateInstance(CLSID_ShellLink, 0,  CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<LPVOID*>(&pSL));

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
                                hRes = pSL->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&pPF));
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

bool CreateFolder(LPCTSTR szFolder)
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
        if (!CreateFolder(szPath))
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

bool CreateFilePath(LPCTSTR szFilePath)
{
    TCHAR* szPath = _tcsdup(szFilePath);
    TCHAR* p = _tcsrchr(szPath, '\\');

    BOOL bRes = FALSE;

    if (p)
    {
        *p = '\0';

        bRes = CreateFolder(szPath);
    }

    free(szPath);

    return bRes!=FALSE;
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

bool IsDirectory(LPCTSTR szFileName)
{
    DWORD res = GetFileAttributes(szFileName);
    return (res&FILE_ATTRIBUTE_DIRECTORY) && (res != -1);    
}

bool IsVista() {
    static int isVista = -1;
    if (isVista == -1)
    {
        OSVERSIONINFO osver;
        osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        isVista = (::GetVersionEx(&osver) &&
            osver.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            (osver.dwMajorVersion >= 6));
    }
    return isVista != FALSE;
}

bool IsWindows8orLater() {
    OSVERSIONINFO osver;
    osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    if ( ::GetVersionEx( &osver ) && 
        osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
        ( (osver.dwMajorVersion > 6 ) || (osver.dwMajorVersion == 6 && osver.dwMinorVersion >=2)) ) {
        return TRUE;
    }

    return FALSE;
}

bool IsWine()
{
    HMODULE hDll = LoadLibrary(_T("ntdll.dll"));

    if (hDll)
    {
        return GetProcAddress(hDll, "wine_get_version") != 0;
    }
    return false;
}

bool IsWindows64Bit()
{
    SYSTEM_INFO si;
    PGNSI pGNSI;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = reinterpret_cast<PGNSI>(GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo"));
    if (NULL != pGNSI) {
        pGNSI(&si);
    } else {
        GetSystemInfo(&si);
    }
    return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;    
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

CString FormatWindowsErrorMessage(int idCode)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,
        idCode, 0, reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL);
    CString res = reinterpret_cast<LPCTSTR>(lpMsgBuf);
    // Free the buffer.
    LocalFree( lpMsgBuf );
    return res;
}

bool FileExists(LPCTSTR FileName)
{
    return (FileName && GetFileAttributes(FileName)!=-1);
}

const CString TrimString(const CString& source, int nMaxLen)
{
    int nLen = source.GetLength();
    if(nLen <= nMaxLen) return source;

    int PartSize = (nMaxLen-3) / 2;
    return source.Left(PartSize)+_T("...")+source.Right(PartSize);
}

CString GetAppFolder()
{
    TCHAR szFileName[1024], szPath[1024];
    HINSTANCE inst;
#if defined(IU_WTL)
    inst =  _Module.GetModuleInstance();
#elif defined(IU_SHELLEXT)
    inst = hDllInstance;
#else
    inst = GetModuleHandle(0);
#endif
    GetModuleFileName(inst, szFileName, 1023);
    ExtractFilePath(szFileName, szPath);
    return szPath;
}

CString GetAppFileName() {
    TCHAR szFileName[1024];
    GetModuleFileName(0, szFileName, 1023);
    return szFileName;
}

LPTSTR ExtractFilePath(LPCTSTR FileName, LPTSTR buf)
{  
    int i, len = lstrlen(FileName);
    for(i=len; i>=0; i--)
    {
        if(FileName[i] == _T('\\'))
            break;
    }
    lstrcpyn(buf, FileName, i+2);
    return buf;
}

//  Function doesn't allocate new string, it returns  pointer
//        to a part of source string
CString myExtractFileName(const CString & FileName)
{  
    CString temp = FileName;
    int Qpos = temp.ReverseFind('?');
    if(Qpos>=0) temp = temp.Left(Qpos);
    int i,len = lstrlen(temp);
    for(i=len-1; i>=0; i--)
    {
        if(temp[i] == _T('\\') || temp[i]==_T('/'))
            break;
    }
    return temp.Right(len-i-1);

}

//  Function doesn't allocate new string, it returns  pointer
//        to a part of source string
LPCTSTR GetFileExt(LPCTSTR szFileName)
{
    if(!szFileName) return 0;
    int nLen = lstrlen(szFileName);

    LPCTSTR szReturn = szFileName+nLen;
    for( int i=nLen-1; i>=0; i-- )
    {
        if(szFileName[i] == '.')
        {
            szReturn = szFileName + i + 1;
            break;
        }
        else if(szFileName[i] == '\\' || szFileName[i] == '/') break;
    }
    return szReturn;
}

bool IsStrInList(LPCTSTR szExt,LPCTSTR szList)
{
    if(!szList || !szExt) return false;

    while ((*szList)!=0)
    {
        if(lstrcmpi(szExt,szList)==0) return true;
        szList += lstrlen(szList)+1;
    }
    return false;
}

CString GetOnlyFileName(const CString& szFilename) {
    CString tempName = myExtractFileName(szFilename);
    int dotPos = tempName.ReverseFind(_T('.'));
    if(dotPos != -1) {
        tempName=tempName.Left(dotPos);
    }

    return tempName;
}

CString IntToStr(int n)
{
    CString Result;
    Result.Format(_T("%d"), n);
    return Result;
}

// Преобразование размера файла в строку
bool NewBytesToString(__int64 nBytes, LPTSTR szBuffer, int nBufSize)
{
    std::string res = IuCoreUtils::fileSizeToString(nBytes);
    lstrcpyn(szBuffer, IuCoreUtils::Utf8ToWstring(res).c_str(), nBufSize);
    return TRUE;
}


bool IsElevated() 
{
    BOOL pbElevated = false;
    ATLASSERT( IsVista() );

    HRESULT hResult = E_FAIL; // assume an error occured
    HANDLE hToken  = NULL;

    if ( !::OpenProcessToken(
        ::GetCurrentProcess(),
        TOKEN_QUERY,
        &hToken ) )
    {
        ATLASSERT( FALSE );
        return hResult!=0;
    }

    TOKEN_ELEVATION te = { 0 };
    DWORD dwReturnLength = 0;

    if ( !::GetTokenInformation(
        hToken,
        static_cast<TOKEN_INFORMATION_CLASS>(TokenElevation),
        &te,
        sizeof(te),
        &dwReturnLength ) )
    {
        ATLASSERT( FALSE );
    }
    else
    {
        ATLASSERT( dwReturnLength == sizeof(te) );

        hResult = te.TokenIsElevated ? S_OK : S_FALSE;

        //if ( pbElevated)
            pbElevated = (te.TokenIsElevated != 0);
    }

    ::CloseHandle( hToken );

    return pbElevated!=FALSE;
}


void DeleteDir2(LPCTSTR Dir)
{
    if (!Dir)
        return;
    TCHAR szBuffer[MAX_PATH];
    lstrcpyn(szBuffer, Dir, MAX_PATH);
    int nLen = lstrlen(szBuffer) - 1;
    if (nLen >= 0 && szBuffer[nLen] == _T('\\'))
        szBuffer[nLen] = 0;

    SHFILEOPSTRUCT FileOp;
    ZeroMemory(&FileOp, sizeof(FileOp));
    FileOp.hwnd = NULL;
    FileOp.wFunc = FO_DELETE;
    FileOp.pFrom = szBuffer;
    FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
    SHFileOperation(&FileOp);
}

CString GetUniqFileName(const CString& filePath)
{
    TCHAR path[256];
    if (!WinUtils::FileExists(filePath))
        return filePath;
    WinUtils::ExtractFilePath(filePath, path);
    CString name;
    name = WinUtils::GetOnlyFileName(filePath);
    CString extension = WinUtils::GetFileExt(filePath);
    CString result;
    for (int i = 2;; i++)
    {
        result = path + name + WinUtils::IntToStr(i) + (extension.IsEmpty() ? _T("") : _T(".") + extension);
        if (!WinUtils::FileExists(result))
            break;
    }
    return result;
}

size_t GetFolderFileList(std::vector<CString>& list, CString folder, CString mask)
{
    WIN32_FIND_DATA wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    HANDLE findfile = 0;
    TCHAR szNameBuffer[MAX_PATH];

    for (;; )
    {
        if (!findfile)
        {
            findfile = FindFirstFile(folder + _T("\\") + mask, &wfd);
            if (!findfile)
                break;
            ;
        }
        else
        {
            if (!FindNextFile(findfile, &wfd))
                break;
        }
        if (lstrlen(wfd.cFileName) < 1)
            break;
        lstrcpyn(szNameBuffer, wfd.cFileName, 254);
        list.push_back(szNameBuffer);
    }
    // return TRUE;

    // error:
    if (findfile)
        FindClose(findfile);
    return list.size();
    // return FALSE;
}

bool FontToString(const LOGFONT * lFont, CString &Result)
{
    TCHAR  szBuffer[1024];
    if( !lFont  ) return false;

    int nPixelsPerInch;
    int nFontSize;
    //HDC hScreenDC;
    bool bBold = false;
    bool bItalic = false;
    bool bUnderline = false;
    bool bStrikeOut = false;
    TCHAR szBold[2][2]={_T("\0"),_T("b")};
    TCHAR szItalic[2][2]={_T("\0"),_T("i")};
    TCHAR szUnderline[2][2]={_T("\0"),_T("u")};
    TCHAR szStrikeOut[2][2]={_T("\0"),_T("s")};

    //hScreenDC = ::GetDC( NULL );

    //if( !hScreenDC ) return false;
    HDC dc = ::GetDC(0);
    nPixelsPerInch = GetDeviceCaps(dc , LOGPIXELSY );
    ReleaseDC(0, dc);

    if( nPixelsPerInch <= 0 ) return false;

    nFontSize = - MulDiv( lFont->lfHeight, 72, nPixelsPerInch );

    if(lFont->lfWeight >= FW_BOLD) bBold = true;

    if(lFont->lfUnderline == (char)true) bUnderline = true;
    if(lFont->lfItalic) bItalic = true;
    if(lFont->lfStrikeOut == (char)true) bStrikeOut = true;

    wsprintf(szBuffer,_T("%s, %d, %s%s%s%s, %d"),lFont->lfFaceName,nFontSize,
        szBold[bBold],szItalic[bItalic],szUnderline[bUnderline],szStrikeOut[bStrikeOut],(int)lFont->lfCharSet);
    Result = szBuffer;
    return true;
}

bool StringToFont(LPCTSTR szBuffer,LPLOGFONT lFont)
{
    TCHAR szFontName[LF_FACESIZE] = _T("Ms Sans Serif");

    TCHAR szFontSize[MAX_PATH];
    TCHAR szFormat[MAX_PATH];
    TCHAR szCharset[MAX_PATH];
    bool bBold=false;
    bool bItalic=false;
    bool bUnderline=false;
    bool bStrikeOut=false;
    int nFontSize=10;
    int nCharSet=ANSI_CHARSET;

    ExtractStrFromList(szBuffer,0,szFontName,sizeof(szFontName)/sizeof(TCHAR));
    if(ExtractStrFromList(szBuffer,1,szFontSize,sizeof(szFontSize)/sizeof(TCHAR)))
    {
        _stscanf(szFontSize,_T("%d"),&nFontSize);
    }

    ExtractStrFromList(szBuffer,2,szFormat,sizeof(szFontSize)/sizeof(TCHAR));

    if(_tcschr(szFormat, 'b')) bBold=true;
    if(_tcschr(szFormat, 'u')) bUnderline=true;
    if(_tcschr(szFormat, 'i')) bItalic=true;
    if(_tcschr(szFormat, 's')) bStrikeOut=true;

    if( ExtractStrFromList(szBuffer,3,szCharset,sizeof(szCharset)/sizeof(TCHAR)))
    {
        _stscanf(szCharset,_T("%d"),&nCharSet);
    }

    ZeroMemory(lFont,sizeof(LOGFONT));
    lstrcpy(lFont->lfFaceName,szFontName);
    HDC dc = ::GetDC(0);
    lFont->lfHeight = -MulDiv(nFontSize, GetDeviceCaps(dc , LOGPIXELSY), 72);
    ReleaseDC(0, dc);

    lFont->lfItalic=bItalic;
    lFont->lfStrikeOut=bStrikeOut;
    lFont->lfWeight=bBold?FW_BOLD:FW_NORMAL;
    lFont->lfUnderline=bUnderline;
    lFont->lfCharSet=static_cast<BYTE>(nCharSet);

    return true;
}

bool ExtractStrFromList(
                                LPCTSTR szString /* Source string */,
                                int nIndex, /* Zero based item index */
                                LPTSTR szBuffer /* Destination buffer */,
                                LONG nSize, /* Length in characters of destionation buffer */
                                LPCTSTR szDefString,
                                TCHAR cSeparator /* Character to be separator in list */)
{
    int nStringLen = 0;
    LPCTSTR szSeparator,szPrevSep;

    szSeparator=szPrevSep=0;

    int i;
    int nStart,nLen,nNumOfItems;
    nStart=nLen=-1;
    nNumOfItems=1;

    if(!szString || !szBuffer) return false;

    nStringLen = lstrlen(szString);

    if(nStringLen<=0) goto lbl_copydef;

    while(*szString==_T(' ')) szBuffer++;

    if(nStringLen<=0) goto lbl_copydef;

    szPrevSep=szString;

    nStart=0;
    nLen=0;

    for(i=0;i<nStringLen+1;i++)
    {
        if(szString[i]==0)
        {
            if(nIndex<nNumOfItems)
                nLen=i-nStart;
            break;
        }

        else if(szString[i]==cSeparator)
        {
            nNumOfItems++;
            if( nNumOfItems-1 == nIndex)
            {
                nStart=i+1;

            }
            else if( nNumOfItems-2 == nIndex)
            {
                nLen=i-nStart;
                break;

            }
        }
    }

    if(nLen>nSize-1) nLen = nSize-1;

    if(nLen<=0) goto lbl_copydef;

    lstrcpyn(szBuffer, szString + nStart, nLen+1);

    goto lbl_allok;

lbl_copydef:
    if(szDefString) lstrcpy(szBuffer, szDefString);
    return false;

lbl_allok:
    return true;
}

const CString StringSection(const CString& str,TCHAR sep, int index) {
    CString result;
    ExtractStrFromList(str, index, result.GetBuffer(256),256,_T(""),sep);
    result.ReleaseBuffer();
    return result;
}

bool ShowFilePropertiesDialog(HWND hWnd, const CString& fileName) {

    SHELLEXECUTEINFO ShInfo;
    ZeroMemory(&ShInfo, sizeof(SHELLEXECUTEINFO));
    ShInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShInfo.nShow  = SW_SHOW;
    ShInfo.fMask  = SEE_MASK_INVOKEIDLIST | SEE_MASK_IDLIST;
    ShInfo.hwnd   = hWnd;
    ShInfo.lpVerb = TEXT("properties");
    ShInfo.lpFile = fileName;
    return ShellExecuteEx(&ShInfo) != FALSE;
}

bool GetClipboardHtml(CString& text, CString& outSourceUrl) {
    UINT clipboardFormat = RegisterClipboardFormat(_T("HTML Format"));
    if ( OpenClipboard(NULL) ) {
        HGLOBAL hglb = GetClipboardData(clipboardFormat);
        LPCSTR lpstr = reinterpret_cast<LPCSTR>(GlobalLock(hglb));
        std::string ansiString = lpstr;

        std::istringstream f(ansiString);
        std::string line;    
        int startFragment = -1;
        int endFragment = -1;
        std::string sourceUrl;
        bool result = false;

        while (std::getline(f, line)) {
            std::vector<std::string> tokens;
            IuStringUtils::Split(line, ":", tokens, 2);
            if ( tokens.size() == 2) {
                if ( tokens[0] == "StartFragment") {
                    startFragment = atoi(tokens[1].c_str());
                } else if ( tokens[0] == "EndFragment" ) {
                    endFragment = atoi(tokens[1].c_str());
                } else if ( tokens[0] == "SourceURL" ) {
                    sourceUrl = tokens[1];
                }
            } else {
                break;
            }
        }
        if ( startFragment != -1 && endFragment != -1 ) {
            text = IuCoreUtils::Utf8ToWstring( ansiString.substr(startFragment, endFragment - startFragment) ).c_str();
            outSourceUrl = IuCoreUtils::Utf8ToWstring(sourceUrl).c_str();
            result = true;
        }
        
        GlobalUnlock(hglb);
        CloseClipboard();

        return result;
    }

    return false;
}

float GetMonitorScaleFactor()
{
    float res = 1.0;
    /*MONITORINFOEX LogicalMonitorInfo;
    LogicalMonitorInfo.cbSize = sizeof(MONITORINFOEX);       
    GetMonitorInfo(hMonitor, &LogicalMonitorInfo);
    int LogicalMonitorWidth = LogicalMonitorInfo.rcMonitor.right – LogicalMonitorInfo.rcMonitor.left;
    int LogicalDesktopWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    typedef LONG (WINAPI * QueryDisplayConfig_FuncType)(UINT32,UINT32 *,DISPLAYCONFIG_PATH_INFO *,UINT32 *);
    typedef LONG (WINAPI * GetDisplayConfigBufferSizes_FuncType) (UINT32, UINT32*,UINT32 *);

    HMODULE lib = LoadLibrary(_T("user32.dll"));
    QueryDisplayConfig_FuncType QueryDisplayConfigFunc = GetProcAddress(lib, _T("QueryDisplayConfig"));
    GetDisplayConfigBufferSizes_FuncType GetDisplayConfigBufferSizeFunc =  GetProcAddress(lib, _T("GetDisplayConfigBufferSize"));
    if ( QueryDisplayConfigFunc && GetDisplayConfigBufferSizeFunc ) {
        UINT32 NumPathArrayElements = 0;
        UINT32 NumModeInfoArrayElements = 0;
        if ( GetDisplayConfigBufferSizeFunc(QDC_DATABASE_CURRENT, &NumPathArrayElements, &NumModeInfoArrayElements) != ERROR_SUCCESS ) {
            return res;
        }
        if ( !NumModeInfoArrayElements ) {
            return res;
        }
        DISPLAYCONFIG_TOPOLOGY_ID CurrentTopologyId = DISPLAYCONFIG_TOPOLOGY_INTERNAL;
        DISPLAYCONFIG_PATH_INFO* PathInfoArray = new DISPLAYCONFIG_PATH_INFO[NumPathArrayElements];
        memset(PathInfoArray, 0, sizeof(DISPLAYCONFIG_PATH_INFO) * NumPathArrayElements);
        DISPLAYCONFIG_MODE_INFO* ModeInfoArray = new DISPLAYCONFIG_PATH_INFO[NumModeInfoArrayElements];
        memset(ModeInfoArray, 0, sizeof(DISPLAYCONFIG_PATH_INFO) * NumModeInfoArrayElements);

        if ( QueryDisplayConfigFunc(QDC_DATABASE_CURRENT, &NumPathArrayElements, PathInfoArray, NumModeInfoArrayElements, ModeInfoArray, &CurrentTopologyId) != ERROR_SUCCESS ) {
            return res;
        }

        res = (LogicalMonitorWidth / LogicalDesktopWidth) / ModeInfoArray[0].sourceMode.width

    }*/
    return res;
}

CString GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return _T("No error message has been recorded");

    LPTSTR messageBuffer = 0;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&messageBuffer, 0, NULL);

    CString res = messageBuffer;

    //Free the buffer.
    LocalFree(messageBuffer);

    return res;
}


BOOL MakeDirectoryWritable(LPCTSTR lpPath) {
    HANDLE hDir = CreateFile(lpPath,READ_CONTROL|WRITE_DAC,0,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
    if(hDir == INVALID_HANDLE_VALUE)
        return FALSE; 

    ACL* pOldDACL;
    SECURITY_DESCRIPTOR* pSD = NULL;
    GetSecurityInfo(hDir, SE_FILE_OBJECT , DACL_SECURITY_INFORMATION,NULL, NULL, &pOldDACL, NULL, (void**)&pSD);

    PSID pSid = NULL;
    SID_IDENTIFIER_AUTHORITY authNt = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&authNt,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_USERS,0,0,0,0,0,0,&pSid);

    EXPLICIT_ACCESS ea={0};
    ea.grfAccessMode = GRANT_ACCESS;
    ea.grfAccessPermissions = GENERIC_ALL;
    ea.grfInheritance = CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE;
    ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.ptstrName = (LPTSTR)pSid;

    ACL* pNewDACL = 0;
    /*DWORD err = */SetEntriesInAcl(1,&ea,pOldDACL,&pNewDACL);

    if(pNewDACL)
        SetSecurityInfo(hDir,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL, NULL, pNewDACL, NULL);

    FreeSid(pSid);
    LocalFree(pNewDACL);
    LocalFree(pSD);
    LocalFree(pOldDACL);
    CloseHandle(hDir);
    return TRUE;
}


bool IsProcessRunning(DWORD pid) {
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!process) {
        return false;
    }

    CloseHandle(process);
    return true;
}

int GetInternetExplorerMajorVersion()
{
    CRegistry Reg;
    Reg.SetRootKey( HKEY_LOCAL_MACHINE );
    if ( Reg.SetKey( _T("Software\\Microsoft\\Internet Explorer") , FALSE ) ) {
        CString version = Reg.ReadString(_T("svcVersion"));
        if ( version.IsEmpty() ) {
            version = Reg.ReadString(_T("Version"));
        }
        int dotPos = version.Find(L'.');
        if ( dotPos != -1 ) {
            return StrToInt(version.Left(dotPos));
        }
    }
    return 7;
}

TCHAR* GetBrowserKey() {
    //32bit OS
    //\Software\Microsoft\Internet Explorer\Main\FeatureControl\FEATURE_BROWSER_EMULATION
    //32bit app on 64bit OS
    //\SOFTWARE\Wow6432Node\Microsoft\Internet Explorer\MAIN\FeatureControl\FEATURE_BROWSER_EMULATION

    /*if ( sizeof(void*) == 4 && IsWindows64Bit() ) {
        return _T("SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION");
    }*/
    return _T("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
}

void RemoveBrowserKey(){
    CRegistry Reg;
    Reg.SetRootKey( HKEY_CURRENT_USER );
    if ( Reg.SetKey( GetBrowserKey(), false ) ) {
        Reg.DeleteKey(myExtractFileName(GetAppFileName()));
    }

}

void UseLatestInternetExplorerVersion(bool IgnoreIDocDirective) {
    // Thank to https://www.daniweb.com/software-development/vbnet/code/442963/make-the-webbrowser-control-give-you-the-installed-ie-version-rendering

    int value = 0;
    int majorVersion = GetInternetExplorerMajorVersion();
    //Value reference: http://msdn.microsoft.com/en-us/library/ee330730%28v=VS.85%29.aspx
    //IDOC Reference:  http://msdn.microsoft.com/en-us/library/ms535242%28v=vs.85%29.aspx
    //majorVersion = 7;
    switch (majorVersion) {
        case 8:
            value = IgnoreIDocDirective ? 8888 : 8000;
            break;
        case 9:
            value = IgnoreIDocDirective ? 9999 : 9000;
            break;
        case 10:
            value = IgnoreIDocDirective ? 10001 : 10000;
            break;
        case 11:
            value = IgnoreIDocDirective ? 11001 :  11000;
            break;
        default:
            return;
        
    }

    CRegistry Reg;
    Reg.SetRootKey( HKEY_CURRENT_USER );
    if ( Reg.SetKey( GetBrowserKey(), true ) ) {
        Reg.WriteDword(myExtractFileName(GetAppFileName()), value);
    }    

}

void TimerWait(int Delay)
{
    HANDLE hTimer = CreateWaitableTimer(0, TRUE, 0);
    LARGE_INTEGER interval;
    interval.QuadPart = -Delay * 10000;
    SetWaitableTimer(hTimer, &interval, 0, 0, 0, 0);
    MsgWaitForSingleObject(hTimer, INFINITE);
    CloseHandle(hTimer);
}

CString ConvertRelativePathToAbsolute(const CString& fileName)
{
    TCHAR result[MAX_PATH+1];
    if (!GetFullPathName(fileName, MAX_PATH, result, 0))
    {
        return fileName;
    }
    return result;
}

std::wstring strtows(const std::string &str, UINT codePage)
{
    std::wstring ws;
    int n = MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/NULL, 0);
    if (n)
    {
        ws.resize(n - 1);
        if (MultiByteToWideChar(codePage, 0, str.c_str(), str.size() + 1, /*dst*/&ws[0], n) == 0)
            ws.clear();
    }
    return ws;
}
std::string wstostr(const std::wstring &ws, UINT codePage)
{
    std::string str;
    int n = WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size() + 1, /*dst*/NULL, 0, /*defchr*/0, NULL);
    if (n)
    {
        str.resize(n - 1);
        if (WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size() + 1, /*dst*/&str[0], n, /*defchr*/0, NULL) == 0)
            str.clear();
    }
    return str;
}

const std::string AnsiToUtf8(const std::string &str, int codepage)
{

    return wstostr(strtows(str, codepage), CP_UTF8);
}

const std::string Utf8ToAnsi(const std::string &str, int codepage)
{

    return wstostr(strtows(str, CP_UTF8), codepage);
}

CString GetProcessName(DWORD pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == pid) {
                    CloseHandle(hSnapshot);
                    return pe32.szExeFile;
                }
                
            }
            while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return CString();
}

std::string chcp(const std::string &str, UINT codePageSrc, UINT codePageDst)
{
    return wstostr(strtows(str, codePageSrc), codePageDst);
}

CString ErrorCodeToString(DWORD idCode)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        idCode, 0, reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL);
    CString res = reinterpret_cast<LPCTSTR>(lpMsgBuf);
    // Free the buffer.
    LocalFree(lpMsgBuf);
    return res;
}

};

const std::wstring Utf8ToWstring(const std::string &str)
{
    return WinUtils::strtows(str, CP_UTF8);
}