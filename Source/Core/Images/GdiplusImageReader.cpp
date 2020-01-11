#include "GdiplusImageReader.h"

#include "Utils.h"
#include "Func/WinUtils.h"

typedef IStream * (STDAPICALLTYPE *SHCreateMemStreamFuncType)(const BYTE *pInit, UINT cbInit);

std::unique_ptr<Gdiplus::Bitmap> GdiplusImageReader::readFromFile(const wchar_t* fileName) {
    std::unique_ptr<Gdiplus::Bitmap> bm = std::make_unique<Gdiplus::Bitmap>(fileName);
    if (!checkLastStatus(bm.get())) {
        return nullptr;
    }
;   
    postLoad(bm.get());
    return bm;
}

std::unique_ptr<Gdiplus::Bitmap> GdiplusImageReader::readFromMemory(uint8_t* data, size_t size) {
    if (WinUtils::IsVistaOrLater()) {
        auto SHCreateMemStreamFunc = shlwapiLib.GetProcAddress<SHCreateMemStreamFuncType>("SHCreateMemStream");
        if (!SHCreateMemStreamFunc) {
            return nullptr;
        }

        IStream* pStream = SHCreateMemStreamFunc(data, size);
        if (pStream) {
            std::unique_ptr<Gdiplus::Bitmap> bitmap = readFromStream(pStream);
            pStream->Release();
            return bitmap;
        }
    }
    else {
        HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, size);
        if (buffer) {
            void* pBuffer = ::GlobalLock(buffer);
            if (pBuffer) {
                CopyMemory(pBuffer, data, size);
                IStream* pStream = nullptr;
                if (::CreateStreamOnHGlobal(buffer, FALSE, &pStream) == S_OK) {
                    std::unique_ptr<Gdiplus::Bitmap> bitmap = readFromStream(pStream);
                    pStream->Release();
                    return bitmap;
                }
                ::GlobalUnlock(buffer);
            }
            ::GlobalFree(buffer);
        }
    }
    return nullptr;
}

std::unique_ptr<Gdiplus::Bitmap> GdiplusImageReader::readFromStream(IStream* stream) {
    std::unique_ptr<Gdiplus::Bitmap> bm(Gdiplus::Bitmap::FromStream(stream, FALSE));

    if (!checkLastStatus(bm.get())) {
        return nullptr;
    }
    ;
    postLoad(bm.get());
    return bm;
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

void GdiplusImageReader::postLoad(Gdiplus::Bitmap* bm) {
    short orient = ImageUtils::GetImageOrientation(bm);
    ImageUtils::RotateAccordingToOrientation(orient, bm, true);
}
