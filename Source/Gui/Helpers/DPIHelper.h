#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <shellscalingapi.h>

#include "Func/Library.h"

class DPIHelper {
private:
   typedef UINT(WINAPI* GetDpiForWindowFunc)(HWND hwnd);
   typedef HRESULT(WINAPI* GetDpiForMonitorFunc)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
   typedef int(WINAPI* GetSystemMetricsForDpiFunc)(int nIndex, UINT dpi);

   static GetDpiForWindowFunc s_GetDpiForWindow;
   static GetDpiForMonitorFunc s_GetDpiForMonitor;
   static GetSystemMetricsForDpiFunc s_GetSystemMetricsForDpi;
   static bool s_initialized;
   
   static void Initialize();
   
public:
   static UINT GetDpiForWindow(HWND hwnd);

   static UINT GetDpiForDialog(HWND hwnd);

   // Helper function to get DPI for monitor (if available)
   static UINT GetDpiForMonitor(HMONITOR hMonitor);
   
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

   static int GetSystemMetricsForDpi(int nIndex, UINT dpi);
};
