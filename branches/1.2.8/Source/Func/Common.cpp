/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>

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

#include <openssl/md5.h>

#include "atlheaders.h"
#include "Common/CmdLine.h"
#include "versioninfo.h"
#include "Func/settings.h"
#include "Func/MyUtils.h"
#include "Func/Settings.h"
#include "Core/Utils/CryptoUtils.h"
#include <Func/WinUtils.h>

CString IUCommonTempFolder, IUTempFolder;

WIN32_FIND_DATA wfd;
HANDLE findfile = 0;

int GetNextImgFile(LPCTSTR folder, LPTSTR szBuffer, int nLength)
{
	TCHAR szBuffer2[MAX_PATH], TempPath[256];

	GetTempPath(256, TempPath);
	wsprintf(szBuffer2, _T("%s*.*"), (LPCTSTR)folder);

	if (!findfile)
	{
		findfile = FindFirstFile(szBuffer2, &wfd);
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
	lstrcpyn(szBuffer, wfd.cFileName, nLength);

	return TRUE;

error:
	if (findfile)
		FindClose(findfile);
	return FALSE;
}


void ClearTempFolder(LPCTSTR folder)
{
	TCHAR szBuffer[256] = _T("\0");
	TCHAR szBuffer2[MAX_PATH], TempPath[256];
	GetTempPath(256, TempPath);
	findfile = 0;
	while (GetNextImgFile(folder, szBuffer, 256))
	{
#ifdef DEBUG
		if (!lstrcmpi(szBuffer, _T("log.txt")))
			continue;
#endif
		wsprintf(szBuffer2, _T("%s%s"), (LPCTSTR) folder, (LPCTSTR)szBuffer);
		DeleteFile(szBuffer2);
	}
	if (!RemoveDirectory(folder))
	{
		WinUtils::DeleteDir2(folder);
	}
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

BOOL CreateTempFolder()
{
	TCHAR TempPath[256];
	GetTempPath(256, TempPath);
	DWORD pid = GetCurrentProcessId() ^ 0xa1234568;
	IUCommonTempFolder.Format(_T("%stmd_iu_temp"), (LPCTSTR)TempPath);

	CreateDirectory(IUCommonTempFolder, 0);
	IUTempFolder.Format(_T("%s\\iu_temp_%x"), (LPCTSTR) IUCommonTempFolder, pid);

	CreateDirectory(IUTempFolder, 0);

	IUTempFolder += _T("\\");
	return TRUE;
}

bool IULaunchCopy(CString params, CAtlArray<CString>& files)
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

void IU_ConfigureProxy(NetworkManager& nm)
{
	if (Settings.ConnectionSettings.UseProxy)
	{
		int ProxyTypeList [5] = { CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, 
											CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME };
		if(Settings.ConnectionSettings.NeedsAuth)
		{
			nm.setProxy(WCstringToUtf8(
				(LPCTSTR)Settings.ConnectionSettings.ServerAddress), Settings.ConnectionSettings.ProxyPort,
				ProxyTypeList[Settings.ConnectionSettings.ProxyType]);
			nm.setProxyUserPassword(WCstringToUtf8((LPCTSTR)Settings.ConnectionSettings.ProxyUser),
				WCstringToUtf8((LPCTSTR)Settings.ConnectionSettings.ProxyPassword));
		}
	}
	nm.setUploadBufferSize(Settings.UploadBufferSize);
}

CPluginManager iuPluginManager;

const CString IU_GetVersion()
{
	return CString("1.2.8.") + _T(BUILD);
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



CString GenerateFileName(const CString& templateStr, int index, const CPoint size, const CString& originalName)
{
	CString result = templateStr;
	time_t t = time(0);
	tm* timeinfo = localtime ( &t );
	CString indexStr;
	CString day, month, year;
	CString hours, seconds, minutes;
	indexStr.Format(_T("%03d"), index);
	CString md5 = IuCoreUtils::Utf8ToWstring(IuCoreUtils::CryptoUtils::CalcMD5HashFromString(WCstringToUtf8(WinUtils::IntToStr(GetTickCount() + IuCoreUtils::random(100))))).c_str();
	result.Replace(_T("%md5"), (LPCTSTR)md5);
	result.Replace(_T("%width%"), WinUtils::IntToStr(size.x));
	result.Replace(_T("%height%"), WinUtils::IntToStr(size.y));
	year.Format(_T("%04d"), (int)1900 + timeinfo->tm_year);
	month.Format(_T("%02d"), (int) timeinfo->tm_mon + 1);
	day.Format(_T("%02d"), (int) timeinfo->tm_mday);
	hours.Format(_T("%02d"), (int)timeinfo->tm_hour);
	seconds.Format(_T("%02d"), (int)timeinfo->tm_sec);
	minutes.Format(_T("%02d"), (int)timeinfo->tm_min);
	result.Replace(_T("%y"), year);
	result.Replace(_T("%m"), month);
	result.Replace(_T("%d"), day);
	result.Replace(_T("%h"), hours);
	result.Replace(_T("%n"), minutes);
	result.Replace(_T("%s"), seconds);
	result.Replace(_T("%i"), indexStr);
	return result;
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
		b = (szSource[i * 2] - _T('A')) * 16 + (szSource[i * 2 + 1] - _T('A'));
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


const CString IU_GetDataFolder()
{
	return Settings.DataFolder;
}

Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
	using namespace Gdiplus;
	HRSRC hrsrc = FindResource(hInstance, szResName, szResType);
	if (!hrsrc)
		return 0;
	// "Fake" HGLOBAL - look at MSDN
	HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
	DWORD sz = SizeofResource(hInstance, hrsrc);
	void* ptr1 = LockResource(hg1);
	HGLOBAL hg2 = GlobalAlloc(GMEM_FIXED, sz);

	// Copy raster data
	CopyMemory(LPVOID(hg2), ptr1, sz);
	IStream* pStream;

	// TRUE means free memory at Release
	HRESULT hr = CreateStreamOnHGlobal(hg2, TRUE, &pStream);
	if (FAILED(hr))
		return 0;

	// use load from IStream
	Gdiplus::Bitmap* image = Bitmap::FromStream(pStream);
	pStream->Release();
	// GlobalFree(hg2);
	return image;
}
