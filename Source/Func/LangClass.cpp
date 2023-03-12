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

#include "LangClass.h"

#include <cstdio>
#include <filesystem>
#include <sstream>
#include <boost/locale.hpp>

#include "Core/Utils/CoreUtils.h"
#include "Func/WinUtils.h"
#include "Gui/Helpers/LangHelper.h"

namespace {


LPTSTR fgetline(LPTSTR buf, int num, FILE *f)
{
    LPTSTR Result;
    LPTSTR cur;
    Result = _fgetts(buf, num, f);
    int n = lstrlen(buf) - 1;

    for (cur = buf + n; cur >= buf; cur--)
    {
        if (*cur == 13 || *cur == 10 || *cur == _T('\n'))
            *cur = 0;
        else break;
    }
    return Result;
}

size_t GetFolderFileList(std::vector<CString>& list, CString folder, CString mask)
{
    WIN32_FIND_DATA wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    HANDLE findfile = nullptr;
    TCHAR szNameBuffer[MAX_PATH];

    for (;; )
    {
        if (!findfile)
        {
            findfile = FindFirstFile(folder + _T("\\") + mask, &wfd);
            if (!findfile)
                break;
            ;
        }
        else
        {
            if (!FindNextFile(findfile, &wfd))
                break;
        }
        if (lstrlen(wfd.cFileName) < 1)
            break;
        lstrcpyn(szNameBuffer, wfd.cFileName, 254);
        list.push_back(szNameBuffer);
    }
    // return TRUE;

    // error:
    if (findfile)
        FindClose(findfile);
    return list.size();
    // return FALSE;
}

bool ReadUtf8TextFile(const CString& fileName, std::string& data)
{
    FILE *stream = _wfopen(fileName, L"rb");
    if (!stream) {
        return false;
    }
    fseek(stream, 0L, SEEK_END);
    size_t size = ftell(stream);
    rewind(stream);

    unsigned char buf[3] = { 0,0,0 };
    size_t bytesRead = fread(buf, 1, 3, stream);

    if (bytesRead == 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) // UTF8 Byte Order Mark (BOM)
    {
        size -= 3;
    }
    else if (bytesRead >= 2 && buf[0] == 0xFF && buf[1] == 0xFE) {
        // UTF-16LE encoding
        // not supported
        fclose(stream);
        return false;
    }
    else {
        // no BOM was found; seeking backward
        fseek(stream, 0L, SEEK_SET);
    }
    try {
        data.resize(size);
    }
    catch (std::exception& ) {
        //LOG(ERROR) << ex.what();
        fclose(stream);
        return false;
    }

    size_t bytesRead2 = fread(&data[0], 1, size, stream);
    if (bytesRead2 == size) {
        fclose(stream);
        return true;
    }

    fclose(stream);
    return false;
}

std::wstring StringToWideString(const std::string &str, UINT codePage)
{
    std::wstring ws;
    int n = MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size() + 1), /*dst*/NULL, 0);
    if (n) {
        ws.reserve(n);
        ws.resize(n - 1);
        if (MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size() + 1), /*dst*/&ws[0], n) == 0)
            ws.clear();
    }
    return ws;
}

std::string WideStringToString(const std::wstring& ws, UINT codePage)
{
    // prior to C++11 std::string and std::wstring were not guaranteed to have their memory be contiguous,
    // although all real-world implementations make them contiguous
    std::string str;
    int srcLen = static_cast<int>(ws.size());
    int n = WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, NULL, 0, /*defchr*/0, NULL);
    if (n) {
        str.reserve(n);
        str.resize(n - 1);
        if (WideCharToMultiByte(codePage, 0, ws.c_str(), srcLen + 1, &str[0], n, /*defchr*/0, NULL) == 0)
            str.clear();
    }
    return str;
}

}

CLang::CLang()
{
    locale_ = "en_US";
    language_ = "en";
    isRTL_ = false;
}
void CLang::SetDirectory(LPCTSTR Directory)
{
    m_Directory = Directory;
}

bool CLang::LoadLanguage(LPCTSTR Lang)
{
    boost::locale::generator gen;
    std::string path = W2U(WinUtils::GetAppFolder() + _T("Lang\\locale\\"));
    gen.add_messages_path(path);
    gen.add_messages_domain("imageuploader");

    try {
        std::locale::global(gen(W2U(Lang) + ".UTF-8"));
    }
    catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
  
    locale_ = Lang;
    int ipos = locale_.Find(_T('_'));
    if (ipos >= 0) {
        language_ = locale_.Mid(0, ipos);
    } else {
        language_ = locale_;
    }

    auto locales = LangHelper::getLocaleList();
    auto it = locales.find(W2U(locale_));

    m_sLang = it == locales.end() ? locale_ : U2W(it->second);

    return true;
}

CString CLang::GetLanguageName() const
{
    return m_sLang;
}

CString CLang::getLanguage() const
{
    return language_;
}

CString CLang::getLocale() const
{
    return locale_;
}
#ifndef IU_SHELLEXT
std::string CLang::getCurrentLanguage() {
    return W2U(m_sLang);
}

std::string CLang::getCurrentLocale() {
    return W2U(locale_);
}

std::string CLang::translate(const char* str) {
    return boost::locale::translate(str);
}

std::wstring CLang::translateW(const char* str)  {
    return IuCoreUtils::Utf8ToWstring(boost::locale::translate(str));
}
#endif

bool CLang::isRTL() const {
    return std::string(_("LAYOUT_DIRECTION")) == "RTL";
}

CLang::~CLang()
{
}

CString CLang::getCurrentLanguageFile() const {
    return currentLanguageFile_;
}

std::string CLang::getLanguageDisplayName() const {
    return W2U(m_sLang);
}