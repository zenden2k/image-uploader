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

#ifndef LANGCLASS_H
#define LANGCLASS_H
#pragma once

#include "atlheaders.h"

#include <string>
#include <unordered_map>
#ifndef IU_SHELLEXT
    #include "Core/i18n/Translator.h"
    #define ITRANLATOR_OVERRIDE override
#else
    #define ITRANLATOR_OVERRIDE 
    #define TR(str) Lang.GetString(_T(str))
#endif

class CLang
#ifndef IU_SHELLEXT
: public ITranslator
#endif
{
    public:
        CLang();
        ~CLang();
        LPCTSTR GetString(LPCTSTR Name) const;
        bool SetDirectory(LPCTSTR Directory);
        bool LoadLanguage(LPCTSTR Lang);
        CString GetLanguageName() const;
        CString getLanguage() const;
        CString getLocale() const;
        CString getLanguageFileNameForLocale(const CString& locale);
        CString getCurrentLanguageFile() const;
        /**
            The RTL option is not being changed during program lifetime
        **/
        bool isRTL() const ITRANLATOR_OVERRIDE;
#ifndef IU_SHELLEXT
        std::string getCurrentLanguage() ITRANLATOR_OVERRIDE;
        std::string getCurrentLocale() ITRANLATOR_OVERRIDE;
        std::string translate(const char* str) ITRANLATOR_OVERRIDE;
        const wchar_t* translateW(const wchar_t* str) ITRANLATOR_OVERRIDE;
#endif
        CLang(const CLang&) = delete;
        CLang& operator=(const CLang) = delete;
    private:
        struct TranslateListItem
        {
            TCHAR *Name;
            TCHAR *Text;
        };
        TCHAR m_Directory[MAX_PATH];
        CString m_sLang;
        std::unordered_map<int, TranslateListItem> StringList;
        std::vector<CString> LanguagesList;
        CString locale_;
        CString language_;
        CString currentLanguageFile_;
        bool isRTL_;
};

#endif  // LANGCLASS_H
