#ifndef FUNC_SYSTEM_UTILS_H
#define FUNC_SYSTEM_UTILS_H

#include <3rdpart/GdiplusH.h>
#include <vector>

namespace SystemUtils {
	bool CopyFilesToClipboard(const std::vector<LPCTSTR>& fileNames, bool clearClipboard = true);
	bool CopyFileAndImageToClipboard(LPCTSTR fileName);
	bool CopyImageToClipboard(LPCTSTR fileName);
	bool CopyImageToClipboard(Gdiplus::Bitmap* image);
};

#endif;