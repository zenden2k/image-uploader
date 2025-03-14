#include "MimeTypeHelper.h"

std::string MimeTypeHelper::getDefaultExtensionForMimeType(const std::string& mimeType) {
    auto it = mimeToExt_.find(mimeType);
    if (it != mimeToExt_.end()) {
        return it->second;
    }
    return {};
}

std::unordered_map<std::string, std::string> MimeTypeHelper::mimeToExt_ = {
    { "image/gif", "gif" },
    { "image/png", "png" },
    { "image/jpeg", "jpg" },
    { "image/webp", "webp" },
    { "image/bmp", "bmp" },
    { "image/sgi", "sgi" },
    { "image/svg+xml", "svg" },
    { "image/tiff", "tiff" },
    { "image/vnd.adobe.photoshop", "psd" },
    { "image/x-icon", "ico" },
    { "image/x-xpixmap", "xpm" },
    { "image/x-tga", "tga" },
    { "image/heic", "heic" },
    { "image/heif", "heif" },
    { "image/avif", "avif" }
};

