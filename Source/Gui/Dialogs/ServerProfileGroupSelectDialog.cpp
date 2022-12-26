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

#include "ServerProfileGroupSelectDialog.h"

#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/Controls/IconButton.h"
#include "Gui/Controls/ServerSelectorControl.h"

constexpr int SELECTOR_HEIGHT = 70;
constexpr int SELECTOR_WIDTH = 400;
constexpr int BUTTON_WIDTH = 30;
constexpr int BUTTON_HEIGHT = 30;

CServerProfileGroupSelectDialog::CServerProfileGroupSelectDialog(UploadEngineManager* uploadEngineManager, ServerProfileGroup group):
    profileGroup_(std::move(group)),
    uploadEngineManager_(uploadEngineManager) {
    
}

CServerProfileGroupSelectDialog::~CServerProfileGroupSelectDialog() {
    for(auto& it: serverSelectors_) {
        delete it;
    }

    for (auto& it : deleteButtons_) {
        delete it;
    }
}

LRESULT CServerProfileGroupSelectDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetWindowText(TR("Choose servers"));
    TRC(IDC_ADDBUTTON, "Add");
    TRC(IDCANCEL, "Cancel");

    DlgResize_Init(false, true, 0); // resizable dialog without "griper"

    icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);

    CWindowDC hdc(m_hWnd);
   
    for (auto& server: profileGroup_.getItems()) {
        addSelector(server, hdc);
    }
    return 1;
}

LRESULT CServerProfileGroupSelectDialog::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    profileGroup_ = {};
    for (auto& it : serverSelectors_) {
        profileGroup_.addItem(it->serverProfile());
    }

    return EndDialog(wID);
}

LRESULT CServerProfileGroupSelectDialog::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

const ServerProfileGroup& CServerProfileGroupSelectDialog::serverProfileGroup() const {
    return profileGroup_;
}

void CServerProfileGroupSelectDialog::addSelector(const ServerProfile& profile, HDC dc) {
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;

    CServerSelectorControl* control = new CServerSelectorControl(uploadEngineManager_);
    control->setServersMask(CServerSelectorControl::smImageServers | CServerSelectorControl::smFileServers);
    //control->setShowImageProcessingParams(false);

    CRect rc(dpiScaleX * 5, serverSelectors_.size() * dpiScaleY * SELECTOR_HEIGHT, dpiScaleX * SELECTOR_WIDTH, (serverSelectors_.size() + 1) * dpiScaleY * SELECTOR_HEIGHT);
    control->Create(m_hWnd, rc);
    control->ShowWindow(SW_SHOW);
    control->SetWindowPos(GetDlgItem(IDC_FILESERVERPLACEHOLDER), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
    control->setServerProfile(profile);
    control->setTitle(TR("Server"));
  

    CRect rcButton(dpiScaleX * (SELECTOR_WIDTH + 5), serverSelectors_.size() * dpiScaleY * SELECTOR_HEIGHT, 
        dpiScaleX * (SELECTOR_WIDTH + 5 + BUTTON_WIDTH) , (serverSelectors_.size()) * dpiScaleY * (SELECTOR_HEIGHT + BUTTON_HEIGHT));

    CIconButton* deleteButton = new CIconButton;
    deleteButton->Create(m_hWnd, rcButton, TR("Delete"), BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_TABSTOP);

    serverSelectors_.push_back(control);
    deleteButtons_.push_back(deleteButton);
}

LRESULT CServerProfileGroupSelectDialog::OnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CWindowDC hdc(m_hWnd);

    addSelector({}, hdc);
    
    return 0;
}