#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <shellscalingapi.h>

#include "Func/Library.h"

class DPIHelper {
private:
   typedef UINT(WINAPI* GetDpiForWindowFunc)(HWND hwnd);
   typedef HRESULT(WINAPI* GetDpiForMonitorFunc)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
   
   static GetDpiForWindowFunc s_GetDpiForWindow;
   static GetDpiForMonitorFunc s_GetDpiForMonitor;
   static bool s_initialized;
   
   static void Initialize() {
       if (s_initialized) return;
       
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
   
public:
   static UINT GetDpiForWindow(HWND hwnd) {
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
   
   // Helper function to get DPI for monitor (if available)
   static UINT GetDpiForMonitor(HMONITOR hMonitor) {
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
   
   // Check if Per-Monitor DPI is supported
   static bool IsPerMonitorDpiSupported() {
       Initialize();
       return s_GetDpiForMonitor != nullptr;
   }
   
   // Check if Per-Monitor DPI v2 is supported
   static bool IsPerMonitorDpiV2Supported() {
       Initialize();
       return s_GetDpiForWindow != nullptr;
   }
};
