#ifndef GDIPLUSIMAGE_H
#define GDIPLUSIMAGE_H

#include <memory>

#include "AbstractImage.h"
#include "3rdpart/GdiplusH.h"

class GdiPlusImage: public AbstractImage
{
public:
    GdiPlusImage();
    explicit GdiPlusImage(Gdiplus::Bitmap *bm, bool takeOwnership  = true);
    ~GdiPlusImage() override;
    bool loadFromFile(const std::string& fileName) override;
    bool saveToFile(const std::string& fileName) const override;
    bool isNull() const override;
    bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data,size_t dataSize, void* parameter) override;
    bool loadFromRgb(int width, int height, uint8_t* data, size_t dataSize);
    Gdiplus::Bitmap* getBitmap() const;

    int getWidth() const override;
    int getHeight() const override;
protected:
    std::shared_ptr<Gdiplus::Bitmap> bm_;
    uint8_t* data_;
    void init();
    DISALLOW_COPY_AND_ASSIGN(GdiPlusImage);
};

#endif
