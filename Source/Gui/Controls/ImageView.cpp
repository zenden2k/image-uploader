/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "atlheaders.h"
#include "ImageView.h"
#include "Core/Images/Utils.h"

// CImageView
CImageView::CImageView()
{
    callback_ = nullptr;
    currentParent_ = nullptr;
}

CImageView::~CImageView()
{ 
}

LRESULT CImageView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rc = {380, 37, 636, 240};
    Img.Create(m_hWnd, rc);
    Img.HideParent = true;
    SetFocus();
    return 0;  // Let the system set the focus
}

LRESULT CImageView::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return ShowWindow(SW_HIDE);
}

LRESULT CImageView::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    switch (wParam) {
        case kDblClickTimer:
        {
            Img.HideParent = true;
            KillTimer(kDblClickTimer);
        }
        break;
    }
    return 0;
}

LRESULT CImageView::OnKillFocus(HWND hwndNewFocus)
{
    return ShowWindow(SW_HIDE);
}

bool CImageView::ViewImage(const CImageViewItem& item, HWND Parent){
    currentItem_ = item;
    std::unique_ptr<Gdiplus::Bitmap> img(LoadImageFromFileExtended(item.fileName));
    
    if (img) {
        float width = static_cast<float>(GetSystemMetrics(SM_CXSCREEN) - 12);
        float height = static_cast<float>(GetSystemMetrics(SM_CYSCREEN) - 50);
        float imgwidth = static_cast<float>(img->GetWidth());
        float imgheight = static_cast<float>(img->GetHeight());
        float newheight, newwidth;
        newwidth = imgwidth;
        newheight = imgheight;

        if (newwidth > width || newheight > height) {
            float k1 = imgwidth / imgheight;
            float k2 = width / height;
            if (k1 >= k2) {
                newwidth = width;
                newheight = newwidth / imgwidth*imgheight;
            } else {
                newheight = height;
                newwidth = (newheight / imgheight)*imgwidth;
            }
        }

        int realwidth = static_cast<int>(newwidth + 2);
        int realheight = static_cast<int>(newheight + 2);

        if (realwidth < 200) realwidth = 200;
        if (realheight < 180) realheight = 180;
        //ShowWindow(SW_HIDE);
        if (realwidth && realheight) {
            //MoveWindow(0, 0, realwidth, realheight);
            Img.MoveWindow(0, 0, realwidth, realheight);
            Img.LoadImage(item.fileName, img.get());
        }
        currentParent_ = Parent;
        MyCenterWindow(Parent, realwidth, realheight);
        if (!IsWindowVisible()) {
            Img.HideParent = false;
            SetTimer(kDblClickTimer, 300);
        }
        ShowWindow(SW_SHOW);
        SetForegroundWindow(m_hWnd);
    } 
    
    return false;
}

LRESULT CImageView::OnActivate(UINT state, BOOL fMinimized, HWND hwndActDeact)
{
    if (state == WA_INACTIVE) 
        return ShowWindow(SW_HIDE);
    return 0;
}

void CImageView::MyCenterWindow(HWND hWndCenter, int width, int height) {
    ATLASSERT(::IsWindow(m_hWnd));

    // determine owner window to center against
    DWORD dwStyle = GetStyle();

    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    if (!(dwStyle & WS_CHILD)) {
        // don't center against invisible or minimized windows
        if (hWndCenter != NULL) {
            DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
            if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
                hWndCenter = NULL;
        }

        // center within screen coordinates
        HMONITOR hMonitor = NULL;
        if (hWndCenter != NULL) {
            hMonitor = ::MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST);
        } else {
            hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        }

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        BOOL bResult = ::GetMonitorInfo(hMonitor, &minfo);

        rcArea = minfo.rcWork;

        if (hWndCenter == NULL)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);
    } else {
        // center within parent client coordinates
        hWndParent = ::GetParent(m_hWnd);
        ATLASSERT(::IsWindow(hWndParent));

        ::GetClientRect(hWndParent, &rcArea);
        ATLASSERT(::IsWindow(hWndCenter));
        ::GetClientRect(hWndCenter, &rcCenter);
        ::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    int DlgWidth = width;
    int DlgHeight = height;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if (xLeft + DlgWidth > rcArea.right)
        xLeft = rcArea.right - DlgWidth;
    if (xLeft < rcArea.left)
        xLeft = rcArea.left;

    if (yTop + DlgHeight > rcArea.bottom)
        yTop = rcArea.bottom - DlgHeight;
    if (yTop < rcArea.top)
        yTop = rcArea.top;

    // map screen coordinates to child coordinates
    ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CImageView::OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags)
{
    switch (vk) {
        case VK_RIGHT:
        {
            if (callback_) {
                CImageViewItem item = callback_->getNextImgViewItem(currentItem_);
                if (!item.fileName.IsEmpty()) {
                    ViewImage(item, currentParent_);
                }
            }
        }
        break;  
        case VK_LEFT:
        {
            if (callback_) {
                CImageViewItem item = callback_->getPrevImgViewItem(currentItem_);
                if (!item.fileName.IsEmpty()) {
                    ViewImage(item, currentParent_);
                }
            }
        }
        break;
        case VK_RETURN:
        case VK_ESCAPE:
            ShowWindow(SW_HIDE);
    }
    return 0;
}

void CImageView::setCallback(CImageViewCallback* callback) {
    callback_ = callback;
}