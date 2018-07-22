#ifndef IU_CORE_SCREENCAPTURE_UTILS
#define IU_CORE_SCREENCAPTURE_UTILS

#include <windows.h>
#include <vector>

class MonitorEnumerator {

public:
    MonitorEnumerator();
    BOOL DoEnumDisplayMonitors(HDC hdc, LPCRECT lprcClip);
    size_t getCount() const;

    struct MonitorInfo {
        HMONITOR monitor;
        CRect rect;
        CString deviceName;
    };

    std::vector<MonitorInfo>::const_iterator begin();
    std::vector<MonitorInfo>::const_iterator end();

    MonitorInfo* getByIndex(size_t index);
protected:
    static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    std::vector<MonitorInfo> monitors_;
};

#endif