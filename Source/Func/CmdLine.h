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
#ifndef IU_FUNC_CMDLINE_H
#define IU_FUNC_CMDLINE_H

#pragma once
#include "atlheaders.h"

class CCmdLine
{
    public:
        CCmdLine();
        CCmdLine(const CCmdLine&) = delete;
        CCmdLine &operator=(const CCmdLine& p);
        explicit CCmdLine(LPCTSTR szCmdLine);
        void Parse(LPCTSTR szCmdLine);
        size_t AddParam(LPCTSTR szParam);

        //Returns command line without module name
        CString OnlyParams() const;
        CString operator[](size_t nIndex) const;
        CString ModuleName() const;
        bool GetNextFile(CString& FileName, size_t& nIndex) const;
        bool IsOption(LPCTSTR Option, bool bUsePrefix = true) const;
        bool RemoveOption(LPCTSTR option);
        size_t GetCount() const;
        std::vector<CString>::const_iterator begin() const;
        std::vector<CString>::const_iterator end() const;
    private:
        std::vector<CString> params_;
        CString onlyParams_;
};

extern CCmdLine CmdLine;
#endif // IU_FUNC_CMDLINE_H
