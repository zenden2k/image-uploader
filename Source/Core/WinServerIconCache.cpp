#include "WinServerIconCache.h"

#include "Core/UploadEngineList.h"
#include "Gui/IconBitmapUtils.h"
#include "Core/Utils/StringUtils.h"
#include "Core/ServiceLocator.h"
#include "Core/TaskDispatcher.h"
#include "Func/WinUtils.h"

WinServerIconCache::WinServerIconCache(CUploadEngineListBase* engineList, std::string iconsDir)
    : AbstractServerIconCache(engineList, iconsDir)
{
    iconBitmapUtils_ = std::make_unique<IconBitmapUtils>();
}

WinServerIconCache::~WinServerIconCache(){
    for (const auto& it : serverIcons_) {
        DestroyIcon(it.second.icon);
        DeleteObject(it.second.bm);
    }
}

WinServerIconCache::WinIcon WinServerIconCache::tryIconLoad(const std::string& name) {
    std::lock_guard lk(cacheMutex_);
    const auto iconIt = serverIcons_.find(name);
    if (iconIt != serverIcons_.end()) {
        return iconIt->second;
    }

    CUploadEngineData* ued = engineList_->byName(name);

    HICON icon = nullptr;
    CString iconFileName = IuCoreUtils::Utf8ToWstring(getIconNameForServer(name, true)).c_str();

    if (!WinUtils::FileExists(iconFileName)) {
        serverIcons_[name] = {};
        return {};
    }

    const int w = GetSystemMetrics(SM_CXSMICON);
    const int h = GetSystemMetrics(SM_CYSMICON);

    LoadIconWithScaleDown(nullptr, iconFileName, w, h, &icon);

    if (!icon) {
        icon = static_cast<HICON>(LoadImage(nullptr, iconFileName, IMAGE_ICON, w, h, LR_LOADFROMFILE));
    }

    if (!icon) {
        return {};
    }
    WinIcon item(icon, iconBitmapUtils_->HIconToBitmapPARGB32(icon));
    serverIcons_[name] = item;
    return item;
}

NativeBitmap WinServerIconCache::getIconBitmapForServer(const std::string& name) {
     return tryIconLoad(name).bm;
}

NativeIcon WinServerIconCache::getIconForServer(const std::string& name) {
    return tryIconLoad(name).icon;
}

NativeIcon WinServerIconCache::getBigIconForServer(const std::string& name) {
    CString iconFileName = IuCoreUtils::Utf8ToWstring(getIconNameForServer(name, true)).c_str();

    if (iconFileName.IsEmpty()) {
        return {};
    }
    const int w = GetSystemMetrics(SM_CXICON);
    const int h = GetSystemMetrics(SM_CYICON);
    HICON icon {};
    LoadIconWithScaleDown(nullptr, iconFileName, w, h, &icon);

    if (!icon) {
        icon = static_cast<HICON>(LoadImage(nullptr, iconFileName, IMAGE_ICON, w, h, LR_LOADFROMFILE));
    }

    return icon;
}

void WinServerIconCache::preLoadIcons() {
    if (iconsPreload_) {
        throw std::logic_error("preLoadIcons() should not be called twice");
    }
    iconsPreload_ = true;
    auto* taskDispatcher = ServiceLocator::instance()->taskDispatcher();

    taskDispatcher->post([this] {
        for (int i = 0; i < engineList_->count(); i++) {
            CUploadEngineData* ued = engineList_->byIndex(i);
            [[maybe_unused]] auto icon = getIconBitmapForServer(ued->Name);
        }
    });
}
