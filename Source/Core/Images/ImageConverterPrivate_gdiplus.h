#pragma once

#include "ImageConverterPrivateBase.h"
#include "Core/Video/AbstractImage.h"
#include <string>

class ImageConverterPrivate: public ImageConverterPrivateBase {
public:
    bool ImageConverterPrivate::Convert(const std::string& sourceFile);
    bool createThumbnail(AbstractImage* image, AbstractImage** outResult, int64_t fileSize, int fileformat);
protected:
    bool createThumb(Gdiplus::Bitmap* bm, const CString& imageFile, int fileformat);
    Gdiplus::Brush* CreateBrushFromString(const std::string& br, RECT rect);
    bool EvaluateRect(const std::string& rectStr, RECT* out);
 

};
