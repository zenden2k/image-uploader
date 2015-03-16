#ifndef IU_CORE_IMAGES_UTILS_H
#define IU_CORE_IMAGES_UTILS_H

#include <windows.h>
namespace Gdiplus {
	class Bitmap;
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType);

#endif
