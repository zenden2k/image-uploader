/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "Core/Settings.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"


char CMyEngineList::DefaultServer[] = "default";

char CMyEngineList::RandomServer[]  = "random";


CMyEngineList::CMyEngineList() :dllModule_(_T("comctl32.dll"))
{
    if (WinUtils::IsVistaOrLater()) {
        LoadIconWithScaleDownFunc_ = dllModule_.GetProcAddress<LoadIconWithScaleDownFuncType>("LoadIconWithScaleDown");
    } else {
        LoadIconWithScaleDownFunc_ = nullptr;
    }
}

CMyEngineList::~CMyEngineList()
{
    for ( const auto& it: serverIcons_) {
        DestroyIcon(it.second);
    }
}

CUploadEngineData* CMyEngineList::byName(const CString& name)
{
    return CUploadEngineList_Base::byName(WCstringToUtf8(name));
}

int CMyEngineList::getUploadEngineIndex(const CString& Name)
{
    return CUploadEngineList_Base::GetUploadEngineIndex(WCstringToUtf8(Name));
}

CString CMyEngineList::ErrorStr() const
{
    return m_ErrorStr;
}

bool CMyEngineList::loadFromFile(const CString& filename)
{
    if (!IuCoreUtils::FileExists(WCstringToUtf8(filename)))
    {
        m_ErrorStr = "File not found.";
        return 0;
    }
    return CUploadEngineList::LoadFromFile(WCstringToUtf8(filename),Settings.ServersSettings);
}



HICON CMyEngineList::getIconForServer(const std::string& name) {
    auto iconIt = serverIcons_.find(name);
    if ( iconIt != serverIcons_.end() )
        return iconIt->second;
    
    CUploadEngineData *ued = CUploadEngineList::byName(name);
    std::string newName =  name;
    HICON icon = 0;
    CString iconFileName = IuCommonFunctions::GetDataFolder()+_T("Favicons\\")+Utf8ToWCstring(newName)+_T(".ico");

    if ( !WinUtils::FileExists(iconFileName) && ued && !ued->PluginName.empty() ) {
        iconFileName = IuCommonFunctions::GetDataFolder()+_T("Favicons\\") + Utf8ToWCstring(ued->PluginName) +_T(".ico");
    }

    int w = GetSystemMetrics(SM_CXSMICON);
    int h = GetSystemMetrics(SM_CYSMICON);
    if (LoadIconWithScaleDownFunc_) {
        LoadIconWithScaleDownFunc_(nullptr, iconFileName, w, h, &icon);
    }

    if (!icon) {
        icon = reinterpret_cast<HICON>(LoadImage(0, iconFileName, IMAGE_ICON, w, h, LR_LOADFROMFILE));
    }
    
    if ( !icon ) {
        return 0;
    }
    serverIcons_[name] = icon;
    return icon;
}

CString CMyEngineList::getIconNameForServer(const std::string& name) {
    CUploadEngineData *ued = CUploadEngineList::byName(name);
    std::string newName =  name;
    CString iconFileName = IuCommonFunctions::GetDataFolder()+_T("Favicons\\")+Utf8ToWCstring(newName)+_T(".ico");

    if ( !WinUtils::FileExists(iconFileName) && ued && !ued->PluginName.empty() ) {
        iconFileName = IuCommonFunctions::GetDataFolder()+_T("Favicons\\") + Utf8ToWCstring(ued->PluginName) +_T(".ico");
        if(!WinUtils::FileExists(iconFileName)) {
            return CString();
        }
    }
    return Utf8ToWCstring( IuCoreUtils::ExtractFileName(WCstringToUtf8(iconFileName)) );
}