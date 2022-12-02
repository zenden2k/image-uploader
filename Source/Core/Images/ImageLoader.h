#ifndef IU_CORE_IMAGES_IMAGELOADER_H
#define IU_CORE_IMAGES_IMAGELOADER_H

#pragma once

#include <memory>
#include <string>

#include "3rdpart/GdiplusH.h"
#include "Core/Video/GdiPlusImage.h"

class AbstractImageReader;

enum class ImageFormat { Unknown, WebP };

class ImageLoader {
public:
    std::unique_ptr<GdiPlusImage> loadFromFile(const wchar_t* fileName);
    std::unique_ptr<GdiPlusImage> loadFromMemory(uint8_t* data, size_t size);
    std::unique_ptr<GdiPlusImage> loadFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType);
    std::wstring getLastError() const;
    static ImageFormat getImageFormatFromData(uint8_t* data, size_t size);
    static std::unique_ptr<AbstractImageReader> createReaderForFormat(ImageFormat format);
protected:
    std::wstring lastError_;
};
#endif
