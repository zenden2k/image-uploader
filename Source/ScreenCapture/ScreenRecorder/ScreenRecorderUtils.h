#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
#include <dxgi.h>

bool StringToGUID(const std::string& str, GUID& guid);
CString GetMessageForHresult(HRESULT hr);
CComPtr<IDXGIAdapter1> GetAdapterByIndex(int index);
CComPtr<IDXGIAdapter1> GetAdapterForMonitor(HMONITOR hMonitor, UINT* adapterIndex = nullptr, UINT* outputIndex = nullptr);

#endif
