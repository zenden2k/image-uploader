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
#ifndef IU_FUNC_CMDLINE_H
#define IU_FUNC_CMDLINE_H

#pragma once
#include "atlheaders.h"

class CCmdLine
{
    public:
        CCmdLine();
        CCmdLine &operator=(const CCmdLine& p);
        explicit CCmdLine(LPCTSTR szCmdLine);
        void Parse(LPCTSTR szCmdLine);
        size_t AddParam(LPCTSTR szParam);

        //Returns command line without module name
        CString OnlyParams();
        CString operator[](size_t nIndex);
        CString ModuleName();
        bool GetNextFile(CString& FileName, size_t& nIndex);
        bool IsOption(LPCTSTR Option, bool bUsePrefix = true);
        size_t GetCount();
    private:
        CStringList m_Params;
        CString m_OnlyParams;
};

extern CCmdLine CmdLine;
#endif // IU_FUNC_CMDLINE_H
