#ifndef IU_CORE_IMAGES_IMAGEREADER_H
#define IU_CORE_IMAGES_IMAGEREADER_H

#pragma once

#include <memory>
#include <string>

#include "3rdpart/GdiplusH.h"


class AbstractImageReader {
public:
    virtual ~AbstractImageReader() = default;
    virtual std::unique_ptr<Gdiplus::Bitmap> readFromFile(const wchar_t* fileName) = 0;
    virtual std::unique_ptr<Gdiplus::Bitmap> readFromMemory(uint8_t* data, size_t size) = 0;
    virtual std::unique_ptr<Gdiplus::Bitmap> readFromStream(IStream* stream) = 0;
    virtual std::wstring getLastError() = 0;
};

#endif
