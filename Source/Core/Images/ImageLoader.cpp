#include "ImageLoader.h"

#include <boost/format.hpp>

#include "Core/Upload/CommonTypes.h"
#include "WebpImageReader.h"
#include "GdiplusImageReader.h"

std::unique_ptr<Gdiplus::Bitmap> ImageLoader::loadFromFile(const wchar_t* fileName) {
    FILE* f = _wfopen(fileName, L"rb");
    if (!f) {
        lastError_ = str(boost::wformat(L"Cannot open file %s for reading.") % std::wstring(fileName));
        return nullptr;
    }
    uint8_t buffer[16];
    fread(buffer, 1, sizeof(buffer), f);
    fclose(f);
    ImageFormat format = getImageFormatFromData(buffer, sizeof(buffer));
    auto reader = createReaderForFormat(format);
    auto result = reader->readFromFile(fileName);
    if (!result) {
        lastError_ = reader->getLastError();
    }
    return result;
}

std::unique_ptr<Gdiplus::Bitmap> ImageLoader::loadFromMemory(uint8_t* data, size_t size) {
    ImageFormat format = getImageFormatFromData(data, sizeof(size));
    auto reader = createReaderForFormat(format);
    auto result = reader->readFromMemory(data, size);
    if (!result) {
        lastError_ = reader->getLastError();
    }
    return result;
}

std::unique_ptr<Gdiplus::Bitmap> ImageLoader::loadFromResource(HINSTANCE hInstance, LPCTSTR szResName,
    LPCTSTR szResType) {
    using namespace Gdiplus;
    HRSRC hrsrc = FindResource(hInstance, szResName, szResType);
    if (!hrsrc)
        return nullptr;
    // "Fake" HGLOBAL - look at MSDN
    HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
    DWORD sz = SizeofResource(hInstance, hrsrc);
    void* ptr1 = LockResource(hg1);
    HGLOBAL hg2 = GlobalAlloc(GMEM_FIXED, sz);

    ImageFormat format = getImageFormatFromData(reinterpret_cast<uint8_t*>(ptr1), sz);

    // Copy raster data
    CopyMemory(LPVOID(hg2), ptr1, sz);
    IStream* pStream;

    // TRUE means free memory at Release
    HRESULT hr = CreateStreamOnHGlobal(hg2, TRUE, &pStream);
    if (FAILED(hr)) {
        return nullptr;
    }

    // use load from IStream
    auto reader = createReaderForFormat(format);
    auto result = reader->readFromStream(pStream);
    if (!result) {
        lastError_ = reader->getLastError();
    }
    pStream->Release();
    // GlobalFree(hg2);
    return result;
}

std::wstring ImageLoader::getLastError() const {
    return lastError_;
}

ImageFormat ImageLoader::getImageFormatFromData(uint8_t* data, size_t size) {
    if (size < 12) {
        return ImageFormat::Unknown;
    }
    if (data[0] == 'R' && data[1] == 'I' &&data[2] == 'F' &&data[3] == 'F'
        && data[8] == 'W' && data[9] == 'E' && data[10] == 'B'&& data[11] == 'P'
        ) {
        return ImageFormat::WebP;
    }
    return ImageFormat::Unknown;
}

std::unique_ptr<AbstractImageReader> ImageLoader::createReaderForFormat(ImageFormat format) {
    if (format == ImageFormat::WebP) {
        return std::make_unique<WebpImageReader>();
    }
    return std::make_unique<GdiplusImageReader>();
}
