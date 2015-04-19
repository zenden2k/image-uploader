/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#ifndef _IU_UTILS_WIN_H
#define _IU_UTILS_WIN_H

#include <windows.h>
#include "Core/3rdpart/codepages.h"
#include "Core/Logging.h"

namespace IuCoreUtils
{

typedef HRESULT  (STDAPICALLTYPE  *FindMimeFromDataFunc)(LPBC, LPCWSTR, LPVOID, DWORD, LPCWSTR, DWORD, LPWSTR*, DWORD);

std::wstring strtows(const std::string &str, UINT codePage)
{
	 std::wstring ws;
	 int n = MultiByteToWideChar(codePage, 0, str.c_str(), str.size()+1, /*dst*/NULL, 0);
	 if(n)
	 {
          ws.reserve(n);
		  ws.resize(n-1);
		  if(MultiByteToWideChar(codePage, 0, str.c_str(), str.size()+1, /*dst*/&ws[0], n) == 0)
				ws.clear();
	 }
	 return ws;
}

std::string wstostr(const std::wstring &ws, UINT codePage)
{
    // prior to C++11 std::string and std::wstring were not guaranteed to have their memory be contiguous,
    // although all real-world implementations make them contiguous
	 std::string str;
     int srcLen = ws.size();
	 int n = WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen+1, NULL, 0, /*defchr*/0, NULL);
	 if(n)
	 {
		  str.reserve(n);
          str.resize(n-1);
		  if(WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen+1, &str[0], n, /*defchr*/0, NULL) == 0)
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
	FreeLibrary(urlMonDll);
	//delete szMimeW;
	return result;
}

bool DirectoryExists(const Utf8String path)
{

	DWORD dwFileAttributes = GetFileAttributes(Utf8ToWstring(path).c_str());
	if(dwFileAttributes != INVALID_FILE_ATTRIBUTES && (dwFileAttributes &
		FILE_ATTRIBUTE_DIRECTORY)) {
		return true;
	}
	return false;
}

bool createDirectory(const Utf8String& path_,unsigned int mode)
{
	 Utf8String path = path_;
	if(path.empty()) return false;
	if ( path[path.length()-1] == '\\' ||  path[path.length()-1] == '/') {
		path.resize(path.length()-1);
	}
	std::wstring wstrFolder = Utf8ToWstring(path);
	//MessageBox(0,wstrFolder.c_str(),0,0);

	DWORD dwAttrib = GetFileAttributes(wstrFolder.c_str()); 

	// already exists ?
	if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

	// recursively create from the top down
	char* szPath = _strdup(path.c_str());
	char * p = 0;
	for(int i=path.length()-1; i>=0; i--)
	{
		if(szPath[i] == '\\' || szPath[i] == '/')
		{
			p = szPath + i;
			break;
		}
	}
	if (p) 
	{
		// The parent is a dir, not a drive
		*p = '\0';

		// if can't create parent
		if (!createDirectory(szPath))
		{
			free(szPath);
			return false;
		}
		free(szPath);

		if (!::CreateDirectory(wstrFolder.c_str(), NULL)) {
			//LOG(WARNING) << wstrFolder;
			return false;
		}
	}

	return TRUE;
}


bool copyFile(const std::string& src, const std::string & dest, bool overwrite)
{
	return ::CopyFile(Utf8ToWstring(src).c_str(), Utf8ToWstring(dest).c_str(), !overwrite)!=FALSE;
}

bool RemoveFile(const Utf8String& utf8Filename) {
	return DeleteFile(IuCoreUtils::Utf8ToWstring(utf8Filename).c_str())!=FALSE;
}

bool MoveFileOrFolder(const Utf8String& from ,const Utf8String& to) {
	return MoveFile(IuCoreUtils::Utf8ToWstring(from).c_str() ,IuCoreUtils::Utf8ToWstring(to).c_str());
}

}



#endif
