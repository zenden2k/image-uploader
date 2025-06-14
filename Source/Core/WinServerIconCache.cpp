#include "WinServerIconCache.h"

#include <ComDef.h>

#include "Core/UploadEngineList.h"
#include "Gui/IconBitmapUtils.h"
#include "Core/Utils/StringUtils.h"
#include "Func/WinUtils.h"
#include "Gui/Helpers/DPIHelper.h"

WinServerIconCache::WinServerIconCache(CUploadEngineListBase* engineList, std::string iconsDir)
    : AbstractServerIconCache(engineList, iconsDir)
{
    iconBitmapUtils_ = std::make_unique<IconBitmapUtils>();
}

WinServerIconCache::~WinServerIconCache(){
    if (future_.valid()) {
        future_.wait();
    }
    for (const auto& it : serverIcons_) {
        DestroyIcon(it.second.icon);
        DeleteObject(it.second.bm);
    }
}

WinServerIconCache::WinIcon WinServerIconCache::tryIconLoad(const std::string& name, int dpi) {
    std::lock_guard lk(cacheMutex_);
    auto key = std::make_pair(dpi, name);
    const auto iconIt = serverIcons_.find(key);
    if (iconIt != serverIcons_.end()) {
        return iconIt->second;
    }

    CUploadEngineData* ued = engineList_->byName(name);

    HICON icon = nullptr;
    CString iconFileName = IuCoreUtils::Utf8ToWstring(getIconNameForServer(name, true)).c_str();

    /*if (!WinUtils::FileExists(iconFileName)) {
        serverIcons_[name] = {};
        return {};
    }*/

    const int w = DPIHelper::GetSystemMetricsForDpi(SM_CXSMICON, dpi);
    const int h = DPIHelper::GetSystemMetricsForDpi(SM_CYSMICON, dpi);

    HRESULT hr = LoadIconWithScaleDown(nullptr, iconFileName, w, h, &icon);

    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            serverIcons_[key] = {};
            return {}; 
        } else {
            _com_error err(hr);
            LOG(WARNING) << "LoadIconWithScaleDown" << std::endl << err.ErrorMessage();
        }
    }

    if (!icon) {
        icon = static_cast<HICON>(LoadImage(nullptr, iconFileName, IMAGE_ICON, w, h, LR_LOADFROMFILE));
    }

    if (!icon) {
        serverIcons_[key] = {};
        return {};
    }
    WinIcon item(icon, iconBitmapUtils_->HIconToBitmapPARGB32(icon, dpi));
    serverIcons_[key] = item;
    return item;
}

NativeBitmap WinServerIconCache::getIconBitmapForServer(const std::string& name, int dpi) {
     return tryIconLoad(name, dpi).bm;
}

NativeIcon WinServerIconCache::getIconForServer(const std::string& name, int dpi) {
    return tryIconLoad(name, dpi).icon;
}

NativeIcon WinServerIconCache::getBigIconForServer(const std::string& name, int dpi) {
    CString iconFileName = IuCoreUtils::Utf8ToWstring(getIconNameForServer(name, true)).c_str();

    if (iconFileName.IsEmpty()) {
        return {};
    }
    const int w = DPIHelper::GetSystemMetricsForDpi(SM_CXICON, dpi);
    const int h = DPIHelper::GetSystemMetricsForDpi(SM_CYICON, dpi);
    HICON icon {};
    HRESULT hr = LoadIconWithScaleDown(nullptr, iconFileName, w, h, &icon);

    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            return {};
        } else {
            _com_error err(hr);
            LOG(WARNING) << "getBigIconForServer() LoadIconWithScaleDown" << std::endl
                         << err.ErrorMessage();
        }
    }

    if (!icon) {
        icon = static_cast<HICON>(LoadImage(nullptr, iconFileName, IMAGE_ICON, w, h, LR_LOADFROMFILE));
    }

    return icon;
}

void WinServerIconCache::preLoadIcons(int dpi) {
    if (iconsPreload_) {
        throw std::logic_error("preLoadIcons() should not be called twice");
    }
    iconsPreload_ = true;

    future_ = std::async(std::launch::async, [this, dpi]() -> int {
        for (int i = 0; i < engineList_->count(); i++) {
            CUploadEngineData* ued = engineList_->byIndex(i);
            [[maybe_unused]] auto icon = getIconBitmapForServer(ued->Name, dpi);
        }
        return 0;
    });
}
