#ifndef IU_CORE_SCREENCAPTURE_SCREENSHOTHELPER_H
#define IU_CORE_SCREENCAPTURE_SCREENSHOTHELPER_H

#include "Func/Library.h"

namespace ScreenshotHelper {

BOOL getActualWindowRect(HWND hWnd, RECT* res, bool maximizedFix = true);
RECT maximizedWindowFix(HWND handle, RECT windowRect);
bool isWindowMaximized(HWND handle);
RECT screenFromRectangle(RECT rc);
HRGN getWindowRegion(HWND wnd);
HRGN getWindowVisibleRegion(HWND wnd);
BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

};

#endif