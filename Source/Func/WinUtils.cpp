#include "WinUtils.h"
#include <Core/Utils/CoreUtils.h>
#include <sstream>
#include <Core/Utils/StringUtils.h>
#include <Func/MyUtils.h>

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

bool GetClipboardText(CString& text)
{
	if (OpenClipboard(NULL))
	{
		HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
		LPCWSTR lpstr = (LPCWSTR)GlobalLock(hglb);
		text = lpstr;
		GlobalUnlock(hglb);
		CloseClipboard();
		return true;
	}
	return false;
}

bool CopyTextToClipboard(CString text)
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
	lptstrCopy = (LPTSTR) GlobalLock(hglbCopy);
	memcpy(lptstrCopy, (LPCTSTR)text, text.GetLength() * sizeof(TCHAR));
	lptstrCopy[cch] = (TCHAR) 0;
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
						//	hRes = pSL->SetHotkey(wHotKey);
						//	if( SUCCEEDED(hRes) )
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

bool IsDirectory(LPCTSTR szFileName)
{
	DWORD res = GetFileAttributes(szFileName);
	return (res&FILE_ATTRIBUTE_DIRECTORY) && (res != -1);	
}

bool IsVista() {
	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

	if ( ::GetVersionEx( &osver ) && 
		osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
		(osver.dwMajorVersion >= 6 ) )
		return TRUE;

	return FALSE;
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

bool IsWindows64Bit()
{
	SYSTEM_INFO si;
	PGNSI pGNSI;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), 
		"GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&si);
	else GetSystemInfo(&si);
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
		idCode, 0, (LPTSTR) &lpMsgBuf, 0, NULL);
	CString res = (LPCTSTR)lpMsgBuf;
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return res;
}


bool FileExists(LPCTSTR FileName)
{
	if(!FileName || GetFileAttributes(FileName)==-1) return FALSE;
	return TRUE;
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
#if !defined(IU_SHELLEXT) && !defined(IU_CLI)
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

	return pbElevated;
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


int GetFolderFileList(std::vector<CString>& list, CString folder, CString mask)
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
	lFont->lfCharSet=nCharSet;

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
		LPCSTR lpstr = (LPCSTR)GlobalLock(hglb);
		std::string ansiString = (LPCSTR)lpstr;

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


};