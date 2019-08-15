#ifndef QTIMAGE_H
#define QTIMAGE_H

#include "AbstractImage.h"
#include <QImage>

class QtImage : public AbstractImage
{
public:
    QtImage();
    ~QtImage();
    virtual bool loadFromFile(const std::string& fileName);
    virtual bool saveToFile(const std::string& fileName) const override;
    virtual bool isNull() const override;
    virtual bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data, size_t dataSize, void* parameter = 0 ) override;
    QImage toQImage() const;
protected:
    QImage img_;
};

#endif // QTIMAGE_H
