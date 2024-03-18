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

#include "Func/MyEngineList.h"

#include "Core/Settings/BasicSettings.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"
#include "Core/ServiceLocator.h"

char CMyEngineList::DefaultServer[] = "default";

char CMyEngineList::RandomServer[]  = "random";

CMyEngineList::CMyEngineList(TaskDispatcher* taskDispatcher): iconBitmapUtils_(std::make_unique<IconBitmapUtils>()),
    taskDispatcher_(taskDispatcher)
{
}

CMyEngineList::~CMyEngineList()
{
    for (const auto& it: serverIcons_) {
        DestroyIcon(it.second.icon);
        DeleteObject(it.second.bm);
    }
}

CUploadEngineData* CMyEngineList::byName(const CString& name)
{
    return CUploadEngineListBase::byName(WCstringToUtf8(name));
}

int CMyEngineList::getUploadEngineIndex(const CString& Name) const
{
    return CUploadEngineListBase::getUploadEngineIndex(WCstringToUtf8(Name));
}

CString CMyEngineList::errorStr() const
{
    return m_ErrorStr;
}

bool CMyEngineList::loadFromFile(const CString& filename)
{
    if (!IuCoreUtils::FileExists(WCstringToUtf8(filename)))
    {
        m_ErrorStr = "File not found.";
        return false;
    }
    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    return CUploadEngineList::loadFromFile(WCstringToUtf8(filename), Settings->ServersSettings);
}

CMyEngineList::ServerIconCacheItem CMyEngineList::getIconBitmapForServer(const std::string& name) {
    std::lock_guard lk(cacheMutex_);
    const auto iconIt = serverIcons_.find(name);
    if (iconIt != serverIcons_.end()) {
        return iconIt->second;
    }
    
    CUploadEngineData *ued = CUploadEngineList::byName(name);

    HICON icon = nullptr;
    CString serverName = Utf8ToWCstring(name);
    serverName.Replace(_T("\\"), _T("_"));
    serverName.Replace(_T("/"), _T("_"));
    const CString dataFolder = IuCommonFunctions::GetDataFolder();
    CString iconFileName = dataFolder  + _T("Favicons\\")+ serverName +_T(".ico");

    if ( !WinUtils::FileExists(iconFileName) ) {
        if (ued && !ued->PluginName.empty()) {
            iconFileName = dataFolder + _T("Favicons\\") + Utf8ToWCstring(ued->PluginName) + _T(".ico");
            if (!WinUtils::FileExists(iconFileName)) {
                serverIcons_[name]={};
                return {};
            }
        } else {
            serverIcons_[name] = {};
            return {};
        }
    }

    const int w = GetSystemMetrics(SM_CXSMICON);
    const int h = GetSystemMetrics(SM_CYSMICON);

    LoadIconWithScaleDown(nullptr, iconFileName, w, h, &icon);
    
    if (!icon) {
        icon = static_cast<HICON>(LoadImage(nullptr, iconFileName, IMAGE_ICON, w, h, LR_LOADFROMFILE));
    }
    
    if ( !icon ) {
        return {};
    }
    ServerIconCacheItem item(icon, iconBitmapUtils_->HIconToBitmapPARGB32(icon));
    serverIcons_[name] = item;
    return item;
}

HICON CMyEngineList::getIconForServer(const std::string& name) {
    return getIconBitmapForServer(name).icon;
}

CString CMyEngineList::getIconNameForServer(const std::string& name) {
    CUploadEngineData *ued = CUploadEngineList::byName(name);
    CString iconFileName = IuCommonFunctions::GetDataFolder()+_T("Favicons\\")+Utf8ToWCstring(name).MakeLower()+_T(".ico");

    if ( !WinUtils::FileExists(iconFileName) && ued && !ued->PluginName.empty() ) {
        iconFileName = IuCommonFunctions::GetDataFolder()+_T("Favicons\\") + Utf8ToWCstring(ued->PluginName).MakeLower() +_T(".ico");
        if(!WinUtils::FileExists(iconFileName)) {
            return {};
        }
    }
    return U2W( IuCoreUtils::ExtractFileName(W2U(iconFileName)) );
}

void CMyEngineList::preLoadIcons() {
    if (iconsPreload_) {
        throw std::logic_error("preLoadIcons() should not be called twice");
    }
    iconsPreload_ = true;
    taskDispatcher_->post([this] {
        for (int i = 0; i < count(); i++) {
            CUploadEngineData* ued = byIndex(i);
            CString name = Utf8ToWCstring(ued->Name);
            [[maybe_unused]] auto icon = getIconBitmapForServer(ued->Name);
        }
    });
}
