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
#include "Core/i18n/Translator.h"
#include <string>
#include <unordered_map>

class CLang : public ITranslator
{
    public:
        CLang();
        ~CLang();
        LPCTSTR GetString(LPCTSTR Name);
        bool SetDirectory(LPCTSTR Directory);
        bool LoadLanguage(LPCTSTR Lang);
        CString GetLanguageName();
        CString getLanguage() const;
        CString getLocale() const;
        CString getLanguageFileNameForLocale(const CString& locale);
        virtual std::string getCurrentLanguage() override;
        virtual std::string getCurrentLocale() override;
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
};
#if defined(IU_WTL_APP) || defined(IU_SERVERLISTTOOL) || defined(IU_SHELLEXT)
extern CLang Lang;

#endif

// Begin: translation macros
#ifdef IU_TESTS
#define TR(str) _T(str)
#else
#define TR(str) Lang.GetString(_T(str))
#endif

#ifdef NDEBUG
#define TRC(c, str) SetDlgItemText(c, Lang.GetString(_T(str)))
#else
#define TRC(c, str) (ATLASSERT(GetDlgItem(c)),SetDlgItemText(c, Lang.GetString(_T(str))), (void)0)
#endif
#define TR_CONST(str) const_cast<LPWSTR>(TR(str))
// End

#endif  // LANGCLASS_H
