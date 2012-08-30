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

#include "myutils.h"

#include "atlheaders.h"
#include <shobjidl.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <algorithm>
#include <Core/Utils/CoreUtils.h>
#include <Core/Utils/StringUtils.h>
#include <Core/Video/VideoUtils.h>
#include <Func/WinUtils.h>

bool IsImage(LPCTSTR szFileName)
{
	LPCTSTR szExt = WinUtils::GetFileExt(szFileName);
	if(lstrlen(szExt)<1) return false;
	return WinUtils::IsStrInList(szExt,_T("jpg\0jpeg\0png\0bmp\0gif\0tif\0tiff\0\0"));
}

bool IsVideoFile(LPCTSTR szFileName)
{
	std::string ext = IuStringUtils::toLower( IuCoreUtils::ExtractFileExt(IuCoreUtils::WstringToUtf8(szFileName)) );
	std::vector<std::string>& extensions = VideoUtils::Instance().videoFilesExtensions;
	if(std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
		return true;
	} else {
	return false;
	}
}

LPTSTR fgetline(LPTSTR buf,int num,FILE *f)
{
	LPTSTR Result;
	LPTSTR cur;
	Result = _fgetts(buf,num,f);
	int n = lstrlen(buf)-1;

	for ( cur = buf + n; cur >= buf; cur-- ) {
		if(*cur==13||*cur==10||*cur==_T('\n'))
			*cur=0;
		else break;
	}
	return Result;
}

LPCTSTR  CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize)
{
	int nLen=0;
	if(!szString || !szPattern ||!szBuffer || nBufferSize<0) return FALSE;

	LPCTSTR szStart = (LPTSTR) _tcsstr(szString, szPattern);

	if(!szStart) 
	{
		nLen = lstrlen(szString);
		szStart = szString + nLen-1;
	}
	else nLen = szStart - szString;

	if(nLen > nBufferSize-1) nLen = nBufferSize-1;
	lstrcpyn(szBuffer, szString, nLen+1);
	szBuffer[nLen]=0;

	return szStart+1;
}

#if 1
void ShowX(LPCTSTR str,int line,int n)
{
	TCHAR buf[MAX_PATH];
	wsprintf(buf,_T("Str %d : %s = %d"),line,str,n);
	::MessageBox(0,buf,0,0);
}
void ShowX(LPCTSTR str,int line,float n)
{
	TCHAR buf[MAX_PATH];
	wsprintf(buf,_T("Str %d : %s = %f"),line,str,n);
	::MessageBox(0,buf,0,0);
}

void ShowX(LPCTSTR str,int line,LPCTSTR n)
{
	TCHAR buf[MAX_PATH];
	wsprintf(buf,_T("Str %d : %s = %s"),line,str,n);
	::MessageBox(0,buf,0,0);
}
#endif


bool CheckFileName(const CString& fileName) {
	return (fileName.FindOneOf(_T("\\/:*?\"<>|")) < 0);
}