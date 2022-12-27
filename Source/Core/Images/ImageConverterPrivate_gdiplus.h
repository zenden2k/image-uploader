﻿#pragma once

#include <string>

#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
#include "ImageConverterPrivateBase.h"
#include "Utils.h"
#include "Core/Images/AbstractImage.h"

class ImageConverterPrivate: public ImageConverterPrivateBase {
public:
    bool convert(const std::string& sourceFile);
    std::shared_ptr<AbstractImage> createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat);
protected:
    bool createThumb(Gdiplus::Bitmap* bm, const CString& imageFile, ImageUtils::SaveImageFormat fileformat);
    Gdiplus::Brush* CreateBrushFromString(const std::string& br, const RECT& rect);
    bool EvaluateRect(const std::string& rectStr, RECT* out);
    static void calcCropSize(int srcWidth, int srcHeight, CRect targetRect, CRect& destRect, CRect& srcRect);
};
