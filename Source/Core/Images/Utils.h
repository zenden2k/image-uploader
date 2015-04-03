#ifndef IU_CORE_IMAGES_UTILS_H
#define IU_CORE_IMAGES_UTILS_H

#include "atlheaders.h"
#include <windows.h>
#include <3rdpart/GdiplusH.h>

enum SaveImageFormat {
	sifJPEG,sifPNG,sifGIF, sifDetectByExtension
};
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType);
void PrintRichEdit(HWND hwnd, Gdiplus::Graphics* graphics,  Gdiplus::Bitmap* background, Gdiplus::Rect layoutArea);
void DrawRoundedRectangle(Gdiplus::Graphics* gr, Gdiplus::Rect r, int d, Gdiplus::Pen* p, Gdiplus::Brush*br);
bool SaveImage(Gdiplus::Image* img, const CString& szFilename, SaveImageFormat Format, int Quality);
Gdiplus::Bitmap* IconToBitmap(HICON ico);
void ApplyGaussianBlur(Gdiplus::Bitmap* bm, int x,int y, int w, int h, int radius);
void BlurCleanup();
Gdiplus::Bitmap* LoadImageFromFileWithoutLocking(const WCHAR* fileName);
Gdiplus::Color StringToColor(const std::string& str);
bool CopyBitmapToClipboard(HWND hwnd, HDC dc, Gdiplus::Bitmap* bm, bool preserveAlpha = true);
void Gdip_RemoveAlpha(Gdiplus::Bitmap& source, Gdiplus::Color color );
#endif
