#ifndef FUNC_SYSTEM_UTILS_H
#define FUNC_SYSTEM_UTILS_H

#include <gdiplus.h>

namespace SystemUtils {
	bool CopyFilesToClipboard(const std::vector<LPCTSTR>& fileNames, bool clearClipboard = true);
	bool CopyFileAndImageToClipboard(LPCTSTR fileName);
	bool CopyImageToClipboard(LPCTSTR fileName);
	bool CopyImageToClipboard(Gdiplus::Bitmap* image);
};

#endif;