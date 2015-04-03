#ifndef GDIPLUSIMAGE_H
#define GDIPLUSIMAGE_H

#include "AbstractImage.h"
#include <3rdpart/GdiplusH.h>

class GdiPlusImage: public AbstractImage
{
public:
    GdiPlusImage();
    virtual ~GdiPlusImage();
    virtual bool saveToFile(const Utf8String& fileName) const;
	virtual bool isNull() const;
	virtual bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data,size_t dataSize, void* parameter);
	bool loadFromRgb(int width, int height, uint8_t* data, size_t dataSize);
	Gdiplus::Bitmap* getBitmap() const;
protected:
	Gdiplus::Bitmap *bm_;
	uint8_t* data_;
};

#endif
