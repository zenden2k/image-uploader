#ifndef IU_CORE_SCREENCAPTURE_SCREENSHOTHELPER_H
#define IU_CORE_SCREENCAPTURE_SCREENSHOTHELPER_H

#include "Func/Library.h"

class ScreenshotHelper
{
    public:
        static ScreenshotHelper& getInstance();
        ScreenshotHelper(ScreenshotHelper const&) = delete;
        void operator=(ScreenshotHelper const&)  = delete;

        BOOL getActualWindowRect(HWND hWnd, RECT* res, bool MaximizedFix = true) const;
        static RECT maximizedWindowFix(HWND handle, RECT windowRect);
        static bool isWindowMaximized(HWND handle);
        static RECT screenFromRectangle(RECT rc);
        HRGN getWindowRegion(HWND wnd);
        HRGN getWindowVisibleRegion(HWND wnd);
     private:
        ScreenshotHelper();
        Library dwmApiLibrary{ L"dwmapi.dll" };

        typedef HRESULT (WINAPI * DwmGetWindowAttribute_Func)(HWND, DWORD, PVOID, DWORD);
        typedef HRESULT (WINAPI * DwmIsCompositionEnabled_Func)(BOOL*);

        DwmIsCompositionEnabled_Func dwmIsCompEnabledFunc_ = nullptr;
        DwmGetWindowAttribute_Func dwmGetWindowAttributeFunc_ = nullptr;
        static std::vector<RECT> monitorsRects_;
        static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

};

#endif