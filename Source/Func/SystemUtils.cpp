#include "SystemUtils.h"

#include "atlheaders.h"
#include "MyUtils.h"
#include <vector>

namespace SystemUtils {

bool CopyFileAndImageToClipboard(LPCTSTR fileName) {
    if ( IsImage(fileName) ) {
        CopyImageToClipboard(fileName);
    }
    std::vector<LPCTSTR> fileNames;
    fileNames.push_back(fileName);
    return CopyFilesToClipboard(fileNames, false);
}

bool CopyFilesToClipboard(const std::vector<LPCTSTR>& fileNames, bool clearClipboard ) {
    int argc = fileNames.size();
    WCHAR buf[MAX_PATH];
    WCHAR *pFullNames = (WCHAR*) malloc(argc * MAX_PATH * sizeof(WCHAR));
    WCHAR *p = pFullNames;
    for ( int i = 0; i < argc; i++ ) {
        lstrcpy(p, fileNames[i]);
        p += lstrlen( fileNames[i] ) + 1;
    }
    *p++ = 0; 
    DWORD dwDataBytes = sizeof(WCHAR) * (p - pFullNames);
    DROPFILES df = {sizeof(DROPFILES), {0, 0}, 0, TRUE};
    HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DROPFILES) + dwDataBytes);
    WCHAR *pGlobal = (WCHAR *) GlobalLock(hMem);
    CopyMemory(pGlobal, &df, sizeof(DROPFILES));
    CopyMemory(pGlobal + 10, pFullNames, dwDataBytes); // that's pGlobal + 20 bytes (the size of DROPFILES);
    GlobalUnlock(hMem);
    if ( OpenClipboard(NULL) ) {
        if ( clearClipboard ) {
            EmptyClipboard();
        }
        SetClipboardData(CF_HDROP, hMem);
        CloseClipboard();
    }
    free(pFullNames);
    return true;
}

bool CopyImageToClipboard(LPCTSTR fileName) {
    Gdiplus::Bitmap bitmap(fileName);
    return CopyImageToClipboard(&bitmap);
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
