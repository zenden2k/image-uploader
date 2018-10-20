/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "myutils.h"

#include "atlheaders.h"
#include <string>
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Video/VideoUtils.h"
#include "Func/WinUtils.h"

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

int GetFontSize(int nFontHeight)
{
    HDC dc = ::GetDC(nullptr);
    int res =  -MulDiv(nFontHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
    ReleaseDC(nullptr, dc);
    return res;
}

int GetFontHeight(int nFontSize)
{
    HDC dc = ::GetDC(nullptr);
    int res = -MulDiv(nFontSize, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);
    ReleaseDC(nullptr, dc);
    return res;
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
    CWindowDC dc(0);

    nPixelsPerInch = GetDeviceCaps(dc , LOGPIXELSY );
    
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
    lFont->lfCharSet=static_cast<BYTE>(nCharSet);
    
    return true;
}

LPTSTR ExtractFilePath(LPCTSTR FileName, LPTSTR buf)
{  
    int i, len = lstrlen(FileName);
    for(i=len-1; i>=0; i--)
    {
        if(FileName[i] == _T('\\'))
            break;
    }
    lstrcpyn(buf, FileName, i+2);
    return buf;
}

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

int GetSavingFormat(LPCTSTR szFileName)
{
    if(!szFileName) return -1;
    LPCTSTR FileType = GetFileExt(szFileName);
    if(WinUtils::IsStrInList(FileType,_T("jpg\0jpeg\0\0")))
        return 0;
    else if(WinUtils::IsStrInList(FileType,_T("png\0\0")))
        return 1;
    else if(WinUtils::IsStrInList(FileType,_T("gif\0\0")))
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

const CString TrimString(const CString& source, int nMaxLen)
{
    int nLen = source.GetLength();
    if(nLen <= nMaxLen) return source;

    int PartSize = (nMaxLen-3) / 2;
    return source.Left(PartSize)+_T("...")+source.Right(PartSize);
}

LPCTSTR  CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize)
{
    size_t nLen=0;
    if(!szString || !szPattern ||!szBuffer || nBufferSize<0) return 0;

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
    size_t nLen;

    if(!szString || !szPattern) return szString;

    nLen = wcslen(szPattern);
    if(!nLen) return szString;
    
    LPTSTR szStart = wcsstr(szString, szPattern);

    if(!szStart) return szString;
    else szString = szStart+nLen;

    return szString;
}

bool CheckFileName(const CString& fileName)
{
    return (fileName.FindOneOf(_T("\\/:*?\"<>|")) < 0);
}