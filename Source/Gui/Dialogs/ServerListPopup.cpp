/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

#include "ServerListPopup.h"

#include <strsafe.h>

#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Gui/IconBitmapUtils.h"
#include "Core/ServiceLocator.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Core/AbstractServerIconCache.h"
#include "Gui/Helpers/DPIHelper.h"
#include "Gui/Models/ServerListModel.h"

namespace {

constexpr TCHAR MENU_EXIT_NOTIFY[] = _T("MENU_EXIT_NOTIFY"), MENU_EXIT_COMMAND_ID[] = _T("MENU_EXIT_COMMAND_ID");

}

// CServerListPopup
CServerListPopup::CServerListPopup(CMyEngineList* engineList, WinServerIconCache* serverIconCache, bool isChildWindow)
    : engineList_(engineList)
    , serverListModel_(std::make_unique<ServerListModel>(engineList))
    , listView_(serverListModel_.get(), serverIconCache) {
    serversMask_ = smImageServers | smFileServers;

    iconBitmapUtils_ = std::make_unique<IconBitmapUtils>();
    isChildWindow_ = isChildWindow;
    hMyDlgTemplate_ = nullptr;
    isPopingUp_ = false;
    BasicSettings* settings = ServiceLocator::instance()->basicSettings();
}

CServerListPopup::~CServerListPopup()
{
    if (hMyDlgTemplate_) {
        GlobalFree(hMyDlgTemplate_);
    }
}

void CServerListPopup::TranslateUI() {
}

LRESULT CServerListPopup::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DlgResize_Init(true, true, 0); // resizable dialog without "griper"
    DoDataExchange(FALSE);
    listView_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    TranslateUI();

    //setTitle();

    createResources();
    updateServerList();

    return FALSE;
}

LRESULT CServerListPopup::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    return 0;
}

LRESULT CServerListPopup::OnDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    createResources();
    return 0;
}

void CServerListPopup::setTitle(CString title) {

}

CString CServerListPopup::getTitle() const {
    return {};
}

void CServerListPopup::setServerProfile(const ServerProfile& serverProfile) {
    serverProfile_ = serverProfile;
}

ServerProfile CServerListPopup::serverProfile() const {
    return serverProfile_;
}

void CServerListPopup::serverChanged() {

}

void CServerListPopup::setServersMask(int mask) {
    serversMask_ = mask;
}


void CServerListPopup::notifyChange()
{
    //::SendMessage(GetParent(), WM_SERVERSELECTCONTROL_CHANGE, reinterpret_cast<WPARAM>(m_hWnd), 0);
    if (onChangeCallback_)
    {
        onChangeCallback_(this);
    }
}

void CServerListPopup::notifyServerListChanged()
{
    //::SendMessage(GetParent(), WM_SERVERSELECTCONTROL_SERVERLIST_CHANGED, reinterpret_cast<WPARAM>(m_hWnd), 0);
}

void CServerListPopup::updateServerList()
{
    const int dpi = DPIHelper::GetDpiForDialog(m_hWnd);
    //auto iconCache = ServiceLocator::instance()->serverIconCache();
}

LRESULT CServerListPopup::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return MA_NOACTIVATE;
}

void CServerListPopup::createResources() {
    const int dpi = DPIHelper::GetDpiForDialog(m_hWnd);
    const int iconWidth = DPIHelper::GetSystemMetricsForDpi(SM_CXSMICON, dpi);
    const int iconHeight = DPIHelper::GetSystemMetricsForDpi(SM_CYSMICON, dpi);

}

void CServerListPopup::setOnChangeCallback(std::function<void(CServerListPopup*)> cb) {
    onChangeCallback_ = std::move(cb);
}

int CServerListPopup::showPopup(HWND parent, const RECT& anchorRect) {
    // Code from \Program Files\Microsoft SDKs\Windows\v7.1\Samples\winui\shell\legacysamples\fakemenu\fakemenu.cpp
    isChildWindow_ = false;
    if (Create(parent) == NULL) {
        ATLTRACE(_T("CServerListPopup dialog creation failed!  :( sorry\n"));
        return 0;
    }
    int nRet(-1);
    CRect windowRect;
    GetWindowRect(windowRect);
    int popupWidth = windowRect.Width();
    int popupHeight = windowRect.Height();
    
    int x = anchorRect.left;
    int y = anchorRect.bottom + 2;

    HMONITOR hMonitor = MonitorFromRect(windowRect, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi)) {
        // Check monitor work area boundaries
        RECT workArea = mi.rcWork;
        // Adjust horizontal position
        if (x + popupWidth > workArea.right) {
            // Doesn't fit on the right - shift left
            x = anchorRect.right - popupWidth;
            // If still doesn't fit, align to screen right edge
            if (x < workArea.left) {
                x = workArea.right - popupWidth;
            }
        }
        // Check that window doesn't go beyond left boundary
        if (x < workArea.left) {
            x = workArea.left;
        }
        // Adjust vertical position
        if (y + popupHeight > workArea.bottom) {
            // Doesn't fit below - show above the button
            y = anchorRect.top - popupHeight;
            // If doesn't fit above either
            if (y < workArea.top) {
                // Place next to button on the right
                if (anchorRect.right + popupWidth <= workArea.right) {
                    x = anchorRect.right;
                    y = anchorRect.top;
                }
                // Or on the left
                else if (anchorRect.left - popupWidth >= workArea.left) {
                    x = anchorRect.left - popupWidth;
                    y = anchorRect.top;
                }
                // As last resort - center in work area
                else {
                    x = workArea.left + (workArea.right - workArea.left - popupWidth) / 2;
                    y = workArea.top + (workArea.bottom - workArea.top - popupHeight) / 2;
                }
            }
        }
        // Final boundary check
        if (x < workArea.left) {
            x = workArea.left;
        }
        if (y < workArea.top) {
            y = workArea.top;
        }
        if (x + popupWidth > workArea.right) {
            x = workArea.right - popupWidth;
        }
        if (y + popupHeight > workArea.bottom) {
            y = workArea.bottom - popupHeight;
        }
    }

    SetWindowPos(0, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    ShowWindow(SW_SHOWNOACTIVATE);

    //BOOL bMenuDestroyed(FALSE);
    HWND hwndOwner = GetWindow(GW_OWNER);
    HWND hwndPopup = m_hWnd;
    // We want to receive all mouse messages, but since only the active
    // window can capture the mouse, we have to set the capture to our
    // owner window, and then steal the mouse messages out from under it.
    //::SetCapture(hwndOwner);

    // Go into a message loop that filters all the messages it receives
    // and route the interesting ones to the color picker window.
    MSG msg;
    POINT pt {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // If our owner stopped being the active window (e.g. the user
        // Alt+Tab'd to another window in the meantime), then stop.
        HWND hwndActive = GetActiveWindow();
        if (hwndActive != hwndPopup && hwndActive != hwndOwner && !::IsChild(hwndActive, hwndOwner) /*||
            GetCapture() != hwndOwner*/)
        {
            break;
        }


        bool isChildMessage = ::IsChild(hwndPopup, msg.hwnd)!=FALSE;
        if (!isChildMessage)
        {
            TCHAR className[MAX_PATH];
            if (::GetClassName(msg.hwnd, className, MAX_PATH) != 0)
            {
                isChildMessage = lstrcmp(className, _T("ComboLBox"))==0; // Style of ComboboxEx popup list box
            }
        }
        bool breakLoop = false;
        // At this point, we get to snoop at all input messages before
        // they get dispatched.  This allows us to route all input to our
        // popup window even if really belongs to somebody else.

        // All mouse messages are remunged and directed at our popup
        // menu. If the mouse message arrives as client coordinates, then
        // we have to convert it from the client coordinates of the original
        // target to the client coordinates of the new target.
        switch (msg.message)
        {
            // These mouse messages arrive in client coordinates, so in
            // addition to stealing the message, we also need to convert the
            // coordinates.

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            if (msg.hwnd == hwndOwner || ::IsChild(hwndOwner, msg.hwnd))
            {
                breakLoop = true;
                break;
            }
        case WM_MOUSEMOVE:
            if (!isChildMessage) {
                pt.x = (short)LOWORD(msg.lParam);
                pt.y = (short)HIWORD(msg.lParam);
                ::MapWindowPoints(msg.hwnd, hwndPopup, &pt, 1);
                msg.lParam = MAKELPARAM(pt.x, pt.y);
                msg.hwnd = hwndPopup;
            }
            break;

            // These mouse messages arrive in screen coordinates, so we just
            // need to steal the message.

        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCMBUTTONDBLCLK:
        case WM_SETCURSOR:
            if (msg.hwnd == hwndOwner || ::IsChild(hwndOwner, msg.hwnd))
            {
                breakLoop = true;
                break;
            }
        case WM_NCMOUSEMOVE:
            if (!isChildMessage) {
                msg.hwnd = hwndPopup;
            }
            break;

            // We need to steal all keyboard messages, too.
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
        case WM_DEADCHAR:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
        case WM_SYSDEADCHAR:
            if (!isChildMessage)
            {
                msg.hwnd = hwndPopup;
            }
            if (msg.wParam == VK_ESCAPE && (msg.hwnd == hwndPopup || ::IsChild(hwndPopup, msg.hwnd)))
            {
                breakLoop = true;
                break;
            }
            break;
        }
        if (breakLoop)
        {
            break;
        }
        if (!::IsDialogMessage(hwndPopup, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


        // If our owner stopped being the active window (e.g. the user
        // Alt+Tab'd to another window in the meantime), then stop.
        hwndActive = GetActiveWindow();
        if (hwndActive != hwndPopup && hwndActive != hwndOwner && !::IsChild(hwndActive, hwndOwner)/* ||
            GetCapture() != hwndOwner*/)
        {
            break;
        }
    }

    // Clean up the capture we created.
    //ReleaseCapture();

    isPopingUp_ = false;
    // If we got a WM_QUIT message, then re-post it so the caller's message
    // loop will see it.
    if (msg.message == WM_QUIT)
    {
        PostQuitMessage((int)msg.wParam);
    }


    //if (!bMenuDestroyed)
    DestroyWindow();

    return nRet;
}

bool CServerListPopup::exitPopup(int nCommandId)
{
    BOOL bRet = SetProp(m_hWnd, MENU_EXIT_NOTIFY, (HANDLE)1);
    SetProp(m_hWnd, MENU_EXIT_COMMAND_ID, reinterpret_cast<HANDLE>(static_cast<INT_PTR>(nCommandId)));
    return bRet !=FALSE;
}

DLGTEMPLATE* CServerListPopup::GetTemplate()
{
    HINSTANCE hInst = GetModuleHandle(0);
    HRSRC res = FindResource(hInst, MAKEINTRESOURCE(IDD), RT_DIALOG);
    DLGTEMPLATE* dit = reinterpret_cast<DLGTEMPLATE*>(LockResource(LoadResource(hInst, res)));

    unsigned long sizeDlg = ::SizeofResource(hInst, res);
    hMyDlgTemplate_ = ::GlobalAlloc(GPTR, sizeDlg);
    auto pMyDlgTemplate = reinterpret_cast<ATL::_DialogSplitHelper::DLGTEMPLATEEX*>(::GlobalLock(hMyDlgTemplate_));
    ::memcpy(pMyDlgTemplate, dit, sizeDlg);

    if (isChildWindow_)
    {
        //pMyDlgTemplate->style = pMyDlgTemplate->style & ~ WS_POPUP;
        pMyDlgTemplate->style = pMyDlgTemplate->style | WS_CHILD;
    }
    else
    {
        pMyDlgTemplate->style -= WS_CHILD;
        pMyDlgTemplate->style -= DS_CONTROL;
        pMyDlgTemplate->exStyle |= WS_EX_NOACTIVATE |
            WS_EX_TOOLWINDOW |      // So it doesn't show up in taskbar
            WS_EX_DLGMODALFRAME |   // Get the edges right
            WS_EX_WINDOWEDGE /*|
            WS_EX_TOPMOST*/; // with this style window is overlapping modal dialogs (server params dialog, add ftp server, etc...)

        pMyDlgTemplate->style = pMyDlgTemplate->style | WS_POPUP | WS_BORDER/* | WS_CAPTION*/ ;
    }
    if (ServiceLocator::instance()->translator()->isRTL()) {
        pMyDlgTemplate->exStyle |= WS_EX_LAYOUTRTL | WS_EX_RTLREADING;
    }
    return reinterpret_cast<DLGTEMPLATE*>(pMyDlgTemplate);
}

LRESULT CServerListPopup::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (!isChildWindow_) {
        // Disable wizard window when a modal window is shown
        ::EnableWindow(GetParent(), wParam);
    }

    return 0;
}
