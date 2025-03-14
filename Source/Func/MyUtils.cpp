/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "MyUtils.h"

#include <set>
#include <string>

#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Video/VideoUtils.h"

bool IsVideoFile(LPCTSTR szFileName) {
    std::vector<std::string>& v = VideoUtils::instance().videoFilesExtensions;
    static const std::set<std::string> extensions(v.begin(), v.end());
    const std::string ext = IuStringUtils::toLower(IuCoreUtils::ExtractFileExt(W2U(szFileName)));
    return extensions.find(ext) != extensions.end();
}

CString PrepareVideoDialogFilters() {
    CString result;
    for (const auto& ex : VideoUtils::instance().videoFilesExtensions) {
        result += _T("*.");
        result += ex.c_str();
        result += _T(";");
    }
    return result;
}

CString PrepareAudioDialogFilters() {
    CString result;
    for (const auto& ex : VideoUtils::instance().audioFilesExtensions) {
        result += _T("*.");
        result += ex.c_str();
        result += _T(";");
    }
    return result;
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

LPCTSTR  CopyToStartOfW(LPCTSTR szString,LPCTSTR szPattern,LPTSTR szBuffer,int nBufferSize)
{
    int nLen=0;
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
