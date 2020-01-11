#ifndef IU_CORE_IMAGES_WEBPIMAGEREADER_H
#define IU_CORE_IMAGES_WEBPIMAGEREADER_H

#pragma once

#include "AbstractImageReader.h"

struct WebPPic;

class WebpImageReader : public AbstractImageReader {
public:
    std::unique_ptr<Gdiplus::Bitmap> readFromFile(const wchar_t* fileName) override;
    std::unique_ptr<Gdiplus::Bitmap> readFromMemory(uint8_t* data, size_t size) override;
    std::unique_ptr<Gdiplus::Bitmap> readFromStream(IStream* stream) override;
    std::wstring getLastError() override;
private:
    std::wstring lastError_;
    bool readWebP(const uint8_t* const data, size_t data_size, WebPPic* pic);
};

#endif