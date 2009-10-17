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

#ifndef COMMON_H
#define COMMON_H
#define IU_IDC_CONST 12255
#define IDC_SETTINGS		IU_IDC_CONST+1
#define IDC_REGIONPRINT IU_IDC_CONST+2
#define IDC_MEDIAFILEINFO IU_IDC_CONST+3
#define IDC_CLIPBOARD IU_IDC_CONST+4
#define IDC_ADDFOLDER IU_IDC_CONST+5
#include <deque>
#include "thread.h"
class CWizardDlg;
class CWizardPage
{
public:
	CWizardDlg *WizardDlg;
	virtual ~CWizardPage()=NULL;
	HBITMAP HeadBitmap;
	virtual bool OnShow();
	virtual bool OnHide();
	virtual bool OnNext();
	void EnableNext(bool Enable=true);
	void EnablePrev(bool Enable=true);
	void EnableExit(bool Enable=true);
	void SetNextCaption(LPTSTR Caption);
	HWND PageWnd;
	void ShowNext(bool Show=true);
	void ShowPrev(bool Show=true);
};


struct ActionVariable
{
	CString Name;
	int nIndex;
};

struct UploadAction
{
	UINT Index;
	bool IgnoreErrors;
	bool OnlyOnce;
	CString Url;
	CString Description;
	CString Referer;
	CString PostParams;
	CString Type;
	CString RegExp;
	std::vector<ActionVariable> Variables;

	//----
	int RetryLimit;
	int NumOfTries;
};

struct CUrlListItem{
	bool IsImage, IsThumb;
	CString FileName;
	TCHAR ImageUrl[256];
	TCHAR ThumbUrl[256];
	CString DownloadUrl;

};
struct UploadEngine
{
	TCHAR Name[64];
	bool Debug;
	bool ImageHost;
	bool SupportThumbnails, NeedAuthorization;
	DWORD MaxFileSize;
	bool Auth;
	CString CodedLogin;
	CString CodedPassword;
	CString ThumbUrlTemplate, ImageUrlTemplate, DownloadUrlTemplate;
	std::vector<UploadAction> Actions;
	//--------
	int RetryLimit;
	int NumOfTries;

};

struct InfoProgress
{
	DWORD Uploaded,Total;
	bool IsUploading;
	std::deque<DWORD> Bytes;
	CAutoCriticalSection CS;
};

bool IULaunchCopy();
BOOL CreateTempFolder();
void ClearTempFolder();

extern CAtlArray< UploadEngine> EnginesList;
int GetUploadEngineIndex(const CString Name);
extern CString IUTempFolder;
extern CCmdLine CmdLine;
bool MySaveImage(Image *img,LPTSTR szFilename,LPTSTR szBuffer,int Format,int Quality);
bool __fastcall CreateShortCut( 
							 LPCWSTR pwzShortCutFileName, 
							   LPCTSTR pszPathAndFileName, 
							   LPCTSTR pszWorkingDirectory, 
							   LPCTSTR pszArguments, 
							   WORD wHotKey, 
							   int iCmdShow, 
							   LPCTSTR pszIconFileName, 
							   int iIconIndex) ;
void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Gdiplus::Font &font,Color &ColorText,Color &ColorStroke,int HorPos=0,int VertPos=0, int width=1);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

bool IULaunchCopy(CString params, CAtlArray<CString> &files);

#endif