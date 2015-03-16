#ifndef IU_CORE_IMAGES_UTILS_H
#define IU_CORE_IMAGES_UTILS_H

#include <windows.h>
namespace Gdiplus {
	class Bitmap;
	class Graphics;
	class Rect;
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType);
void PrintRichEdit(HWND hwnd, Gdiplus::Graphics* graphics, Gdiplus::Rect layoutArea);
#endif
