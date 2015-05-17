#include "CommonGuiSettings.h"

#include "Func/WinUtils.h"

const TCHAR CommonGuiSettings::VideoEngineDirectshow[] = _T("Directshow");
const TCHAR CommonGuiSettings::VideoEngineFFmpeg[] = _T("FFmpeg");
const TCHAR CommonGuiSettings::VideoEngineAuto[] = _T("Auto");

CommonGuiSettings::CommonGuiSettings() : BasicSettings()
{
    // Default values of settings

    ConnectionSettings.UseProxy = FALSE;
    ConnectionSettings.ProxyPort = 0;
    ConnectionSettings.NeedsAuth = false;
    ConnectionSettings.ProxyType = 0;
    ShowTrayIcon = false;
    ShowTrayIcon_changed = false;
    MaxThreads = 3;
    DeveloperMode = false;
}

CommonGuiSettings::~CommonGuiSettings() {
}




bool CommonGuiSettings::IsFFmpegAvailable() {
    CString appFolder = WinUtils::GetAppFolder();
    return WinUtils::FileExists(appFolder + "avcodec-56.dll") != FALSE;
}
