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
    fileNames.push_back(fileName);
    return CopyFilesToClipboard(fileNames, false);
}

bool CopyFilesToClipboard(const std::vector<CString>& fileNames, bool clearClipboard ) {
    int argc = fileNames.size();
    WCHAR *pFullNames = new WCHAR[argc * MAX_PATH + 1];
    WCHAR *p = pFullNames;
    for ( int i = 0; i < argc; i++ ) {
        LPTSTR end = nullptr;
        StringCchCopyEx(p, MAX_PATH, fileNames[i], &end, nullptr, 0);
        if (!end) {
            break;
        }
        p = end + 1;
    }
    *p++ = 0; 
    DWORD dwDataBytes = sizeof(WCHAR) * (p - pFullNames);
    DROPFILES df = {sizeof(DROPFILES), {0, 0}, 0, TRUE};
    HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DROPFILES) + dwDataBytes);
    char *pGlobal = reinterpret_cast<char*>(GlobalLock(hMem));
    CopyMemory(pGlobal, &df, sizeof(DROPFILES));
    CopyMemory(pGlobal + sizeof(DROPFILES), pFullNames, dwDataBytes); // that's pGlobal + 20 bytes (the size of DROPFILES);
    GlobalUnlock(hMem);
    if ( OpenClipboard(nullptr) ) {
        if ( clearClipboard ) {
            EmptyClipboard();
        }
        SetClipboardData(CF_HDROP, hMem);
        CloseClipboard();
    }
    delete[] pFullNames;
    return true;
}

bool CopyImageToClipboard(LPCTSTR fileName) {
    std::unique_ptr<Gdiplus::Bitmap> src(ImageUtils::LoadImageFromFileExtended(fileName));
    if (!src || src->GetLastStatus() != Gdiplus::Ok) {
        LOG(ERROR) << "Unable to load image:" << std::endl << fileName;
        return false;
    }

    return CopyImageToClipboard(src.get());
}

bool CopyImageToClipboard(Gdiplus::Bitmap* image) {
    if ( OpenClipboard(NULL) ) {
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
        CloseClipboard(); //закрываем буфер обмена
        DeleteObject(out);
        ReleaseDC(NULL, dc);
        return true;
    }
    return false;
}

};
