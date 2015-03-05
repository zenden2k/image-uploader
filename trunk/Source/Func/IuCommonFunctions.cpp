#include "IuCommonFunctions.h"
#include "Settings.h"
#include "versioninfo.h"
#include "WinUtils.h"

namespace IuCommonFunctions {
	 CString IUTempFolder;

	 CString IUCommonTempFolder;

const CString GetDataFolder()
{
	CString result;
#if !defined(IU_CLI) && !defined(IU_SERVERLISTTOOL)
	result= Settings.DataFolder;
#else 
	result= WinUtils::GetAppFolder()+"\\Data\\";
#endif
	if ( result.Right(1) != "\\") {
		result += "\\";
	}
	return result;
}


const CString GetVersion()
{
	return CString(_APP_VER)+_T(".") + _T(BUILD);
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


};