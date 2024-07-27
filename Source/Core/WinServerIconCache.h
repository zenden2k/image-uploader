#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>

#include "AbstractServerIconCache.h"

class IconBitmapUtils;

class WinServerIconCache : public AbstractServerIconCache {
public:
    struct WinIcon {
        HICON icon {};
        HBITMAP bm {};
        WinIcon() = default;
        WinIcon(HICON i, HBITMAP b = NULL)
            : icon(i)
            , bm(b)
        {
        }
    };

    WinServerIconCache(CUploadEngineListBase* engineList, std::string iconsDir);
    ~WinServerIconCache() override;

    NativeIcon getIconForServer(const std::string& name) override;

    /**
    * The caller of this function is responsible for destroying
    * the icon when it is no longer needed.
    */
    [[nodiscard]] NativeIcon getBigIconForServer(const std::string& name) override;

    NativeBitmap getIconBitmapForServer(const std::string& name) override;

    /**
    * @throws std::logic_error 
    */
    void preLoadIcons() override;

private:
    std::unordered_map<std::string, WinIcon> serverIcons_;
    std::unique_ptr<IconBitmapUtils> iconBitmapUtils_;
    std::mutex cacheMutex_;
    bool iconsPreload_ = false;

    WinIcon tryIconLoad(const std::string& name);
};
