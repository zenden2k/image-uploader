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

LRESULT CMyPanel::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    g_scrollY = 0;
    RECT rc = { 0 };
    GetClientRect(&rc);
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = scrollSize_;
    si.nPage = (rc.bottom - rc.top);
    si.nPos = 0;
    si.nTrackPos = 0;
    SetScrollInfo(SB_VERT, &si, true);
    return 0;
}

void CMyPanel::setScrollHeight(int height) {
    scrollSize_ = height;
    if (m_hWnd) {
        RECT rc = { 0 };
        GetClientRect(&rc);
        SCROLLINFO si = { 0 };
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        si.nMin = 0;
        si.nMax = scrollSize_;
        si.nPage = (rc.bottom - rc.top);
        si.nPos = 0;
        si.nTrackPos = 0;
        SetScrollInfo(SB_VERT, &si, true);
    }
}

LRESULT CMyPanel::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    auto action = LOWORD(wParam);
    HWND hScroll = reinterpret_cast<HWND>(lParam);
    int pos = -1;
    if (action == SB_THUMBPOSITION || action == SB_THUMBTRACK) {
        pos = HIWORD(wParam);
    }
    else if (action == SB_LINEDOWN) {
        pos = g_scrollY + 30;
    }
    else if (action == SB_LINEUP) {
        pos = g_scrollY - 30;
    }
    if (pos == -1) {
        return 0;
    }

    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    si.nPos = pos;
    si.nTrackPos = 0;
    SetScrollInfo( SB_VERT, &si, true);
    GetScrollInfo( SB_VERT, &si);
    pos = si.nPos;
    POINT pt;
    pt.x = 0;
    pt.y = pos - g_scrollY;
    auto hdc = GetDC();
    LPtoDP(hdc, &pt, 1);
    ReleaseDC( hdc);
    ScrollWindow(0, -pt.y, NULL, NULL);
    g_scrollY = pos;
    return 0;
}

LRESULT CMyPanel::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    CRect rc;
    GetClientRect(rc);
    CPaintDC dc(m_hWnd);
    dc.FillRect(rc, COLOR_BTNFACE);
    return 0;
}

LRESULT CMyPanel::OnClickedDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    ::SendMessage(GetParent(), WM_COMMAND, wID, reinterpret_cast<LPARAM>(hWndCtl));
    return 0;
}

constexpr int SELECTOR_HEIGHT = 70;
constexpr int SELECTOR_WIDTH = 380;
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
    CenterWindow(GetParent());
    //this->InitializeScroll(m_hWnd);

    RECT rect = {};
    this->GetClientRect(&rect);

    CWindowDC dc(m_hWnd);
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;

    SetWindowText(TR("Choose servers"));
    TRC(IDC_ADDBUTTON, "Add");
    TRC(IDCANCEL, "Cancel");

    //DlgResize_Init(false, true, 0); // resizable dialog without "griper"

    icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);

    deleteIcon_ = static_cast<HICON>(LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONDELETE), IMAGE_ICON, static_cast<int>(16 * dpiScaleX),
        static_cast<int>(16 * dpiScaleY), 0));

    rect.bottom -= 70 * dpiScaleY;
    rect.right -= 130 * dpiScaleX;
    panel_.Create(m_hWnd, rect, nullptr, WS_VISIBLE | WS_CHILD |WS_CLIPCHILDREN, WS_EX_CONTROLPARENT);
    //panel_.SetDlgCtrlID(IDC_SCROLLCONTAINER_ID);
    for (auto& server: profileGroup_.getItems()) {
        addSelector(server, dc);
    }
    updateScroll();
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
    

    CServerSelectorControl* control = new CServerSelectorControl(uploadEngineManager_);
    control->setServersMask(CServerSelectorControl::smImageServers | CServerSelectorControl::smFileServers);
    //control->setShowImageProcessingParams(false);

    

    HWND parent = /*scrollContainer_.m_hWnd ? scrollContainer_.m_hWnd :*/panel_.m_hWnd;
    CRect rc(0, 0, 100, 100);
    control->Create(parent, rc);
    control->ShowWindow(SW_SHOW);
    
    control->SetWindowPos(GetDlgItem(IDC_FILESERVERPLACEHOLDER), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
    control->setServerProfile(profile);
    control->setTitle(TR("Server"));

    serverSelectors_.push_back(control);

    int index = serverSelectors_.size() - 1;

    auto* deleteButton = new CButton;
    deleteButton->Create(parent, rc, TR("Delete"), BS_ICON|BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP);
    deleteButton->SetIcon(deleteIcon_);
    deleteButton->SetDlgCtrlID(IDC_DELETESERVER_FIRST_ID + index);
    deleteButtons_.push_back(deleteButton);
    updateSelectorPos(index, dc);

}

void CServerProfileGroupSelectDialog::updateSelectorPos(size_t index, HDC dc) {
    auto* selector = serverSelectors_[index];
    auto* deleteButton = deleteButtons_[index];
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;
    int selectorTop = index * dpiScaleY * (SELECTOR_HEIGHT);
    CRect rc(dpiScaleX * 5, selectorTop, dpiScaleX * SELECTOR_WIDTH, selectorTop + dpiScaleY * SELECTOR_HEIGHT);

    int buttonTop = selectorTop + dpiScaleY * 10;

    selector->SetWindowPos(nullptr, rc, SWP_NOZORDER);
    CRect rcButton(dpiScaleX * (SELECTOR_WIDTH + 5), buttonTop,
        dpiScaleX * (SELECTOR_WIDTH + 5 + BUTTON_WIDTH), buttonTop + dpiScaleY * BUTTON_HEIGHT);
    deleteButton->SetWindowPos(nullptr, rcButton, SWP_NOZORDER);
}

LRESULT CServerProfileGroupSelectDialog::OnClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CWindowDC hdc(m_hWnd);

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
    CWindowDC dc(m_hWnd);
    for(size_t i = buttonIndex; i < serverSelectors_.size(); i++) {
        updateSelectorPos(i, dc);
    }
    return 0;
}

void CServerProfileGroupSelectDialog::updateScroll() {
    CWindowDC dc(m_hWnd);
    float dpiScaleX = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;

    int selectorTop = serverSelectors_.size() * dpiScaleY * (SELECTOR_HEIGHT);
    panel_.setScrollHeight(selectorTop);
}