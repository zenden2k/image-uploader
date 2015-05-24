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

#include "langclass.h"
#include "atlheaders.h"
#include "myutils.h"
#include "Func/WinUtils.h"
#include "Core/AppParams.h"
#include "Core/Utils/CoreUtils.h"

CLang Lang;

// TODO: rewrite this shit
// check it's compatibility with 64 bit platforms
BYTE hex_digit(TCHAR f)
{
    BYTE p;
    if (f >= _T('0') && f <= _T('9'))
    {
        p = f - _T('0');
    }
    else
    {
        p = f - _T('a') + 10;
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
    for (int i = 0; i < len; i += 2)
    {
        ATLASSERT(i / 2 <= 3);
        b[i / 2] = hex_digit(hex[i]) * 16 + hex_digit(hex[i + 1]);
        step *= 16;
    }
    return *(DWORD*)&b;
}

int myhash(PBYTE key, int len)
{
    int hash = 222;
    for (int i = 0; i < len; ++i)
    {
        hash = (hash ^ key[i]) + ((hash << 26) + (hash >> 6));
    }

    return hash;
}

CLang::CLang()
{
    *m_Directory = 0;
    locale_ = "ru_RU";
    language_ = "ru";
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

    FILE* f = _tfopen(Filename, _T("rb"));
    if (!f) {
        if ( Lang == CString("Русский") ) {
            AppParams::instance()->setLanguageFile(IuCoreUtils::WstringToUtf8((LPCTSTR)(CString(m_Directory) + "Russian" + _T(".lng"))));
        }
        return false;
    }

    fseek(f, 2, 0);
    TCHAR Buffer[1024];
    TCHAR Name[128];
    TCHAR Text[512];

    while (!feof(f))
    {
        *Buffer = *Name = *Text = 0;
        fgetline(Buffer, sizeof(Buffer) / sizeof(TCHAR), f);

        if (*Buffer == _T('#'))
            continue;
        ExtractStrFromList(Buffer, 0, Name, sizeof(Name) / sizeof(TCHAR), _T(""), _T('='));
        ExtractStrFromList(Buffer, 1, Text, sizeof(Text) / sizeof(TCHAR), _T(""), _T('='));

        if (Name[lstrlen(Name) - 1] == _T(' '))
            Name[lstrlen(Name) - 1] = 0;

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
        };

        TranslateListItem tli = {NULL, NULL};
        tli.Name = pName;
        tli.Text = pText;
        int hash = hexstr2int(pName);
        StringList[hash] = tli;
    }

    fclose(f);
    m_sLang = Lang;
    AppParams::instance()->setLanguageFile(IuCoreUtils::WstringToUtf8((LPCTSTR)Filename));
    return true;
}

LPTSTR CLang::GetString(LPCTSTR Name) {
    int hash = myhash((PBYTE)Name, lstrlen(Name) * sizeof(TCHAR));
    auto it = StringList.find(hash);
    if (it != StringList.end()) {
        return it->second.Text;
    }

    // return _T("$NO_SUCH_STRING");
    return (LPTSTR)Name;
}

CString CLang::GetLanguageName()
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
        fseek(f, 2, 0); // skipping BOM
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

CLang::~CLang()
{
    for (auto& it : StringList) {
        delete[] it.second.Name;
        delete[] it.second.Text;
    }
}
