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

#ifndef COMMON_H
#define COMMON_H

#include "atlheaders.h"
#include <atlcoll.h>

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

#define MYRGB(a,color) Color(a,GetRValue(color),GetGValue(color),GetBValue(color))

bool IULaunchCopy(CString params, const CAtlArray<CString> &files);

const CString IU_GetVersion();
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
DWORD MsgWaitForSingleObject(HANDLE pHandle, DWORD dwMilliseconds);
HICON GetAssociatedIcon (LPCTSTR filename, bool Small);
int ScreenBPP();
BOOL Is32BPP();
HRESULT IsElevated( __out_opt BOOL * pbElevated );
// Function that gets path to SendTo folder
CString GetSendToPath() ;
void DecodeString(LPCTSTR szSource, CString &Result, LPSTR code = "{DAb[]=_T('')+b/16;H3N SHJ");
void EncodeString(LPCTSTR szSource, CString &Result, LPSTR code = "{DAb[]=_T('')+b/16;H3N SHJ");

typedef CAtlArray<CString> CStringList;
#endif