/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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
#ifndef IU_SHELLEXT
#include "stdafx.h"
#endif
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>

#define APPNAME _T("Image Uploader")
#define VIDEO_DIALOG_FORMATS _T("Video files (avi, mpg, vob, wmv, flv, etc)\0*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;\0All files\0*.*\0\0")
#define VIDEO_FORMATS _T("avi\0mpg\0mpeg\0vob\0divx\0flv\0wmv\0asf\0mkv\0mov\0ts\0mp2\0mp4\0")_T("3gp\0rm\0mpeg2ts\0\0")
#define IMAGE_DIALOG_FORMATS _T("Image files (JPEG, GIF, PNG, etc)\0*.jpg;*.gif;*.png;*.bmp;*.tiff\0All files\0*.*\0\0")

using namespace Gdiplus;
#define xor(a,b) ((a || b) && !(a && b))	

#define CheckBounds(n,a,b,d) {if((n<a) || (n>b)) n=d;}


int GetFontSize(int nFontHeight);
int GetFontHeight(int nFontSize);

bool ExtractStrFromList(
            LPCTSTR szString /* Source string */,
            int nIndex, /* Zero based item index */
            LPTSTR szBuffer /* Destination buffer */,
            LONG nSize ,/* Length in characters of destionation buffer */
            LPCTSTR szDefString = NULL,
            TCHAR cSeparator = _T(',') /* Character to be separator in list */);
#define LOADICO(ico) LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(ico))

bool FontToString(LPLOGFONT lFont, CString &Result);
bool StringToFont(LPCTSTR szBuffer,LPLOGFONT lFont);
LPTSTR ExtractFilePath(LPCTSTR FileName,LPTSTR buf);
int GetFontSizeInTwips(int nFontSize);
LPCTSTR myExtractFileName(LPCTSTR FileName);

LPCTSTR GetFileExt(LPCTSTR szFileName);
bool IsImage(LPCTSTR szFileName);
bool IsVideoFile(LPCTSTR szFileName);

bool GetOnlyFileName(LPCTSTR szFilename,LPTSTR szBuffer);
bool ReadSetting(LPTSTR szSettingName,int* Value,int DefaultValue,LPTSTR szString=NULL,LPTSTR szDefString=NULL);
int GetSavingFormat(LPTSTR szFileName);
bool IsStrInList(LPCTSTR szExt,LPCTSTR szList);
int MyGetFileSize(LPCTSTR FileName);
void MakeLabelBold(HWND Label);
LPTSTR fgetline(LPTSTR buf,int num,FILE *f);
#define IsChecked(ctrl) (SendDlgItemMessage(ctrl,BM_GETCHECK,0)==BST_CHECKED)

CString GetAppFolder();
BOOL FileExists(LPCTSTR FileName);
void TrimString(LPTSTR Destination, LPCTSTR Source, int MaxLen);
void FillRectGradient(HDC hdc, RECT FillRect, COLORREF start, COLORREF finish, bool Horizontal);
bool NewBytesToString(__int64 nBytes, LPTSTR szBuffer, int nBufSize);
bool SelectDialogFilter(LPTSTR szBuffer, int nMaxSize, int nCount, LPCTSTR szName, LPCTSTR szFilter,...);
LPCTSTR  CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize);


CString IntToStr(int n);

void EnableNextN(HWND Control, int n, bool Enable);
CString DisplayError(int idCode);

HFONT MakeFontUnderLine(HFONT font);
HFONT MakeFontBold(HFONT font);
LPTSTR MoveToEndOfW(LPTSTR szString,LPTSTR szPattern);

#ifdef DEBUG
	void ShowX(LPCTSTR str,int line,int n);

	void ShowX(LPCTSTR str,int line,float n);
	void ShowX(LPCTSTR str,int line,LPCTSTR n);
	#define ShowVar(n) ShowX(_T(#n),__LINE__,n)
#endif
void EnableNextN(HWND Control, int n, bool Enable);
bool IUInsertMenu(HMENU hMenu, int pos, UINT id, const LPCTSTR szTitle,  HBITMAP bm=0);
#endif