#pragma once

#include <memory>
#include <string>

#ifdef IU_QT
#include <QIcon>
#include <QPixmap>

using NativeIcon = QIcon;
using NativeBitmap = QPixmap;
#elif defined(_WIN32)
using NativeIcon = HICON;
using NativeBitmap = HBITMAP;
#else
using NativeIcon = void*;
using NativeBitmap = void*;
#endif

class CUploadEngineListBase;

class AbstractServerIconCache {
public:
    AbstractServerIconCache(CUploadEngineListBase* engineList, std::string iconsDir)
        : engineList_(engineList)
        , iconsDir_(std::move(iconsDir))

    {

    }
    virtual ~AbstractServerIconCache() { }
    virtual NativeIcon getIconForServer(const std::string& name) = 0;

    [[nodiscard]] virtual NativeIcon getBigIconForServer(const std::string& name) = 0;

    virtual NativeBitmap getIconBitmapForServer(const std::string& name) = 0;

    virtual std::string getIconNameForServer(const std::string& name, bool returnFullPath = false);
    /**
    * @throws std::logic_error 
    */
    virtual void preLoadIcons() = 0;

protected:
    CUploadEngineListBase* engineList_;
    std::string iconsDir_;
};
