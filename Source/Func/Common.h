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

#ifndef COMMON_H
#define COMMON_H

#include <atlcoll.h>
#include <ctime>
#include "PluginLoader.h"
#include "MyEngineList.h"

class CPluginManager;
class CCmdLine;

struct CUrlListItem
{
	bool IsImage, IsThumb;
	CString FileName;
	CString ImageUrl;
	CString ThumbUrl;
	CString DownloadUrl;
};

bool IULaunchCopy();
BOOL CreateTempFolder();
void ClearTempFolder(LPCTSTR folder);
extern CString IUTempFolder;

extern CString IUCommonTempFolder;
extern CCmdLine CmdLine;

bool __fastcall CreateShortCut( 
							 LPCWSTR pwzShortCutFileName, 
							   LPCTSTR pszPathAndFileName, 
							   LPCTSTR pszWorkingDirectory, 
							   LPCTSTR pszArguments, 
							   WORD wHotKey, 
							   int iCmdShow, 
							   LPCTSTR pszIconFileName, 
							   int iIconIndex) ;
#define MYRGB(a,color) Color(a,GetRValue(color),GetGValue(color),GetBValue(color))

bool IULaunchCopy(CString params, CAtlArray<CString> &files);

extern CPluginManager iuPluginManager;

void IU_ConfigureProxy(NetworkManager& nm);

const CString IU_GetVersion();
#define IU_NEWFOLDERMARK ("_iu_create_folder_")
void DeleteDir2(LPCTSTR Dir);
bool BytesToString(__int64 nBytes, LPTSTR szBuffer,int nBufSize);
bool IULaunchCopy(CString additionalParams=_T(""));
int GetFolderFileList(std::vector<CString> &list, CString folder, CString mask);
inline COLORREF RGB2COLORREF(unsigned int color)
{
	return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
}

inline unsigned int COLORREF2RGB( COLORREF color)
{
	return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
}
void IU_RunElevated(CString params);
HRESULT IsElevated( __out_opt BOOL * pbElevated );
#define randomize() (srand((unsigned)time(NULL)))
#define random(x) (rand() % x)
bool IU_CopyTextToClipboard(CString text);
DWORD MsgWaitForSingleObject(HANDLE pHandle, DWORD dwMilliseconds);
int dlgX(int WidthInPixels);
int dlgY(int HeightInPixels);
CString GenerateFileName(const CString &templateStr, int index,const CPoint size, const CString& originalName=_T(""));
CString GetUniqFileName(const CString &filePath);
bool IU_GetClipboardText(CString &text);
extern CMyEngineList *_EngineList;
const CString IU_GetWindowText(HWND wnd);
BOOL IU_CreateFolder(LPCTSTR szFolder);
BOOL IU_CreateFilePath(LPCTSTR szFilePath);
HICON GetAssociatedIcon (LPCTSTR filename, bool Small);
BOOL IsWinXP();
int ScreenBPP();
BOOL Is32BPP();
const CString IU_GetDataFolder();
CString GetSystemSpecialPath(int csidl);
const CString GetApplicationDataPath();
const CString GetCommonApplicationDataPath();
HRESULT IsElevated( __out_opt BOOL * pbElevated );
// Function that gets path to SendTo folder
CString GetSendToPath() ;
Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);
void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");
void EncodeString(LPCTSTR szSource, CString &Result,LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");
CString IU_md5_file(const CString& filename);

typedef CAtlArray<CString> CStringList;
#endif