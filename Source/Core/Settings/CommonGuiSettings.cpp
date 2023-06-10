/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "CommonGuiSettings.h"

#include "Func/WinUtils.h"

const TCHAR CommonGuiSettings::VideoEngineDirectshow[] = _T("Directshow");
const TCHAR CommonGuiSettings::VideoEngineDirectshow2[] = _T("Directshow_2");
const TCHAR CommonGuiSettings::VideoEngineFFmpeg[] = _T("FFmpeg");
const TCHAR CommonGuiSettings::VideoEngineAuto[] = _T("Auto");

CommonGuiSettings::CommonGuiSettings() : BasicSettings()
{
    // Default values of settings

    ShowTrayIcon = false;
    ShowTrayIcon_changed = false;
    MaxThreads = 3;
    DeveloperMode = false;
    //temporaryServer.UseDefaultSettings = false;
    HistorySettings.EnableDownloading = true;
    HistorySettings.HistoryConverted = false;
}

CommonGuiSettings::~CommonGuiSettings() {
}

bool CommonGuiSettings::IsFFmpegAvailable() {
#ifndef IU_ENABLE_FFMPEG
    return false;
#else
    #if IU_FFMPEG_STANDALONE
        CString appFolder = WinUtils::GetAppFolder();
        return WinUtils::FileExists(appFolder + "avcodec-58.dll");
    #else
        return true;
    #endif
#endif
}
