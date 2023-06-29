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
#include "Gui/Controls/ServerSelectorControl.h"

void CMyPanel::setScrollDimensions(int width, int height) {
    if (m_hWnd) {
        SIZE sz = { width, height };
        SetScrollSize(sz, TRUE, false);
    }
}

void CMyPanel::DoPaint(CDCHandle dc) {
    CRect rc;
    GetClientRect(rc);
    dc.FillRect(rc, COLOR_BTNFACE);
}

LRESULT CMyPanel::OnClickedDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    ::SendMessage(GetParent(), WM_COMMAND, wID, reinterpret_cast<LPARAM>(hWndCtl));
    return 0;
}

LRESULT CMyPanel::OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    ::SendMessage(GetParent(), uMsg, wParam, lParam);
    return 0;
}

// Dimensions in pixels
constexpr int BUTTON_WIDTH = 30;
constexpr int BUTTON_HEIGHT = 30;
constexpr int BUTTON_MARGIN = 10;

// Size of the dialog IDD_SERVERSELECTORCONTROL in dialog units 
const int CHILD_DIALOG_WIDTH = 260;
const int CHILD_DIALOG_HEIGHT = 46;

CServerProfileGroupSelectDialog::CServerProfileGroupSelectDialog(UploadEngineManager* uploadEngineManager, ServerProfileGroup group, int serverMask):
    profileGroup_(std::move(group)),
    uploadEngineManager_(uploadEngineManager),
    serverMask_(serverMask)
{
}

CServerProfileGroupSelectDialog::~CServerProfileGroupSelectDialog() {
}

LRESULT CServerProfileGroupSelectDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    //this->InitializeScroll(m_hWnd);

    RECT rect = {};
    this->GetClientRect(&rect);

    CClientDC dc(m_hWnd);
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;

    SetWindowText(TR("Choose servers"));
    TRC(IDC_ADDBUTTON, "Add");
    TRC(IDCANCEL, "Cancel");
    TRC(IDOK, "OK");

    //DlgResize_Init(false, true, 0); // resizable dialog without "griper"

    icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);

    int iconWidth = ::GetSystemMetrics(SM_CXSMICON);
    int iconHeight = ::GetSystemMetrics(SM_CYSMICON);

    deleteIcon_.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICONDELETEBIG), iconWidth, iconHeight);

    CRect dlgRect(0, 0, CHILD_DIALOG_WIDTH, CHILD_DIALOG_HEIGHT);
    MapDialogRect(&dlgRect);
    rect.bottom -= 50 * dpiScaleY;
    rect.right = dlgRect.Width() + (40 + BUTTON_WIDTH + BUTTON_MARGIN) * dpiScaleX;
    panel_.Create(m_hWnd, rect, nullptr, WS_VISIBLE | WS_CHILD, WS_EX_CONTROLPARENT);
    //panel_.SetDlgCtrlID(IDC_SCROLLCONTAINER_ID);
    for (auto& server: profileGroup_.getItems()) {
        addSelector(server, dc);
    }
    updateScroll();
    return 1;
}

LRESULT CServerProfileGroupSelectDialog::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    for (auto& item: serverSelectors_) {
        if (item->serverProfile().serverName().empty()) {
            continue;
        }
        if (!item->isAccountChosen()) {
            CString message;
            message.Format(TR("You have not selected account for server \"%s\""), IuCoreUtils::Utf8ToWstring(item->serverProfile().serverName()).c_str());
            GuiTools::LocalizedMessageBox(m_hWnd, message, TR("Error"), MB_ICONERROR);
            return false;
        }
    }

    profileGroup_ = {};
    for (auto& it : serverSelectors_) {
        if (!it->serverProfile().isNull()) {
            profileGroup_.addItem(it->serverProfile());
        }
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
    auto control = std::make_unique<CServerSelectorControl>(uploadEngineManager_);
    control->setShowEmptyItem(true);
    control->setServersMask(serverMask_);
    //control->setShowImageProcessingParams(false);

    HWND parent = /*scrollContainer_.m_hWnd ? scrollContainer_.m_hWnd :*/panel_.m_hWnd;
    CRect rc(0, 0, 100, 100);
    control->Create(parent, rc);
    control->ShowWindow(SW_SHOW);
    
    control->SetWindowPos(GetDlgItem(IDC_FILESERVERPLACEHOLDER), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
    control->setServerProfile(profile);
    control->setTitle(TR("Server"));

    serverSelectors_.push_back(std::move(control));

    int index = serverSelectors_.size() - 1;

    auto deleteButton = std::make_unique<CButton>();
    deleteButton->Create(parent, rc, TR("Delete"), BS_ICON|BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP);
    deleteButton->SetIcon(deleteIcon_);
    deleteButton->SetDlgCtrlID(IDC_DELETESERVER_FIRST_ID + index);
    deleteButtons_.push_back(std::move(deleteButton));
    updateSelectorPos(index, dc);
}

void CServerProfileGroupSelectDialog::updateSelectorPos(size_t index, HDC dc) {
    auto& selector = serverSelectors_[index];
    auto& deleteButton = deleteButtons_[index];
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;

    CRect dlgRect(0, 0, CHILD_DIALOG_WIDTH, CHILD_DIALOG_HEIGHT);
    MapDialogRect(&dlgRect);
    POINT pt;
    panel_.GetScrollOffset(pt);
    int selectorTop = index * dlgRect.Height() - pt.y;

    CRect rc(dpiScaleX * 5, selectorTop, dlgRect.Width() + dpiScaleX * 5, selectorTop + dlgRect.Height());

    int buttonTop = selectorTop + dpiScaleY * 10;

    selector->SetWindowPos(nullptr, rc, SWP_NOZORDER);
    CRect rcButton(dlgRect.Width() + dpiScaleX *  BUTTON_MARGIN, buttonTop,
        dlgRect.Width() + dpiScaleX * ( BUTTON_MARGIN + BUTTON_WIDTH), buttonTop + dpiScaleY * BUTTON_HEIGHT);
    deleteButton->SetWindowPos(nullptr, rcButton, SWP_NOZORDER);
    deleteButton->SetDlgCtrlID(IDC_DELETESERVER_FIRST_ID + index);
}

LRESULT CServerProfileGroupSelectDialog::OnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CClientDC hdc(m_hWnd);

    addSelector({}, hdc);
    updateScroll();
    
    return 0;
}

LRESULT CServerProfileGroupSelectDialog::OnClickedDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    unsigned int buttonIndex = wID - IDC_DELETESERVER_FIRST_ID;
    /*if (serverSelectors_.size() < 2) {
        return 0;
    }*/
    assert(buttonIndex >= 0 && buttonIndex < serverSelectors_.size());
    serverSelectors_[buttonIndex]->DestroyWindow();
    serverSelectors_.erase(serverSelectors_.begin() + buttonIndex);
    deleteButtons_[buttonIndex]->DestroyWindow();
    deleteButtons_.erase(deleteButtons_.begin() + buttonIndex);
    CClientDC dc(m_hWnd);
    for(size_t i = 0 /*buttonIndex*/; i < serverSelectors_.size(); i++) {
        updateSelectorPos(i, dc);
    }
    updateScroll();
    return 0;
}

void CServerProfileGroupSelectDialog::updateScroll() {
    CClientDC dc(m_hWnd);
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;
    CRect dlgRect(0, 0, CHILD_DIALOG_WIDTH, CHILD_DIALOG_HEIGHT);
    MapDialogRect(&dlgRect);

    int scrollHeight = serverSelectors_.size() * dlgRect.Height();
    
    int scrollWidth = dlgRect.Width() + (BUTTON_WIDTH + BUTTON_MARGIN) * dpiScaleX;

    // Add 1 pixel to avoid debug assertion in WTL
    panel_.setScrollDimensions(1+scrollWidth, 1+scrollHeight);
}

LRESULT CServerProfileGroupSelectDialog::OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    for (auto& it : serverSelectors_) {
        it->updateServerList();
    }
    return 0;
}
