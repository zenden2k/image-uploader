#include "QtImage.h"
#include "Core/CommonDefs.h"

QtImage::QtImage()
{

}

QtImage::~QtImage()
{

}

bool QtImage::loadFromFile(const std::string &fileName)
{
    return img_.load(U2Q(fileName));
}

bool QtImage::loadFromRawData(DataFormat dt, int width, int height, uint8_t* data, size_t dataSize, void* parameter) {
    if (dt == dfRGB888) {
        //img_.loadFromData(data, dataSize, );
        img_ = QImage((const unsigned char*)data, width, height, QImage::Format_RGB888);
        if (!img_.isNull()) {
            return true;
        }
    }
    else if (dt == dfBitmapRgb) {
        //img_ = QImage((const unsigned char*)data, width, height, QImage::Format_RGB888);
        img_.loadFromData(data, dataSize);
        if (!img_.isNull()) {
            return true;
        }
    }
    return false;
}

QImage QtImage::toQImage()
{
    return img_;
}

bool QtImage::isNull() const
{
    return img_.isNull();
}

bool QtImage::saveToFile(const std::string &fileName) const
{
    return img_.save(U2Q(fileName));
}

