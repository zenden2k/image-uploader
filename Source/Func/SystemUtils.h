#ifndef FUNC_SYSTEM_UTILS_H
#define FUNC_SYSTEM_UTILS_H

#include <vector>
#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"

namespace SystemUtils {
    bool CopyFilesToClipboard(const std::vector<CString>& fileNames, bool clearClipboard = true);
    bool CopyFileAndImageToClipboard(LPCTSTR fileName);
    bool CopyImageToClipboard(LPCTSTR fileName);
    bool CopyImageToClipboard(Gdiplus::Bitmap* image);
};

#endif;