#include "Utils.h"

MonitorEnumerator::MonitorEnumerator() {

}

BOOL MonitorEnumerator::DoEnumDisplayMonitors(HDC hdc, LPCRECT lprcClip) {
    return EnumDisplayMonitors(hdc, lprcClip, MonitorEnumProc, reinterpret_cast<LPARAM>(this));
}

size_t MonitorEnumerator::getCount() const {
    return monitors_.size();
}

MonitorEnumerator::MonitorInfo* MonitorEnumerator::getByIndex(size_t index) {
    if (index >= 0 && index < monitors_.size()) {
        return &monitors_[index];
    } else {
        return nullptr;
    }
}

BOOL CALLBACK MonitorEnumerator::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MonitorInfo info;
    MonitorEnumerator* pthis = reinterpret_cast<MonitorEnumerator*>(dwData);
    info.monitor = hMonitor;
    if (lprcMonitor) {
        info.rect = *lprcMonitor;
    }
    MONITORINFOEX mi;
    memset(&mi, 0, sizeof(mi));
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    info.deviceName = mi.szDevice;
    pthis->monitors_.push_back(info);

    return TRUE;
}

std::vector<MonitorEnumerator::MonitorInfo>::const_iterator MonitorEnumerator::begin() {
    return monitors_.begin();
    
}
std::vector<MonitorEnumerator::MonitorInfo>::const_iterator MonitorEnumerator::end() {
    return monitors_.end();
}