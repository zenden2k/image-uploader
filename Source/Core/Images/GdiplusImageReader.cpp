#include "GdiplusImageReader.h"

#include "Utils.h"
#include "Func/WinUtils.h"
#include "Core/Utils/CoreUtils.h"

typedef IStream * (STDAPICALLTYPE *SHCreateMemStreamFuncType)(const BYTE *pInit, UINT cbInit);

std::unique_ptr<GdiPlusImage> GdiplusImageReader::readFromFile(const wchar_t* fileName) {
    std::unique_ptr<Gdiplus::Bitmap> bm = std::make_unique<Gdiplus::Bitmap>(fileName);
    std::unique_ptr<GdiPlusImage> img = std::make_unique<GdiPlusImage>(bm.release());
    if (!checkLastStatus(img->getBitmap())) {
        return nullptr;
    }

    postLoad(img.get());
    return img;
}

std::unique_ptr<GdiPlusImage> GdiplusImageReader::readFromMemory(uint8_t* data, size_t size) {
    auto SHCreateMemStreamFunc = shlwapiLib.GetProcAddress<SHCreateMemStreamFuncType>("SHCreateMemStream");
    if (!SHCreateMemStreamFunc) {
        return nullptr;
    }

    IStream* pStream = SHCreateMemStreamFunc(data, size);
    if (pStream) {
        auto bitmap = readFromStream(pStream);
        pStream->Release();
        return bitmap;
    }

    return nullptr;
}

std::unique_ptr<GdiPlusImage> GdiplusImageReader::readFromStream(IStream* stream) {
    std::unique_ptr<Gdiplus::Bitmap> bm(Gdiplus::Bitmap::FromStream(stream, FALSE));

    if (!checkLastStatus(bm.get())) {
        return nullptr;
    }

    std::unique_ptr<GdiPlusImage> img = std::make_unique<GdiPlusImage>(bm.release());
    
    postLoad(img.get());
    return img;
}

std::wstring GdiplusImageReader::getLastError() {
    return lastError_;
}

bool GdiplusImageReader::checkLastStatus(Gdiplus::Bitmap* bm) {
    if (!bm) {
        return false;
    }
    int lastError = ::GetLastError();
    Gdiplus::Status status = bm->GetLastStatus();
    if (status != Gdiplus::Ok) {
    
        CString error = ImageUtils::GdiplusStatusToString(status);

        if (status == Gdiplus::Win32Error) {
            error += L"\r\n" + WinUtils::FormatWindowsErrorMessage(lastError);
        }
        lastError_ = error;
        return false;
    }
    return true;
}

void GdiplusImageReader::postLoad(GdiPlusImage* bm) {
    GUID format;
    if (bm->getBitmap()->GetRawFormat(&format) == Gdiplus::Ok) {
        bm->setSrcFormat(W2U(ImageUtils::ImageFormatGUIDToString(format)));
    }
    short orient = ImageUtils::GetImageOrientation(bm->getBitmap());
    ImageUtils::RotateAccordingToOrientation(orient, bm->getBitmap(), true);
    bm->setSrcMultiFrame(ImageUtils::IsImageMultiFrame(bm->getBitmap()));  
}
