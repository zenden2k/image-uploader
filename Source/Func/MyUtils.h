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

#ifndef _MYUTILS_H_
#define _MYUTILS_H_

#include <atlheaders.h>
#include <tchar.h>
#include <stdlib.h>
#include <string>

#ifndef IU_SHELLEXT
#define WCstringToUtf8(str) IuCoreUtils::wstostr(((LPCTSTR)(str)), CP_UTF8)
#define Utf8ToWCstring(str) CString(IuCoreUtils::Utf8ToWstring(str).c_str())
#endif

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}

#define LOADICO(ico) LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(ico))

LPTSTR fgetline(LPTSTR buf,int num,FILE *f);
LPCTSTR CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize);

void ShowX(LPCTSTR str,int line,int n);
void ShowX(LPCTSTR str,int line,float n);
void ShowX(LPCTSTR str,int line,LPCTSTR n);
#define ShowVar(n) ShowX(_T(#n),__LINE__,n)

bool CheckFileName(const CString& fileName);

#ifndef IU_SHELLEXT


#define PROP_OBJECT_PTR			MAKEINTATOM(ga.atom)
#define PROP_ORIGINAL_PROC		MAKEINTATOM(ga.atom)

/*
 * typedefs
 */
class CGlobalAtom
{
public:
	CGlobalAtom(void)
	{ atom = GlobalAddAtom(TEXT("_Hyperlink_Object_Pointer_")
	         TEXT("\\{AFEED740-CC6D-47c5-831D-9848FD916EEF}")); }
	~CGlobalAtom(void)
	{ DeleteAtom(atom); }

	ATOM atom;
};

/*
 * Local variables
 */
static CGlobalAtom ga;
#endif

#endif