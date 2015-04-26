#ifndef QTIMAGE_H
#define QTIMAGE_H

#include "AbstractImage.h"
#include <QImage>
class QtImage : public AbstractImage
{
public:
    QtImage();
    ~QtImage();
    enum DataFormat { dfRGB888, dfBitmapRgb} ;
    virtual bool loadFromFile(const Utf8String& fileName);
    virtual bool saveToFile(const Utf8String& fileName) const = 0;
    virtual bool isNull() const = 0;
    virtual bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data, size_t dataSize, void* parameter = 0 );
    QImage toQImage();
protected:
    QImage img_;
};

#endif // QTIMAGE_H
