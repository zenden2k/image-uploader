#ifndef QTIMAGE_H
#define QTIMAGE_H

#include "AbstractImage.h"
#include <QImage>

class QtImage : public AbstractImage
{
public:
    QtImage();
    ~QtImage();
    bool loadFromFile(const std::string& fileName) override;
    bool saveToFile(const std::string& fileName) const override;
    bool isNull() const override;
    bool loadFromRawData(DataFormat dt, int width, int height, uint8_t* data, size_t dataSize, void* parameter) override;
    QImage toQImage() const;
protected:
    QImage img_;
};

#endif // QTIMAGE_H
