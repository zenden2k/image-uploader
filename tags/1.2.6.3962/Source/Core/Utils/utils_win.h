#ifndef _IU_UTILS_WIN_H
#define _IU_UTILS_WIN_H

#include <windows.h>
#include "codepages.h"

namespace IuCoreUtils
{

typedef HRESULT  (STDAPICALLTYPE  *FindMimeFromDataFunc)(LPBC, LPCWSTR, LPVOID, DWORD, LPCWSTR, DWORD, LPWSTR*, DWORD);

std::wstring strtows(const std::string &str, UINT codePage)
{
	 std::wstring ws;
	 int n = MultiByteToWideChar(codePage, 0, str.c_str(), str.size()+1, /*dst*/NULL, 0);
	 if(n)
	 {
		  ws.resize(n-1);
		  if(MultiByteToWideChar(codePage, 0, str.c_str(), str.size()+1, /*dst*/&ws[0], n) == 0)
				ws.clear();
	 }
	 return ws;
}

std::string wstostr(const std::wstring &ws, UINT codePage)
{
	 std::string str;
	 int n = WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size()+1, NULL, 0, /*defchr*/0, NULL);
	 if(n)
	 {
		  str.resize(n-1);
		  if(WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size()+1, &str[0], n, /*defchr*/0, NULL) == 0)
				str.clear();
	 }
	 return str;
}

const std::string AnsiToUtf8(const std::string &str, int codepage)
{

	return wstostr(strtows(str, codepage), CP_UTF8);
}

const Utf8String WstringToUtf8(const std::wstring &str)
{
	return wstostr(str, CP_UTF8);
}

const std::wstring Utf8ToWstring(const Utf8String &str)
{

	return strtows(str, CP_UTF8);
}

const std::string Utf8ToAnsi(const std::string &str, int codepage)
{

	return wstostr(strtows(str, CP_UTF8), codepage);
}


Utf8String ConvertToUtf8(const Utf8String &text, const Utf8String codePage)
{
	unsigned int codePageNum = CodepageByName(codePage.c_str());
	if(codePageNum != CP_UTF8)
	return AnsiToUtf8(text, codePageNum);
	return text;
}

Utf8String GetFileMimeType(const Utf8String fileName)
{
	const Utf8String DefaultMimeType = "application/octet-stream";
	FILE * InputFile = fopen_utf8(fileName.c_str(), "rb");
	if (!InputFile) return "";

	BYTE byBuff[256] ;
	int nRead = fread(byBuff, 1, 256, InputFile);

	fclose(InputFile);

	PWSTR		szMimeW = NULL ;

	HMODULE urlMonDll = LoadLibraryA("urlmon.dll");
	if(!urlMonDll)
		return DefaultMimeType;
	FindMimeFromDataFunc _Win32_FindMimeFromData = (FindMimeFromDataFunc)
																  GetProcAddress(urlMonDll,"FindMimeFromData");

	if(!_Win32_FindMimeFromData)
		return DefaultMimeType;

	if ( NOERROR != _Win32_FindMimeFromData(NULL, NULL, byBuff, nRead, NULL, 0, &szMimeW, 0) )
	{
		return DefaultMimeType;
	}

	Utf8String result = WstringToUtf8(szMimeW);
	if(result == "image/x-png")
		result = "image/png";
	else if(result == "image/pjpeg")
		result = "image/jpeg";

	//delete szMimeW;
	return result;
}

}

#endif
