#include "DPIHelper.h"

// Static member definitions
DPIHelper::GetDpiForWindowFunc DPIHelper::s_GetDpiForWindow = nullptr;
DPIHelper::GetDpiForMonitorFunc DPIHelper::s_GetDpiForMonitor = nullptr;
bool DPIHelper::s_initialized = false;

void DPIHelper::Initialize() {
    if (s_initialized)
        return;

    // Try to load GetDpiForWindow from user32.dll (Windows 10 1607+)
    Library user32(L"user32.dll");
    if (user32) {
        s_GetDpiForWindow = user32.GetProcAddress<GetDpiForWindowFunc>("GetDpiForWindow");
    }

    // Try to load GetDpiForMonitor from shcore.dll (Windows 8.1+)
    Library shcore(L"shcore.dll");
    if (shcore) {
        s_GetDpiForMonitor = shcore.GetProcAddress<GetDpiForMonitorFunc>("GetDpiForMonitor");
    }

    s_initialized = true;
}

UINT DPIHelper::GetDpiForWindow(HWND hwnd) {
    Initialize();

    // Windows 10 1607+ - use GetDpiForWindow
    if (s_GetDpiForWindow) {
        return s_GetDpiForWindow(hwnd);
    }

    // Windows 8.1+ - use GetDpiForMonitor
    if (s_GetDpiForMonitor) {
        HMONITOR hMonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (hMonitor) {
            UINT dpiX = 96, dpiY = 96;
            if (SUCCEEDED(s_GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                return dpiX;
            }
        }
    }

    // Fallback for Windows 7 - use GetDeviceCaps
    // Get DC for window or screen
    HDC hdc = ::GetDC(hwnd);
    if (!hdc) {
        hdc = ::GetDC(nullptr); // Desktop DC
    }

    UINT dpi = 96; // Default DPI
    if (hdc) {
        dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
        ::ReleaseDC(hwnd, hdc);
    }

    return dpi;
}

UINT DPIHelper::GetDpiForDialog(HWND hwnd) {
    if (DPIHelper::IsPerMonitorDpiV2Supported()) {
        return DPIHelper::GetDpiForWindow(hwnd);
    } else {
        CClientDC dc(hwnd);
        return dc.GetDeviceCaps(LOGPIXELSX);
    }
}

UINT DPIHelper::GetDpiForMonitor(HMONITOR hMonitor) {
    Initialize();

    if (s_GetDpiForMonitor && hMonitor) {
        UINT dpiX = 96, dpiY = 96;
        if (SUCCEEDED(s_GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
            return dpiX;
        }
    }

    // Fallback
    HDC hdc = ::GetDC(nullptr);
    UINT dpi = 96;
    if (hdc) {
        dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
        ::ReleaseDC(nullptr, hdc);
    }
    return dpi;
}

int DPIHelper::GetSystemMetricsForDpi(int nIndex, UINT dpi) {
    // If dpi is 0 or standard DPI, return regular metrics
    if (dpi == 0) {
        return GetSystemMetrics(nIndex);
    }

    // Try to use new function for Windows 10 Anniversary Update (1607)+
    static Library user32(L"user32.dll");
    if (user32) {
        typedef int(WINAPI * GetSystemMetricsForDpiFunc)(int nIndex, UINT dpi);

        static GetSystemMetricsForDpiFunc pGetSystemMetricsForDpi = user32.GetProcAddress<GetSystemMetricsForDpiFunc>("GetSystemMetricsForDpi");

        if (pGetSystemMetricsForDpi) {
            return pGetSystemMetricsForDpi(nIndex, dpi);
        }
    }

    CClientDC dc(NULL);
    int systemDpi = dc.GetDeviceCaps(LOGPIXELSX);

    if (dpi == systemDpi) {
        return GetSystemMetrics(nIndex);
    }

    // Fallback for Windows 7/8/8.1 - manual scaling
    int baseValue = GetSystemMetrics(nIndex);

    // Some metrics should not be scaled
    switch (nIndex) {
    // Quantitative metrics - do not scale
    case SM_CMOUSEBUTTONS:
    case SM_CMONITORS:
    case SM_MOUSEPRESENT:
    case SM_MOUSEHORIZONTALWHEELPRESENT:
    case SM_MOUSEWHEELPRESENT:
    case SM_SWAPBUTTON:
    case SM_TABLETPC:
    case SM_MEDIACENTER:
    case SM_STARTER:
    case SM_SERVERR2:
    case SM_DIGITIZER:
    case SM_MAXIMUMTOUCHES:
        return baseValue;

    // Boolean metrics - do not scale
    case SM_DEBUG:
    case SM_DBCSENABLED:
    case SM_IMMENABLED:
    case SM_MIDEASTENABLED:
    case SM_NETWORK:
    case SM_PENWINDOWS:
    case SM_REMOTESESSION:
    case SM_SECURE:
    case SM_SLOWMACHINE:
    case SM_SHUTTINGDOWN:
        return baseValue;

    // Size metrics - scale them
    case SM_CXSCREEN:
    case SM_CYSCREEN:
    case SM_CXVSCROLL:
    case SM_CYHSCROLL:
    case SM_CYCAPTION:
    case SM_CXBORDER:
    case SM_CYBORDER:
    case SM_CXDLGFRAME:
    case SM_CYDLGFRAME:
    case SM_CYVTHUMB:
    case SM_CXHTHUMB:
    case SM_CXICON:
    case SM_CYICON:
    case SM_CXCURSOR:
    case SM_CYCURSOR:
    case SM_CYMENU:
    case SM_CXFULLSCREEN:
    case SM_CYFULLSCREEN:
    case SM_CYKANJIWINDOW:
    case SM_CXMINTRACK:
    case SM_CYMINTRACK:
    case SM_CXDOUBLECLK:
    case SM_CYDOUBLECLK:
    case SM_CXICONSPACING:
    case SM_CYICONSPACING:
    case SM_CXMAXIMIZED:
    case SM_CYMAXIMIZED:
    case SM_CXMAXTRACK:
    case SM_CYMAXTRACK:
    case SM_CXMENUCHECK:
    case SM_CYMENUCHECK:
    case SM_CXMINIMIZED:
    case SM_CYMINIMIZED:
    case SM_CXMINSPACING:
    case SM_CYMINSPACING:
    case SM_CXSIZE:
    case SM_CYSIZE:
    case SM_CXFRAME:
    case SM_CYFRAME:
    case SM_CXHSCROLL:
    case SM_CYVSCROLL:
    case SM_CXSMICON:
    case SM_CYSMICON:
    case SM_CYSMCAPTION:
    case SM_CXSMSIZE:
    case SM_CYSMSIZE:
    case SM_CXMENUSIZE:
    case SM_CYMENUSIZE:
    case SM_CXEDGE:
    case SM_CYEDGE:
    case SM_CXPADDEDBORDER:
        return MulDiv(baseValue, (int)dpi, systemDpi);

    // For unknown metrics try scaling
    default:
        // If value is greater than 1, it's probably a size metric
        if (baseValue > 1) {
            return MulDiv(baseValue, (int)dpi, systemDpi);
        }
        return baseValue;
    }
}
