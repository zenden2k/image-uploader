#ifndef ABSTRACTIMAGE_H
#define ABSTRACTIMAGE_H

#include "Core/Utils/CoreTypes.h"

class AbstractImage
{
public:
    enum DataFormat { dfRGB888, dfBitmapRgb} ;
    AbstractImage();
    virtual ~AbstractImage();
    virtual bool loadFromFile(const std::string& fileName);
    virtual bool saveToFile(const std::string& fileName) const = 0;
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual bool isNull() const = 0;
    virtual bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data, size_t dataSize, void* parameter = 0 ) = 0;
    static AbstractImage* createImage();
protected:
    int width_;
    int height_;
};

#endif
