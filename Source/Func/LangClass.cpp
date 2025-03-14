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

#include <filesystem>
#include <boost/locale.hpp>

#include "Core/Utils/CoreUtils.h"
#include "Func/WinUtils.h"
#include "Gui/Helpers/LangHelper.h"

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

    auto locales = LangHelper::instance()->getLocaleList();
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
