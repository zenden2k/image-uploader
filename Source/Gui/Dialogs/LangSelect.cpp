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

#include "LangSelect.h"

#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/Helpers/LangHelper.h"

LRESULT CLangSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    langListCombo_ = GetDlgItem(IDC_LANGLIST);
    LogoImage.SubclassWindow(GetDlgItem(IDC_STATICLOGO));
    LogoImage.SetWindowPos(0, 0,0, 48, 48, SWP_NOMOVE|SWP_NOZORDER );
    LogoImage.LoadImage(0, 0, IDR_ICONMAINNEW, false, GetSysColor(COLOR_BTNFACE));
    
    boldFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_PLEASECHOOSE));

    // English language is always in language list
    langListCombo_.AddString(_T("English"));

    std::vector<std::wstring> languageList{ LangHelper::getLanguageList((WinUtils::GetAppFolder() + "Lang").GetString()) };

    for (const auto& language : languageList) {
        langListCombo_.AddString(language.c_str());
    }

    if (languageList.empty()) {
        EndDialog(IDOK);
        language_ = _T("English");
        return 0;
    }
    TCHAR buffer[LOCALE_NAME_MAX_LENGTH];
    GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH);
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    // FIXME: detect system language and select corresponding language file
    if ( !settings->Language.IsEmpty() ) {
        SelectLang(settings->Language);
    } else  if (GetUserDefaultLangID() == 0x419) {
        SelectLang(_T("Russian"));
    } else if (GetUserDefaultLangID() == 0x0418) {
        SelectLang(_T("Romanian"));
    } else {
        SelectLang(_T("English"));
    }

    return 1;
}

LRESULT CLangSelect::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int Index = langListCombo_.GetCurSel();
    if (Index < 0) {
        return 0;
    }
    langListCombo_.GetLBText(Index, language_);
    return EndDialog(wID);
}

LRESULT CLangSelect::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

void CLangSelect::SelectLang(LPCTSTR Lang)
{
    int Index = langListCombo_.FindStringExact(0, Lang);
    if (Index < 0) {
        return;
    }
    langListCombo_.SetCurSel(Index);
}

CString CLangSelect::getLanguage() const {
    return language_;
}
