#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

#include "AbstractServerIconCache.h"

class QtServerIconCache: public AbstractServerIconCache {
public:
    struct CacheItem {
        QIcon icon;
        QPixmap bm;
        CacheItem() = default;
        CacheItem(QIcon i, QPixmap b = {})
            : icon(i)
            , bm(b)
        {
        }
    };
    QtServerIconCache(CUploadEngineListBase* engineList, std::string iconsDir);

    NativeIcon getIconForServer(const std::string& name) override;

    [[nodiscard]] NativeIcon getBigIconForServer(const std::string& name) override;
    NativeBitmap getIconBitmapForServer(const std::string& name) override;

    /**
    * @throws std::logic_error 
    */
    void preLoadIcons() override;
private:
    std::unordered_map<std::string, CacheItem> serverIcons_;
    std::mutex cacheMutex_;
    bool iconsPreload_ = false;
    QIcon defaultServerIcon_;
    CacheItem tryIconLoad(const std::string& name);
};
