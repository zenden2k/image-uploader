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

#include "langclass.h"

#include "Core/CommonDefs.h"
#include "myutils.h"
#include "Func/WinUtils.h"
#include "Core/AppParams.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/StringUtils.h"

CLang Lang;

namespace {
// TODO: rewrite this shit
// check it's compatibility with 64 bit platforms
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

}

CLang::CLang()
{
    *m_Directory = 0;
    locale_ = "en_US";
    language_ = "en";
    isRTL_ = false;
}

bool CLang::SetDirectory(LPCTSTR Directory)
{
    lstrcpyn(m_Directory, Directory, sizeof(m_Directory) / sizeof(TCHAR));
    return true;
}

bool CLang::LoadLanguage(LPCTSTR Lang)
{
    StringList.clear();
    if (!Lang ) {
        return false;
    }

    CString Filename = CString(m_Directory) + Lang + _T(".lng");

    std::string fileContents;

    if (Lang == CString("English")) { // English is built-in language
        AppParams::instance()->setLanguageFile(IuCoreUtils::WstringToUtf8((LPCTSTR)(CString(m_Directory) + _T("English.lng"))));
        return true;
    }

    if (!IuCoreUtils::ReadUtf8TextFile(W2U(Filename), fileContents)) {
       
        return false;
    }

    std::vector<std::string> lines;
    CString Buffer;
    TCHAR Name[128];
    TCHAR Text[512];
    IuStringUtils::Split(fileContents, "\r\n", lines, -1);

    for (const auto& line : lines) {
        Buffer.Empty();
        *Name = *Text = 0;
        Buffer = U2W(line);

        if (*Buffer == _T('#'))
            continue;
        WinUtils::ExtractStrFromList(Buffer, 0, Name, sizeof(Name) / sizeof(TCHAR), _T(""), _T('='));
        WinUtils::ExtractStrFromList(Buffer, 1, Text, sizeof(Text) / sizeof(TCHAR), _T(""), _T('='));

        int len = lstrlen(Name);
        if (len > 0 && Name[len - 1] == _T(' '))
            Name[len - 1] = 0;

        CString RepText = Text;
        RepText.Replace(_T("\\n"), _T("\r\n"));

        int NameLen = lstrlen(Name);
        int TextLen = lstrlen(RepText);

        if (!NameLen || !TextLen)
            continue;

        TCHAR* pName = new TCHAR[NameLen + 1];
        TCHAR* pText = new TCHAR[TextLen + 1];

        lstrcpy(pName, Name);
        lstrcpy(pText, (LPCTSTR)RepText + ((*Text == _T(' ')) ? 1 : 0));

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

        TranslateListItem tli = {NULL, NULL};
        tli.Name = pName;
        tli.Text = pText;
        int hash = hexstr2int(pName);
        StringList[hash] = tli;
    }

    m_sLang = Lang;
    AppParams::instance()->setLanguageFile(W2U(Filename));
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

CString CLang::getLanguageFileNameForLocale(const CString& locale)
{
    std::vector<CString> list;
    WinUtils::GetFolderFileList(list, m_Directory, _T("*.lng"));
    CString foundName;

    for(size_t i=0; i<list.size(); i++)
    {
        FILE* f = _tfopen(m_Directory + list[i], _T("rb"));
        if (!f) {
            continue;
        }
        fseek(f, 2, SEEK_SET); // skipping BOM
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
                    foundName =  list[i].Left(list[i].ReverseFind('.'));
                    fclose(f);
                    return foundName;
                }
                CString lang = value.Left(value.Find('_'));
                if (  lang == locale ) {
                    foundName =  list[i].Left(list[i].ReverseFind('.'));
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
