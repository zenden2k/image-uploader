#include "HeifImageReader.h"

#include <libheif/heif.h>

#include <boost/format.hpp>

#include "Core/Utils/CoreUtils.h"
#include "Core/Images/GdiPlusImage.h"
#include "Utils.h"

using namespace Gdiplus;

bool HeifImageReader::canRead(const uint8_t* data, int len) {
    auto type = heif_check_filetype(data, len);      
    return type != heif_filetype_no && type != heif_filetype_yes_unsupported;
}

std::unique_ptr<GdiPlusImage> HeifImageReader::readFromFile(const wchar_t* fileName) {
    heif_context* ctx = heif_context_alloc();
    heif_error err = heif_context_read_from_file(ctx, IuCoreUtils::WstringToUtf8(fileName).c_str(), nullptr);

    if (err.code != 0) {
        lastError_ = boost::str(boost::wformat(L"Could not read HEIF/AVIF file: %s") % err.message);
        heif_context_free(ctx);
        return {};
    }
    // get a handle to the primary image
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);

    // decode the image and convert colorspace to RGBA, saved as 32bit interleaved
    heif_image* img;
    err =  heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, nullptr);

    if (err.code != 0) {
        lastError_ = boost::str(boost::wformat(L"Could not decode HEIF/AVIF file: %s") % err.message);
        heif_context_free(ctx);
        return {};
    }
    
    auto bm = readHeif(img);
    // clean up resources
    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);
    std::unique_ptr<GdiPlusImage> res = std::make_unique<GdiPlusImage>(bm.release());
    postLoad(res.get());
    return res;
}

std::unique_ptr<Gdiplus::Bitmap> HeifImageReader::readHeif(heif_image* img) {
    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

    int width = heif_image_get_width(img, heif_channel_interleaved);
    int height = heif_image_get_height(img, heif_channel_interleaved);

    auto bm = std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);

    BitmapData dstData;
    Rect rc(0, 0, width, height);
    constexpr auto colorSize = 4;
    if (bm->LockBits(&rc, ImageLockModeWrite, PixelFormat32bppARGB, &dstData) == Ok) {
        auto* dstBits = static_cast<uint8_t*>(dstData.Scan0);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                size_t srcOffset = y * stride + x * colorSize;
                size_t destOffset = y * dstData.Stride + x * colorSize;
                dstBits[destOffset] = data[srcOffset + 2];
                dstBits[destOffset + 1] = data[srcOffset + 1];
                dstBits[destOffset + 2] = data[srcOffset];
                dstBits[destOffset + 3] = data[srcOffset + 3];
            }
        }
        bm->UnlockBits(&dstData);
    }
    return bm;
}

std::unique_ptr<GdiPlusImage> HeifImageReader::readFromMemory(uint8_t* data, size_t size) {

    heif_context* ctx = heif_context_alloc();
    heif_error err = heif_context_read_from_memory_without_copy(ctx, data, size, nullptr);

    if (err.code != 0) {
        lastError_ = boost::str(boost::wformat(L"Could not read HEIF/AVIF file: %s") % err.message);
        heif_context_free(ctx);
        return {};
    }
    // get a handle to the primary image
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);

    // decode the image and convert colorspace to RGBA, saved as 32bit interleaved
    heif_image* img;
    err = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, nullptr);

    if (err.code != 0) {
        lastError_ = boost::str(boost::wformat(L"Could not decode HEIF/AVIF file: %s") % err.message);
        heif_context_free(ctx);
        return {};
    }

    auto bm = readHeif(img);
    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    std::unique_ptr<GdiPlusImage> res = std::make_unique<GdiPlusImage>(bm.release());
    postLoad(res.get());
    return res;
}

std::unique_ptr<GdiPlusImage> HeifImageReader::readFromStream(IStream* stream) {
    LARGE_INTEGER li;
    li.QuadPart = 0;

    if (FAILED(stream->Seek(li, STREAM_SEEK_SET, nullptr))) {
        return {};
    }

    STATSTG stats;
    if (FAILED(stream->Stat(&stats, STATFLAG_NONAME))) {
        return {};
    }
    size_t sSize = static_cast<size_t>(stats.cbSize.QuadPart);
    ULONG bytesRead;
    std::unique_ptr<uint8_t[]> pBuffer;
    try {
        pBuffer = std::make_unique<uint8_t[]>(sSize);
    }
    catch (const std::exception &) {
        lastError_ = str(boost::wformat(L"Unable to allocate %d bytes") % sSize);
        return {};
    }
   
    if (FAILED(stream->Read(pBuffer.get(), static_cast<ULONG>(sSize), &bytesRead))) {
        lastError_ = L"Failed to read from IStream";
        return {};
    }

    return readFromMemory(pBuffer.get(), sSize);
}

std::wstring HeifImageReader::getLastError() {
    return lastError_;
}

void HeifImageReader::postLoad(GdiPlusImage* img) {
    img->setSrcFormat("HEIF/AVIF");
}
