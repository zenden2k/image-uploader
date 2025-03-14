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

#include "SettingsPage.h"

#include <Uxtheme.h>
#include "Gui/Dialogs/WizardDlg.h"

CSettingsPage::CSettingsPage() {
}

bool CSettingsPage::onShow() {
    return true;
}

bool CSettingsPage::onHide() {
    return false;
}    

void CSettingsPage::addError(CString message, HWND control /*= NULL*/) {
    errors_.emplace_back(message, control);
}

bool CSettingsPage::apply() {
    return true;
}

bool CSettingsPage::validate() {
    return errors_.empty();
}

void CSettingsPage::clearErrors() {
    errors_.clear();
}

const std::vector<ValidationError>& CSettingsPage::errors() const {
    return errors_;
}

void CSettingsPage::fixBackground() const {
    EnableThemeDialogTexture(PageWnd, ETDT_ENABLETAB);
}
