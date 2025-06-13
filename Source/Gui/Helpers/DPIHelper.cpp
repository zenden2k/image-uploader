#include "DPIHelper.h"

// Static member definitions
DPIHelper::GetDpiForWindowFunc DPIHelper::s_GetDpiForWindow = nullptr;
DPIHelper::GetDpiForMonitorFunc DPIHelper::s_GetDpiForMonitor = nullptr;
bool DPIHelper::s_initialized = false;
