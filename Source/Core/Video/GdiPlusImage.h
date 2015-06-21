#ifndef GDIPLUSIMAGE_H
#define GDIPLUSIMAGE_H

#include "AbstractImage.h"
#include <windows.h>
#include "3rdpart/GdiplusH.h"
#include <memory>

class GdiPlusImage: public AbstractImage
{
public:
    GdiPlusImage();
    explicit GdiPlusImage(Gdiplus::Bitmap *bm, bool takeOwnership  = true);
    virtual ~GdiPlusImage();
    virtual bool saveToFile(const std::string& fileName) const;
    virtual bool isNull() const;
    virtual bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data,size_t dataSize, void* parameter);
    bool loadFromRgb(int width, int height, uint8_t* data, size_t dataSize);
    Gdiplus::Bitmap* getBitmap() const;

    int getWidth() const override;
    int getHeight() const override;
protected:
    std::shared_ptr<Gdiplus::Bitmap> bm_;
    uint8_t* data_;
    void init();
};

#endif
