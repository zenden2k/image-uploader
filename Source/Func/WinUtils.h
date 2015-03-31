#ifndef FUNC_WINUTILS_H
#define FUNC_WINUTILS_H

#include <windows.h>
#include "atlheaders.h"
#include <vector>

namespace Gdiplus {
	class Bitmap;
}
#define MYRGB(a,color) Color(a,GetRValue(color),GetGValue(color),GetBValue(color))

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
	bool    CopyTextToClipboard(CString text);
	bool    GetClipboardText(CString &text);
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

	CString GetAppFolder();
	CString GetAppFileName();
	CString FormatWindowsErrorMessage(int idCode);
	bool IsElevated();
	void DeleteDir2(LPCTSTR Dir);
	CString GetUniqFileName(const CString& filePath);
	int GetFolderFileList(std::vector<CString> &list, CString folder, CString mask);

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
};

#endif