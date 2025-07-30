#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <future>

#include "AbstractServerIconCache.h"

class IconBitmapUtils;

template <typename T>
inline void hash_combine(std::size_t& seed, const T& val) {
    std::hash<T> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

//  taken from https://stackoverflow.com/a/7222201/916549
//
template <typename S, typename T>
struct std::hash<std::pair<S, T>> {
    inline size_t operator()(const std::pair<S, T>& val) const {
        size_t seed = 0;
        hash_combine(seed, val.first);
        hash_combine(seed, val.second);
        return seed;
    }
};

/*struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto hash1 = std::hash<T1> {}(p.first);
        auto hash2 = std::hash<T2> {}(p.second);
        return hash1 ^ (hash2 << 1); // Combine hashes
    }
};*/

class ImageListDeleter {
public:
    void operator()(CImageList* ptr) const {
        ptr->Destroy();
        delete ptr;
    }
};

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

    NativeIcon getIconForServer(const std::string& name, int dpi) override;

    /**
    * The caller of this function is responsible for destroying
    * the icon when it is no longer needed.
    */
    [[nodiscard]] NativeIcon getBigIconForServer(const std::string& name, int dpi) override;

    NativeBitmap getIconBitmapForServer(const std::string& name, int dpi) override;

    /**
    * @throws std::logic_error 
    */
    void preLoadIcons(int dpi) override;

    using ImageListWithIndexes = std::pair<HIMAGELIST, std::vector<int>>;
    ImageListWithIndexes getImageList(int dpi);

private :
    std::unordered_map<std::pair<int, std::string>, WinIcon> serverIcons_;
    std::unique_ptr<IconBitmapUtils> iconBitmapUtils_;
    std::mutex cacheMutex_;
    std::future<int> future_;
    bool iconsPreload_ = false;
    std::unordered_map<int, std::pair<std::unique_ptr<CImageList, ImageListDeleter>, std::vector<int>>> imageLists_;
    WinIcon tryIconLoad(const std::string& name, int dpi);
    void loadIcons(int dpi);
};
