#include "SystemUtils.h"

#include <vector>

#include <strsafe.h>

#include "IuCommonFunctions.h"
#include "Core/Images/Utils.h"
#include "Core/Logging.h"

namespace SystemUtils {

bool CopyFileAndImageToClipboard(LPCTSTR fileName) {
    if (IuCommonFunctions::IsImage(fileName) ) {
        CopyImageToClipboard(fileName);
    }
    std::vector<CString> fileNames;
    fileNames.emplace_back(fileName);
    return CopyFilesToClipboard(fileNames, false);
}

bool CopyFilesToClipboard(const std::vector<CString>& fileNames, bool clearClipboard ) {
    size_t argc = fileNames.size();
    std::unique_ptr<WCHAR[]> pFullNames = std::make_unique<WCHAR[]>(argc * MAX_PATH + 1);
    WCHAR *p = pFullNames.get();
    for (size_t i = 0; i < argc; i++ ) {
        LPTSTR end = nullptr;
        StringCchCopyEx(p, MAX_PATH, fileNames[i], &end, nullptr, 0);
        if (!end) {
            break;
        }
        p = end + 1;
    }
    *p++ = 0; 
    DWORD dwDataBytes = sizeof(WCHAR) * (p - pFullNames.get());
    DROPFILES df = {sizeof(DROPFILES), {0, 0}, 0, TRUE};
    HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DROPFILES) + dwDataBytes);
    if (!hMem) {
        return false;
    }
    char *pGlobal = static_cast<char*>(GlobalLock(hMem));
    if (!pGlobal) {
        GlobalFree(hMem);
        return false;
    }
    CopyMemory(pGlobal, &df, sizeof(DROPFILES));
    CopyMemory(pGlobal + sizeof(DROPFILES), pFullNames.get(), dwDataBytes); // that's pGlobal + 20 bytes (the size of DROPFILES);
    GlobalUnlock(hMem);
    if ( OpenClipboard(nullptr) ) {
        if ( clearClipboard ) {
            EmptyClipboard();
        }
        SetClipboardData(CF_HDROP, hMem);
        CloseClipboard();
    }

    return true;
}

bool CopyImageToClipboard(LPCTSTR fileName) {
    std::unique_ptr<GdiPlusImage> img = ImageUtils::LoadImageFromFileExtended(fileName);
    if (!img) {
        LOG(ERROR) << "Unable to load image:" << std::endl << fileName;
        return false;
    }
    Gdiplus::Bitmap* src = img->getBitmap();
    if (!src || src->GetLastStatus() != Gdiplus::Ok) {
        LOG(ERROR) << "Unable to load image:" << std::endl << fileName;
        return false;
    }

    return CopyImageToClipboard(src);
}

bool CopyImageToClipboard(Gdiplus::Bitmap* image) {
    if ( OpenClipboard(nullptr) ) {
        EmptyClipboard();
        //if ( savingFormat != 0 ) {
            //Gdip_RemoveAlpha(*image, Color(255,255,255,255));
        //}
        HBITMAP out = 0;
        image->GetHBITMAP(Gdiplus::Color(255,255,255,255),&out);
        HDC dc = GetDC(NULL);
        CDC origDC,  destDC;
        origDC.CreateCompatibleDC(dc);
        CBitmap destBmp;
        destBmp.CreateCompatibleBitmap(dc, image->GetWidth(), image->GetHeight());
        HBITMAP oldOrigBmp = origDC.SelectBitmap(out);
        destDC.CreateCompatibleDC(dc);
        HBITMAP oldDestBmp = destDC.SelectBitmap(destBmp);
        destDC.BitBlt(0,0,image->GetWidth(),image->GetHeight(),origDC,0,0,SRCCOPY);
        destDC.SelectBitmap(oldDestBmp);
        origDC.SelectBitmap(oldOrigBmp);
        SetClipboardData(CF_BITMAP, destBmp);
        CloseClipboard();
        DeleteObject(out);
        ReleaseDC(NULL, dc);
        return true;
    }
    return false;
}

};
