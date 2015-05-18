#ifndef IU_CORE_IMAGES_UTILS_H
#define IU_CORE_IMAGES_UTILS_H

#include "atlheaders.h"
#include <windows.h>
#include "3rdpart/GdiplusH.h"
#include <string>

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
bool MySaveImage(Gdiplus::Image* img, const CString& szFilename, CString& szBuffer, int Format, int Quality,
    LPCTSTR Folder = 0);
void DrawGradient(Gdiplus::Graphics& gr, Gdiplus::Rect rect, Gdiplus::Color& Color1, Gdiplus::Color& Color2);
void DrawStrokedText(Gdiplus::Graphics& gr, LPCTSTR Text, Gdiplus::RectF Bounds, Gdiplus::Font& font,
    Gdiplus::Color& ColorText, Gdiplus::Color& ColorStroke, int HorPos = 0, int VertPos = 0,
    int width = 1);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void changeAplhaChannel(Bitmap& source, Bitmap& dest, int sourceChannel, int destChannel);
Rect MeasureDisplayString(Graphics& graphics, CString text, RectF boundingRect, Gdiplus::Font& font);
CRect CenterRect(CRect r1, CRect intoR2);
void DrawRect(Bitmap& gr, Color& color, Rect rect);
Gdiplus::Bitmap* GetThumbnail(Gdiplus::Image* bm, int width, int height, Gdiplus::Size* realSize = 0);
Gdiplus::Bitmap* GetThumbnail(const CString& filename, int width, int height, Gdiplus::Size* realSize = 0);
Size AdaptProportionalSize(const Size& szMax, const Size& szReal);
Gdiplus::Bitmap* BitmapFromMemory(BYTE* data, unsigned int size);
#endif
