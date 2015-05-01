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

#ifndef COMMON_H
#define COMMON_H

#include "atlheaders.h"
#include <atlcoll.h>
#include <ctime>
#include "MyEngineList.h"
#include "3rdpart/GdiplusH.h"

class UploadEngineManager;
class CCmdLine;

struct CUrlListItem
{
    bool IsImage, IsThumb;
    CString FileName;
    CString ImageUrl;
    CString ImageUrlShortened;
    CString ThumbUrl;
    CString ThumbUrlShortened;
    CString DownloadUrl;
    CString DownloadUrlShortened;

    CString getDownloadUrl(bool shortened = false) {
        return (shortened && !DownloadUrlShortened.IsEmpty()) ? DownloadUrlShortened : DownloadUrl; 
    }

    CString getImageUrl(bool shortened = false) {
        return (shortened && !ImageUrlShortened.IsEmpty()) ? ImageUrlShortened : ImageUrl; 
    }

    CString getThumbUrl(bool shortened = false) {
        return (shortened && !ThumbUrlShortened.IsEmpty()) ? ThumbUrlShortened : ThumbUrl; 
    }

    bool isNull()
    {
        return ImageUrl.IsEmpty() && DownloadUrl.IsEmpty();
    }

};

bool IULaunchCopy();


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

void IU_ConfigureProxy(NetworkClient& nm);

const CString IU_GetVersion();
#define IU_NEWFOLDERMARK ("_iu_create_folder_")
void DeleteDir2(LPCTSTR Dir);
bool BytesToString(__int64 nBytes, LPTSTR szBuffer,int nBufSize);
bool IULaunchCopy(CString additionalParams=_T(""));

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


CString GetUniqFileName(const CString &filePath);
bool IU_GetClipboardText(CString &text);
extern CMyEngineList *_EngineList;

BOOL IU_CreateFolder(LPCTSTR szFolder);
BOOL IU_CreateFilePath(LPCTSTR szFilePath);
HICON GetAssociatedIcon (LPCTSTR filename, bool Small);
BOOL IsWinXP();
int ScreenBPP();
BOOL Is32BPP();
CString GetSystemSpecialPath(int csidl);
const CString GetApplicationDataPath();
const CString GetCommonApplicationDataPath();
HRESULT IsElevated( __out_opt BOOL * pbElevated );
// Function that gets path to SendTo folder
CString GetSendToPath() ;
void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");
void EncodeString(LPCTSTR szSource, CString &Result,LPSTR code="{DAb[]=_T('')+b/16;H3N SHJ");
CString IU_md5_file(const CString& filename);

typedef CAtlArray<CString> CStringList;
#endif