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

#ifndef IU_SHELLEXT
    #include "Core/Utils/CoreUtils.h"
#endif

namespace {
// TODO: rewrite this shit
BYTE hex_digit(TCHAR f)
{
    BYTE p;
    if (f >= _T('0') && f <= _T('9')) {
        p = static_cast<BYTE>(f - _T('0'));
    } else {
        p = static_cast<BYTE>(f - _T('a') + 10);
    }
    return p;
}

int hexstr2int(LPTSTR hex)
{
    int len = lstrlen(hex);
    int  step = 1;

    BYTE b[4];
    if (len > 8)
        return 0;
    for (int i = 0; i < len; i += 2) {
        //ATLASSERT(i / 2 <= 3);
        b[i / 2] = hex_digit(hex[i]) * 16 + hex_digit(hex[i + 1]);
        step *= 16;
    }
    return *(DWORD*)&b;
}

int myhash(PBYTE key, int len)
{
    int hash = 222;
    for (int i = 0; i < len; ++i) {
        hash = (hash ^ key[i]) + ((hash << 26) + (hash >> 6));
    }

    return hash;
}

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
    StringList.clear();
    if (!Lang ) {
        return false;
    }

    CString Filename = CString(m_Directory) + Lang + _T(".lng");

    std::string fileContents;

    if (Lang == CString("English")) { // English is a built-in language
        currentLanguageFile_ = Filename;
        return true;
    }

    if (!ReadUtf8TextFile(Filename, fileContents)) {
       
        return false;
    }
    std::wstring fileContentsW = StringToWideString(fileContents, CP_UTF8);

    std::wstringstream ss(fileContentsW);
    std::wstring line;

    CString Buffer;
    CString Name;
    CString Text;

    while (std::getline(ss, line)) {
        
        Buffer.Empty();
        Name.Empty();
        Text.Empty();
        if (!line.empty() && line[line.length()-1]==L'\r') {
            line.pop_back(); // remove last character
        }
        Buffer = line.c_str();

        if (*Buffer == _T('#'))
            continue;

        int pos = Buffer.Find('=');

        if (pos!=-1) {
            Name = Buffer.Left(pos);
            Text = Buffer.Mid(pos + 1);
        }

        Name.Trim();

        CString RepText = Text;
        RepText.Replace(_T("\\n"), _T("\r\n"));

        if (!RepText.IsEmpty() && RepText[0] == _T(' ')) {
            RepText = RepText.Mid(1);
        }

        int NameLen = Name.GetLength();
        int TextLen = RepText.GetLength();

        if (!NameLen || !TextLen)
            continue;

        TCHAR* pName = new TCHAR[NameLen + 1];
        TCHAR* pText = new TCHAR[TextLen + 1];

        lstrcpy(pName, Name);
        lstrcpy(pText, RepText);

        if ( Name == CString("language") ) {
            locale_ = pText;
            language_ = locale_.Left(locale_.Find('_'));
            delete[] pName;
            delete[] pText;
            continue;
        } else if (Name == CString(_T("RTL"))) {
            CString lowerText = CString(pText).MakeLower();
            if (lowerText == "yes" || lowerText == "1" || lowerText == "true") {
                isRTL_ = true;
            }
            delete[] pName;
            delete[] pText;
            continue;
        }

        TranslateListItem tli = {nullptr, nullptr};
        tli.Name = pName;
        tli.Text = pText;
        int hash = hexstr2int(pName);
        StringList[hash] = tli;
    }

    m_sLang = Lang;
    currentLanguageFile_ = Filename;
    return true;
}

LPCTSTR CLang::GetString(LPCTSTR Name) const {
    int hash = myhash((PBYTE)Name, lstrlen(Name) * sizeof(TCHAR));
    auto it = StringList.find(hash);
    if (it != StringList.end()) {
        return it->second.Text;
    }

    // return _T("$NO_SUCH_STRING");
    return Name;
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
    std::wstring wideStr = IuCoreUtils::Utf8ToWstring(str);
    return IuCoreUtils::WstringToUtf8(GetString(wideStr.c_str()));
}

const wchar_t* CLang::translateW(const wchar_t* str)  {
    return GetString(str);
}
#endif

CString CLang::getLanguageFileNameForLocale(const CString& locale)
{
    std::vector<CString> list;
    GetFolderFileList(list, m_Directory, _T("*.lng"));
    CString foundName;

    for(const auto& fileName: list )
    {
        FILE* f = _tfopen(m_Directory + fileName, _T("rb"));
        if (!f) {
            continue;
        }
        //fseek(f, 2, SEEK_SET); // skipping BOM
        TCHAR buffer[1024];

        while (!feof(f))
        {
            memset(buffer, 0, sizeof(buffer));
            fgetline(buffer, 1024, f);
            CString buf = buffer;
            if (buf.GetLength() && buf[0] == _T('#')) {
                continue;
            }
            
            int equalSignPos = buf.Find(L'=');
            CString key = buf.Left(equalSignPos);
            key.TrimRight(L" ");
            CString value = buf.Right(buf.GetLength() - equalSignPos-1);
            value.TrimLeft(L" ");
            if ( key == "language" ) {
                if ( value == locale ) {
                    foundName = fileName.Left(fileName.ReverseFind('.'));
                    fclose(f);
                    return foundName;
                }
                CString lang = value.Left(value.Find('_'));
                if (  lang == locale ) {
                    foundName = fileName.Left(fileName.ReverseFind('.'));
                    break;
                }
            }
        }
        fclose(f);
    }
    return foundName;
}

bool CLang::isRTL() const {
    return isRTL_;
}

CLang::~CLang()
{
    for (auto& it : StringList) {
        delete[] it.second.Name;
        delete[] it.second.Text;
    }
}

CString CLang::getCurrentLanguageFile() const {
    return currentLanguageFile_;
}