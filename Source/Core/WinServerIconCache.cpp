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

void WinServerIconCache::loadIcons(int dpi) {
    std::unique_ptr<CImageList, ImageListDeleter> imageList(new CImageList, ImageListDeleter {});
    const int iconWidth = DPIHelper::GetSystemMetricsForDpi(SM_CXSMICON, dpi);
    const int iconHeight = DPIHelper::GetSystemMetricsForDpi(SM_CYSMICON, dpi);
    imageList->Create(iconWidth, iconHeight, ILC_COLOR32, 3, 3);
    std::vector<int> indexes(engineList_->count(), -1);
    for (int i = 0; i < engineList_->count(); i++) {
        CUploadEngineData* ued = engineList_->byIndex(i);
        [[maybe_unused]] auto icon = getIconForServer(ued->Name, dpi);
        int iconIndex = imageList->AddIcon(icon);
        indexes[i] = iconIndex;
    }
    std::lock_guard lk(cacheMutex_);
    imageLists_[dpi] = { std::move(imageList), std::move(indexes) };
}

void WinServerIconCache::preLoadIcons(int dpi) {
    if (iconsPreload_) {
        throw std::logic_error("preLoadIcons() should not be called twice");
    }
    iconsPreload_ = true;

    future_ = std::async(std::launch::async, [this, dpi]() -> int {
        loadIcons(dpi);
        return 0;
    });
}

WinServerIconCache::ImageListWithIndexes WinServerIconCache::getImageList(int dpi) {
    std::lock_guard lk(cacheMutex_);
    auto it = imageLists_.find(dpi);
    if (it != imageLists_.end()) {
        return std::make_pair(it->second.first->m_hImageList, it->second.second);
    }
    loadIcons(dpi);
    it = imageLists_.find(dpi);
    if (it != imageLists_.end()) {
        return std::make_pair(it->second.first->m_hImageList, it->second.second);
    }
    return {};
}
