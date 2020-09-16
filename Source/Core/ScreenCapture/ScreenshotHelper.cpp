#include "ScreenshotHelper.h"

#include <dwmapi.h>

#include "Func/WinUtils.h"

std::vector<RECT> ScreenshotHelper::monitorsRects_;

ScreenshotHelper::ScreenshotHelper() {
    if (!WinUtils::IsVistaOrLater()) {
        dwmIsCompEnabledFunc_ = dwmApiLibrary.GetProcAddress<DwmIsCompositionEnabled_Func>("DwmIsCompositionEnabled");
        dwmGetWindowAttributeFunc_ = dwmApiLibrary.GetProcAddress<DwmGetWindowAttribute_Func>("DwmGetWindowAttribute");
    }
}  

ScreenshotHelper& ScreenshotHelper::getInstance() {
    static ScreenshotHelper instance;
    return instance;
}

BOOL ScreenshotHelper::getActualWindowRect(HWND hWnd, RECT* res, bool MaximizedFix) const
{
    if (!WinUtils::IsVistaOrLater()) {
        return GetWindowRect(hWnd, res);
    }

    if (dwmIsCompEnabledFunc_) {
        BOOL isEnabled = false;
        if (S_OK == dwmIsCompEnabledFunc_(&isEnabled))
            if (isEnabled) {
                if (dwmGetWindowAttributeFunc_) {
                    if (S_OK == dwmGetWindowAttributeFunc_(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, res, sizeof(RECT))) {
                        if (MaximizedFix)
                            *res = maximizedWindowFix(hWnd, *res);
                        return TRUE;
                    }
                }
            }
    }

    return GetWindowRect(hWnd, res);
}

RECT ScreenshotHelper::maximizedWindowFix(HWND handle, RECT windowRect)
{
    RECT res = windowRect;
    if (isWindowMaximized(handle))
    {
        RECT screenRect = screenFromRectangle(windowRect);
        if (windowRect.left < screenRect.left)
        {
            windowRect.right -= (screenRect.left - windowRect.left) /** 2*/;
            windowRect.left = screenRect.left;
        }
        if (windowRect.top < screenRect.top)
        {
            windowRect.bottom -= (screenRect.top - windowRect.top) /** 2*/;
            windowRect.top = screenRect.top;
        }
        IntersectRect(&res, &windowRect, &screenRect);
        //  windowRect.Intersect(screenRect);
    }
    return res;
}


bool ScreenshotHelper::isWindowMaximized(HWND handle)
{
    WINDOWPLACEMENT wp;
    GetWindowPlacement(handle, &wp);
    return wp.showCmd == static_cast<UINT>(SW_MAXIMIZE);
}

RECT ScreenshotHelper::screenFromRectangle(RECT rc)
{
    monitorsRects_.clear();
    EnumDisplayMonitors(0, 0, monitorEnumProc, 0);
    int max = 0;
    size_t iMax = 0;
    for (size_t i = 0; i < monitorsRects_.size(); i++)
    {
        CRect Bounds = monitorsRects_[i];
        Bounds.IntersectRect(&Bounds, &rc);
        if (Bounds.Width() * Bounds.Height() > max)
        {
            max = Bounds.Width() * Bounds.Height();
            iMax = i;
        }
        // result.UnionRect(result,Bounds);
    }
    return monitorsRects_[iMax];
}

BOOL CALLBACK ScreenshotHelper::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    if (lprcMonitor)
    {
        monitorsRects_.push_back(*lprcMonitor);
    }
    return TRUE;
}

HRGN ScreenshotHelper::getWindowRegion(HWND wnd)
{
    RECT WndRect;
    getActualWindowRect(wnd, &WndRect );
    CRgn WindowRgn;
    WindowRgn.CreateRectRgnIndirect(&WndRect);
    if (::GetWindowRgn(wnd, WindowRgn) != ERROR)
    {
        // WindowRegion.GetRgnBox( &WindowRect);
        WindowRgn.OffsetRgn( WndRect.left, WndRect.top);
    }
    return WindowRgn.Detach();
}

HRGN ScreenshotHelper::getWindowVisibleRegion(HWND wnd)
{
    CRgn winReg;
    CRect result;
    if (!(GetWindowLong(wnd, GWL_STYLE) & WS_CHILD))
    {
        winReg = getWindowRegion(wnd);
        return winReg.Detach();
    }
    getActualWindowRect(wnd, &result);
    while (GetWindowLong(wnd, GWL_STYLE) & WS_CHILD)
    {
        wnd = GetParent(wnd);
        RECT rc;
        if (GetClientRect(wnd, &rc))
        {
            MapWindowPoints(wnd, 0, reinterpret_cast<POINT*>(&rc), 2);
            // parentRgn.CreateRectRgnIndirect(&rc);
        }
        result.IntersectRect(&result, &rc);
    }
    winReg.CreateRectRgnIndirect(&result);
    return winReg.Detach();
}