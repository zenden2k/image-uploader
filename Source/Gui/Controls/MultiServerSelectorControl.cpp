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

#include "MultiServerSelectorControl.h"

#include <boost/format.hpp>

#include "ServerSelectorControl.h"
#include "Gui/IconBitmapUtils.h"
#include "Core/ServiceLocator.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/Dialogs/ServerProfileGroupSelectDialog.h"

// CMultiServerSelectorControl
CMultiServerSelectorControl::CMultiServerSelectorControl(UploadEngineManager* uploadEngineManager, bool defaultServer, bool isChildWindow) {
    serversMask_ = CServerSelectorControl::smImageServers | CServerSelectorControl::smFileServers;
    uploadEngineManager_ = uploadEngineManager;
}

CMultiServerSelectorControl::~CMultiServerSelectorControl() {

}

void CMultiServerSelectorControl::TranslateUI() {
    TRC(IDC_CHOOSESERVERS, "Choose");
}
    
LRESULT CMultiServerSelectorControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TranslateUI();
    serverGroupboxFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_SERVERGROUPBOX));
    updateInfoLabel();
    return FALSE;  
}

LRESULT CMultiServerSelectorControl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

void CMultiServerSelectorControl::setTitle(CString title) {
    if (m_hWnd){
        SetDlgItemText(IDC_SERVERGROUPBOX, title);
    }
    title_ = title;
}

CString CMultiServerSelectorControl::getTitle() const {
    return title_;
}

void CMultiServerSelectorControl::setServerProfileGroup(const ServerProfileGroup& serverProfileGroup) {
    serverProfileGroup_ = serverProfileGroup;

    if (m_hWnd) {
        updateInfoLabel();
    }
}

const ServerProfileGroup& CMultiServerSelectorControl::serverProfileGroup() const {
    return serverProfileGroup_;
}

LRESULT CMultiServerSelectorControl::OnClickedChoose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CServerProfileGroupSelectDialog serverParamsDlg(uploadEngineManager_, serverProfileGroup_, serversMask_);
    if (serverParamsDlg.DoModal(m_hWnd) == IDOK) {
        serverProfileGroup_ = serverParamsDlg.serverProfileGroup();
        updateInfoLabel();
    }

    return 0;
}

void CMultiServerSelectorControl::serverChanged() {
    
}

void CMultiServerSelectorControl::setServersMask(int mask) {
    serversMask_ = mask;
}

void CMultiServerSelectorControl::updateInfoLabel() {
    std::wstring text;
    if (serverProfileGroup_.getCount() == 0) {
        text = TR("Server not chosen");
    } else if (serverProfileGroup_.getCount() == 1) {
        text = str(boost::wformat(TR("Selected server: %s")) % IuCoreUtils::Utf8ToWstring(serverProfileGroup_.getByIndex(0).serverName()));
    }
    else {
        text = str(boost::wformat(TR("Selected servers: %d")) % serverProfileGroup_.getCount());
    }

    SetDlgItemText(IDC_LABEL, text.c_str());
}