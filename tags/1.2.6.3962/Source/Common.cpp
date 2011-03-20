/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#include "Common.h"
#include "Common/CmdLine.h"
//#include "wizarddlg.h"
#include "versioninfo.h"
#include "settings.h"
#pragma comment(lib,"urlmon.lib")
#include <openssl/md5.h>
#include "MyUtils.h"
CString IUCommonTempFolder, IUTempFolder;


CString IU_md5_file(const CString& filename)
{
	CString result;
	MD5_CTX context;

	MD5_Init(&context);
	FILE *f = _wfopen(filename,_T("rb"));

	if(f)
	{
		unsigned char buf[4096];
		while(!feof(f))
		{	
			size_t bytesRead = fread(buf, 1, sizeof(buf), f);


			MD5_Update(&context, (unsigned char*)buf, bytesRead);
		}
		unsigned char buff[16] = "";    

		MD5_Final(buff, &context);

		fclose(f);

		for(int i=0; i<16; i++)
		{
			TCHAR temp[5];
			swprintf(temp, _T("%02x"),buff[i]);
			result += temp;
		}
	}
	return result;
}


WIN32_FIND_DATA wfd;
HANDLE findfile = 0;

int GetNextImgFile(LPTSTR szBuffer, int nLength)
{
	TCHAR szBuffer2[MAX_PATH], TempPath[256];
	
	GetTempPath(256, TempPath);
	wsprintf(szBuffer2, _T("%s*.*"), (LPCTSTR)IUTempFolder);
	
	if(!findfile)
	{
		findfile = FindFirstFile(szBuffer2, &wfd);
		if(!findfile) goto error;
	}
	else 
	{
		if(!FindNextFile(findfile, &wfd))
			goto error;

	}
	if(lstrlen(wfd.cFileName) < 1) goto error;
	lstrcpyn(szBuffer, wfd.cFileName, nLength);

	return TRUE;

error:
	if(findfile) FindClose(findfile);
	return FALSE;
}

void DeleteDir2(LPCTSTR Dir)
{
	if(!Dir) return;
	TCHAR szBuffer[MAX_PATH];
	lstrcpyn(szBuffer, Dir, MAX_PATH);
	int nLen = lstrlen(szBuffer)-1;
	if(szBuffer[nLen] == _T('\\')) szBuffer[nLen] = 0;

	SHFILEOPSTRUCT FileOp;
	ZeroMemory(&FileOp, sizeof(FileOp));
	FileOp.hwnd = NULL;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = szBuffer;
	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT|FOF_NOERRORUI;
	SHFileOperation(&FileOp);
}

void ClearTempFolder()
{
	TCHAR szBuffer[256] = _T("\0");
	TCHAR szBuffer2[MAX_PATH], TempPath[256];
	GetTempPath(256, TempPath);
	
	while(GetNextImgFile(szBuffer, 256))
	{
		#ifdef DEBUG
			if(!lstrcmpi(szBuffer, _T("log.txt"))) continue;
		#endif
		wsprintf(szBuffer2,_T("%s%s"), (LPCTSTR) IUTempFolder, (LPCTSTR)szBuffer);
		DeleteFile(szBuffer2);
	}
	if(!RemoveDirectory(IUTempFolder))
	{
		DeleteDir2(IUTempFolder);
	}
}

int GetFolderFileList(std::vector<CString> &list, CString folder, CString mask)
{
	WIN32_FIND_DATA wfd;
	ZeroMemory(&wfd, sizeof(wfd));
	HANDLE findfile = 0;

	TCHAR szNameBuffer[MAX_PATH];
	
	//GetTempPath(256, TempPath);
	
	
	for(;;)
	{
		if(!findfile)
		{
			findfile = FindFirstFile(folder+_T("\\")+mask, &wfd);
			if(!findfile) break;;
		}
		else 
		{
			if(!FindNextFile(findfile, &wfd))
				break;

		}
		if(lstrlen(wfd.cFileName) < 1) break;
		lstrcpyn(szNameBuffer, wfd.cFileName, 254);
		list.push_back(szNameBuffer);
	}
	//return TRUE;

//error:
	if(findfile) FindClose(findfile);
	return list.size();
	//return FALSE;
}

bool IULaunchCopy(CString additionalParams)
{
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	
	ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);				 
   ZeroMemory(&pi, sizeof(pi));

	TCHAR Buffer[MAX_PATH*40];
	GetModuleFileName(0, Buffer, sizeof(Buffer)/sizeof(TCHAR));

	CString TempCmdLine = CString(_T("\""))+CmdLine[0]+CString(_T("\"")); 
	for(size_t i=1;i <CmdLine.GetCount(); i++)
		{
			if(!lstrcmpi(CmdLine[i], _T("-Embedding"))) continue;
			TempCmdLine = TempCmdLine + " \"" + CmdLine[i] + "\""; 
		}

	TempCmdLine += _T(" ")+additionalParams;
    // Start the child process.
    if( !CreateProcess(
		NULL,                   // No module name (use command line). 
        (LPWSTR)(LPCTSTR)TempCmdLine, // Command line. 
        NULL,                   // Process handle not inheritable. 
        NULL,                   // Thread handle not inheritable. 
        FALSE,                  // Set handle inheritance to FALSE. 
        0,                      // No creation flags. 
        NULL,                   // Use parent's environment block. 
        NULL,                   // Use parent's starting directory. 
        &si,                    // Pointer to STARTUPINFO structure.
        &pi )                   // Pointer to PROCESS_INFORMATION structure.
    ) 
    
       return false;
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	return true;
}


BOOL CreateTempFolder()
{
	TCHAR TempPath[256];
	GetTempPath(256, TempPath);
	DWORD pid = GetCurrentProcessId() ^ 0xa1234568;
	IUCommonTempFolder.Format(_T("%stmd_iu_temp"), (LPCTSTR)TempPath);
	
	CreateDirectory(IUCommonTempFolder,0);
	IUTempFolder.Format(_T("%s\\iu_temp_%x"),(LPCTSTR) IUCommonTempFolder, pid);
	
	CreateDirectory(IUTempFolder,0);

	IUTempFolder+=_T("\\");
	return TRUE;
}


#define HOTKEY(modifier,key) ((((modifier)&0xff)<<8)|((key)&0xff)) 

// �������� ������ 
// ������� ���������: 
//  pwzShortCutFileName - ���� � ��� ������, ��������, "C:\\�������.lnk" 
//  ���� �� ������ ����, ����� ����� ������ � �����, ��������� � ��������� ���������. 
//  ����.: Windows ���� �� ��������� � ����� ���������� .lnk 
//  pszPathAndFileName  - ���� � ��� exe-�����, ��������, "C:\\Windows\\NotePad.Exe" 
//  pszWorkingDirectory - ������� �������, ��������, "C:\\Windows" 
//  pszArguments        - ��������� ��������� ������, ��������, "C:\\Doc\\Text.Txt" 
//  wHotKey             - ������� �������, ��������, ��� Ctrl+Alt+A     HOTKEY(HOTKEYF_ALT|HOTKEYF_CONTROL,'A') 
//  iCmdShow            - ��������� ���, ��������, SW_SHOWNORMAL 
//  pszIconFileName     - ���� � ��� �����, ����������� ������, ��������, "C:\\Windows\\NotePad.Exe" 
//  int iIconIndex      - ������ ������ � �����, ���������� � 0 
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
					   IShellLink * pSL; 
	IPersistFile * pPF; 
	HRESULT hRes; 
	if( CoInitialize(NULL) != S_OK);
		//return false;
	// ��������� ���������� ���������� "�����" 
	hRes = CoCreateInstance(CLSID_ShellLink, 0,	CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pSL); 

	if( SUCCEEDED(hRes) ) 
	{ 
		hRes = pSL->SetPath(pszPathAndFileName); 
		if( SUCCEEDED(hRes) ) 
		{ 
			hRes = pSL->SetArguments(pszArguments); 
			//if( SUCCEEDED(hRes) ) 
			{ 
				hRes = pSL->SetWorkingDirectory(pszWorkingDirectory); 
				if( SUCCEEDED(hRes) ) 
				{ 
					hRes = pSL->SetIconLocation(pszIconFileName,iIconIndex); 
					if( SUCCEEDED(hRes) ) 
					{ 
					//	hRes = pSL->SetHotkey(wHotKey); 
					//	if( SUCCEEDED(hRes) ) 
						{ 
							hRes = pSL->SetShowCmd(iCmdShow); 
							if( SUCCEEDED(hRes) ) 
							{ 
								// ��������� ���������� ��������� ���������� 
								hRes = pSL->QueryInterface(IID_IPersistFile,(LPVOID *)&pPF); 
								if( SUCCEEDED(hRes) ) 
								{ 
									// ���������� ���������� ������ 
									hRes = pPF->Save(pwzShortCutFileName,TRUE); 
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





bool IULaunchCopy(CString params, CAtlArray<CString> &files)
{
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
        
	ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);                           
   ZeroMemory(&pi, sizeof(pi));


	TCHAR Buffer[MAX_PATH*40];
	GetModuleFileName(0, Buffer, sizeof(Buffer)/sizeof(TCHAR));


	CString TempCmdLine = CString(_T("\"")) + Buffer + CString(_T("\""));
	if(!params.IsEmpty())
		TempCmdLine += _T(" ") + params + _T(" ");

	for(size_t i=0;i <files.GetCount(); i++)
	{
		TempCmdLine = TempCmdLine + " \"" + files[i] + "\""; 
	}

    // Start the child process.
    if( !CreateProcess(
                NULL,                   // No module name (use command line). 
        (LPWSTR)(LPCTSTR)TempCmdLine, // Command line. 
        NULL,                   // Process handle not inheritable. 
        NULL,                   // Thread handle not inheritable. 
        FALSE,                  // Set handle inheritance to FALSE. 
        0,                      // No creation flags. 
        NULL,                   // Use parent's environment block. 
        NULL,                   // Use parent's starting directory. 
        &si,                    // Pointer to STARTUPINFO structure.
        &pi )                   // Pointer to PROCESS_INFORMATION structure.
    ) 
    
        return false;

    // Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	return true;
}

void IU_ConfigureProxy(NetworkManager& nm)
{
	if(Settings.ConnectionSettings.UseProxy)
	{
		int ProxyTypeList [5] = { CURLPROXY_HTTP, 
		CURLPROXY_SOCKS4,CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME};
		nm.setProxy(WstrToUtf8((LPCTSTR)Settings.ConnectionSettings.ServerAddress), Settings.ConnectionSettings.ProxyPort,ProxyTypeList[Settings.ConnectionSettings.ProxyType]);
		nm.setProxyUserPassword(WstrToUtf8((LPCTSTR)Settings.ConnectionSettings.ProxyUser), WstrToUtf8((LPCTSTR)Settings.ConnectionSettings.ProxyPassword));	
	}
	nm.setUploadBufferSize(Settings.UploadBufferSize);
}

CString IU_GetFileMimeType (const CString& filename)
{
	FILE * InputFile = _tfopen(filename,_T("rb"));
	if(!InputFile) 
		return _T("");

	BYTE		byBuff[256] ;
	int nRead = fread(byBuff, 1, 256, InputFile);
	 
	fclose(InputFile);

	PWSTR		szMimeW = NULL ;
	HRESULT		hResult ;

	if ( NOERROR != ::FindMimeFromData(NULL, NULL, byBuff, nRead, NULL, 0, &szMimeW, 0) ) 
	{
		return _T("application/octet-stream"); 
	}

	if(!lstrcmpW(szMimeW,_T("image/x-png"))) lstrcpyW(szMimeW, _T("image/png"));

	return szMimeW ;
}
CPluginManager iuPluginManager;

const CString IU_GetVersion()
{
	return CString("1.2.6.") + _T(BUILD);
}

void IU_RunElevated(CString params)
{
	SHELLEXECUTEINFO TempInfo = {0};
	CString appDir = GetAppFolder();
	CString Command = CmdLine[0];
	CString parameters = _T(" ") + params;
	TempInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	TempInfo.fMask = 0;
	TempInfo.hwnd = NULL;
	if(IsVista())
	TempInfo.lpVerb = _T("runas");
	else
		TempInfo.lpVerb = _T("open");
	TempInfo.lpFile = Command;
	TempInfo.lpParameters = parameters;
	TempInfo.lpDirectory = appDir;
	TempInfo.nShow = SW_NORMAL;

	::ShellExecuteEx(&TempInfo);
}

bool IU_GetClipboardText(CString &text)
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

bool IU_CopyTextToClipboard(CString text)
{
    LPTSTR  lptstrCopy;
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
	while((MsgWaitForMultipleObjects(1, &pHandle, FALSE, INFINITE, QS_SENDMESSAGE)) != WAIT_OBJECT_0)
	{
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 1;
}

int dlgX(int WidthInPixels)
{
	LONG units = GetDialogBaseUnits();
	short baseunitX = LOWORD(units);
	return WidthInPixels*baseunitX/4;
}

int dlgY(int HeightInPixels)
{
	LONG units = GetDialogBaseUnits();
	short baseunitY = HIWORD(units);
	return HeightInPixels*baseunitY/8;
}

CString GetUniqFileName(const CString &filePath)
{
	TCHAR path[256];
	if(!FileExists(filePath)) return filePath;
	ExtractFilePath(filePath, path);
	CString name;
	name = GetOnlyFileName(filePath);
	CString extension = GetFileExt(filePath);
	CString result;
	for(int i=2;;i++)
	{
		result = path + name + IntToStr(i)+ (extension.IsEmpty()?_T(""):_T(".")+extension);
		if(!FileExists(result)) break;
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

const std::string IU_md5(const std::string& data);
CString GenerateFileName(const CString &templateStr, int index, const CPoint size, const CString& originalName)
{
	CString result = templateStr;
	time_t t = time(0);
	tm * timeinfo = localtime ( &t );
	CString indexStr;
	CString day, month, year;
	CString hours,seconds,minutes;
	indexStr.Format(_T("%03d"),index);
	CString md5 = Utf8ToWstring(IU_md5(WCstringToUtf8(IntToStr(GetTickCount()+random(100))))).c_str();
	result.Replace(_T("%md5"), (LPCTSTR)md5);
	result.Replace(_T("%width%"), IntToStr(size.x));
	result.Replace(_T("%height%"), IntToStr(size.y));
	year.Format(_T("%04d"), (int)1900+timeinfo->tm_year);
	month.Format(_T("%02d"), (int) timeinfo->tm_mon+1);
	day.Format(_T("%02d"), (int) timeinfo->tm_mday);
	hours.Format(_T("%02d"), (int)timeinfo->tm_hour);
	seconds.Format(_T("%02d"), (int)timeinfo->tm_sec);
	minutes.Format(_T("%02d"), (int)timeinfo->tm_min);
	result.Replace(_T("%y"),year);
	result.Replace(_T("%m"), month);
	result.Replace(_T("%d"), day);
	result.Replace(_T("%h"), hours);
	result.Replace(_T("%n"), minutes);
	result.Replace(_T("%s"), seconds);
	result.Replace(_T("%i"), indexStr);
	return result;
}

CMyEngineList *_EngineList;

const CString IU_GetWindowText(HWND wnd)
{
	int len = GetWindowTextLength(wnd);
	CString buf;
	GetWindowText(wnd, buf.GetBuffer(len+1),len+1);
	buf.ReleaseBuffer(-1);
	return buf;
}

std::string ExtractFileNameA(const std::string& FileName)
{  
	return WCstringToUtf8(myExtractFileName(Utf8ToWstring(FileName).c_str()));
}
LPCSTR GetFileExtA(LPCSTR szFileName)
{
	if(!szFileName) return 0;
	int nLen = lstrlenA(szFileName);

	LPCSTR szReturn = szFileName+nLen;
	for( int i=nLen-1; i>=0; i-- )
	{
		if(szFileName[i] == '.')
		{
			szReturn = szFileName + i + 1;
			break;
		}
		else if(szFileName[i] == '\\') break;
	}
	return szReturn;
}

void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ")
{
	TCHAR szDestination[1024];
	int br = strlen(code);
	int n = lstrlen(szSource) / 2;
	int j = 0;
	ZeroMemory(szDestination, n*2);

	int i;
	PBYTE data = (PBYTE)szDestination;
	*szDestination=0;

	for(i=0; i<n; i++)
	{
		if(j >= br) j=0;

		BYTE b;
		b = (szSource[i*2] - _T('A'))*16 + (szSource[i*2+1] - _T('A'));
		b = b^code[j];
		data[i] = b;
		j++;
	}
	data[i]=0;
	Result = szDestination;
}

void EncodeString(LPCTSTR szSource, CString &Result,LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ")
{
	TCHAR szDestination[1024];
	int br = strlen(code);
	int n = lstrlen(szSource) * 2;
	int j = 0;

	PBYTE data = (PBYTE)szSource;
	*szDestination = 0;
	for(int i=0; i<n; i++)
	{
		if(j>=br)j=0;

		BYTE b;
		b = data[i]^code[j];
		TCHAR bb[2]={0,0};
		bb[0]=_T('A')+b/16;
		lstrcat(szDestination,bb);
		bb[0]=_T('A')+b%16;
		lstrcat(szDestination,bb);
		j++;

	}
	Result = szDestination;
}

BOOL IU_CreateFilePath(LPCTSTR szFilePath)
{
	TCHAR* szPath = _tcsdup(szFilePath);
	TCHAR* p = _tcsrchr(szPath,'\\');

	BOOL bRes = FALSE;

	if (p)
	{
		*p = '\0';

		bRes = IU_CreateFolder(szPath);
	}

	free(szPath);

	return bRes;
}