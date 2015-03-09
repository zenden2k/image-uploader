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
#include <Core/Utils/CoreUtils.h>
#include <Core/Utils/StringUtils.h>
#include <algorithm>
//#include <Func/Settings.h>
#include <Core/Video/VideoUtils.h>

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

#define STRING MAX_PATH


int GetFontSize(int nFontHeight)
{
	return - MulDiv( nFontHeight, 72, GetDeviceCaps(::GetDC(0), LOGPIXELSY));
}

int GetFontHeight(int nFontSize)
{
	return - MulDiv(nFontSize, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);
}

int GetFontSizeInTwips(int nFontSize)
{
   return MulDiv(nFontSize, 1440, 72);
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

const CString StringSection(const CString& str,TCHAR sep, int index)
{
	CString result;
	ExtractStrFromList(str, index, result.GetBuffer(256),256,_T(""),sep);
	result.ReleaseBuffer();
	return result;
}

bool FontToString(const LOGFONT * lFont, CString &Result)
{
	TCHAR  szBuffer[1024];
	if( !lFont ) return false;

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

	TCHAR szFontSize[STRING];
	TCHAR szFormat[STRING];
	TCHAR szCharset[STRING];
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
const CString myExtractFileName(const CString & FileName)
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

const CString GetOnlyFileName(const CString& szFilename)
{
	CString tempName = myExtractFileName(szFilename);
	int dotPos = tempName.ReverseFind(_T('.'));
	if(dotPos != -1)
		tempName=tempName.Left(dotPos);
	
	return tempName;
}

bool IsImage(LPCTSTR szFileName)
{
	LPCTSTR szExt = GetFileExt(szFileName);
	if(lstrlen(szExt)<1) return false;
	return IsStrInList(szExt,_T("jpg\0jpeg\0png\0bmp\0gif\0tif\0tiff\0\0"));
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



/*bool ReadSetting(LPTSTR szSettingName,int* Value,int DefaultValue,LPTSTR szString,LPTSTR szDefString)
{
	TCHAR szFileName[256],szPath[256];
	GetModuleFileName(0,szFileName,1023);
   ExtractFilePath(szFileName,szPath);
   wsprintf(szFileName,_T("%simgupload.ini"),szPath);



	TCHAR szBuffer1[128],szBuffer2[128];
	lstrcpy(szBuffer2,GetFileExt(szSettingName));
	GetOnlyFileName(szSettingName,szBuffer1);
	if(!szString)
	*Value = GetPrivateProfileInt(szBuffer1,szBuffer2, DefaultValue, szFileName);
	else
	 GetPrivateProfileString(szBuffer1,szBuffer2, szDefString,szString,256, szFileName);
	
	return true;
}

bool WriteSetting(LPCTSTR szSettingName,int Value,LPCTSTR szString)
{
	TCHAR szFileName[256],szPath[256];
	GetModuleFileName(0,szFileName,1023);
   ExtractFilePath(szFileName,szPath);
   wsprintf(szFileName,_T("%simgupload.ini"),szPath);

	TCHAR szBuffer1[128],szBuffer2[128];
	TCHAR szBuffer3[256];
	
	lstrcpy(szBuffer2,GetFileExt(szSettingName));
	GetOnlyFileName(szSettingName,szBuffer1);
	if(!szString)
	{
		wsprintf(szBuffer3,_T("%d"),Value);
		WritePrivateProfileString(szBuffer1,szBuffer2, szBuffer3, szFileName);
	}
	else
		
	WritePrivateProfileString(szBuffer1,szBuffer2, szString, szFileName);
	
	return true;
}
*/
int GetSavingFormat(LPCTSTR szFileName)
{
	if(!szFileName) return -1;
	LPCTSTR FileType = GetFileExt(szFileName);
	if(IsStrInList(FileType,_T("jpg\0jpeg\0\0")))
		return 0;
	else if(IsStrInList(FileType,_T("png\0\0")))
		return 1;
	else if(IsStrInList(FileType,_T("gif\0\0")))
		return 2;
	else return 0;
}

int MyGetFileSize(LPCTSTR FileName)
{
	HANDLE hFile = CreateFile(FileName, /*GENERIC_READ*/0, 0, 0, OPEN_EXISTING, 0, 0);
   if(!hFile) return -1;
	int fSize = GetFileSize (hFile, NULL); 
	CloseHandle(hFile);
	return fSize;
}


LPTSTR fgetline(LPTSTR buf,int num,FILE *f)
{
	LPTSTR Result;
	LPTSTR cur;
	Result=_fgetts(buf,num,f);
	int n=lstrlen(buf)-1;

	for(cur=buf+n;cur>=buf;cur--)
	{
		if(*cur==13||*cur==10||*cur==_T('\n'))
			*cur=0;
		else break;
	}
	return Result;
}



BOOL FileExists(LPCTSTR FileName)
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


#undef PixelFormat8bppIndexed 
#define PixelFormat8bppIndexed (3 | ( 8 << 8) | PixelFormatIndexed | PixelFormatGDI)



CString DisplayError(int idCode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,
						idCode, 0, (LPTSTR) &lpMsgBuf, 0, NULL);
	CString res = (LPCTSTR)lpMsgBuf;
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return res;
}


/* MakeFontBold
	MakeFontUnderLine

	-----------------------
	These functions create bold/underlined fonts based on given font
*/
HFONT MakeFontBold(HFONT font)
{
	if(!font) return 0;

	LOGFONT alf;

	bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

	if(!ok) return 0;

	alf.lfWeight = FW_BOLD;

	HFONT NewFont = CreateFontIndirect(&alf);
	return NewFont;
}

HFONT MakeFontUnderLine(HFONT font)
{
	if(!font) return 0;

	LOGFONT alf;

	bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

	if(!ok) return 0;

	alf.lfUnderline = 1;
	HFONT NewFont = CreateFontIndirect(&alf);

	return NewFont;
}


LPTSTR MoveToEndOfW(LPTSTR szString,LPTSTR szPattern)
{
	int nLen;

	if(!szString || !szPattern) return szString;

	nLen = wcslen(szPattern);
	if(!nLen) return szString;
	
	LPTSTR szStart = (LPTSTR) wcsstr(szString, szPattern);

	if(!szStart) return szString;
	else szString = szStart+nLen;

	return szString;
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

#ifndef IU_SHELLEXT
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
    int n = WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size()+1, /*dst*/NULL, 0, /*defchr*/0, NULL);
    if(n)
    {
        str.resize(n-1);
        if(WideCharToMultiByte(codePage, 0, ws.c_str(), ws.size()+1, /*dst*/&str[0], n, /*defchr*/0, NULL) == 0)
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

const std::wstring Utf8ToWstring(const std::string &str)
{
	
	return strtows(str, CP_UTF8);
}



std::string chcp(const std::string &str, UINT codePageSrc, UINT codePageDst)
{
    return wstostr(strtows(str, codePageSrc), codePageDst);
} 
#endif
bool IsDirectory(LPCTSTR szFileName)
{
	DWORD res = GetFileAttributes(szFileName);
	 return (res&FILE_ATTRIBUTE_DIRECTORY) && (res != -1);	
}

bool IsVista()
{
	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

	if (	::GetVersionEx( &osver ) && 
		osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
		(osver.dwMajorVersion >= 6 ) )
		return TRUE;

	return FALSE;
}


bool CheckFileName(const CString& fileName)
{
	return (fileName.FindOneOf(_T("\\/:*?\"<>|")) < 0);
}