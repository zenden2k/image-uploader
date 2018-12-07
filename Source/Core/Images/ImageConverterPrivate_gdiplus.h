#pragma once

#include <string>

#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
#include "ImageConverterPrivateBase.h"
#include "Core/Video/AbstractImage.h"

class ImageConverterPrivate: public ImageConverterPrivateBase {
public:
    bool ImageConverterPrivate::Convert(const std::string& sourceFile);
    std::shared_ptr<AbstractImage> createThumbnail(AbstractImage* image, long long fileSize, int fileformat);
protected:
    bool createThumb(Gdiplus::Bitmap* bm, const CString& imageFile, int fileformat);
    Gdiplus::Brush* CreateBrushFromString(const std::string& br, const RECT& rect);
    bool EvaluateRect(const std::string& rectStr, RECT* out);
};
