#include "QtImage.h"

#include <cassert>

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
        size_t oldStripeSize = width * 3;
        size_t newDataSize = oldStripeSize * height;
        uint8_t* newData;
        assert( newDataSize <= dataSize);
        try {
            newData = new uint8_t[newDataSize];
        } catch (std::exception& ex) {
            LOG(ERROR) << ex.what();
            return false;
        }
        memcpy(newData, data, newDataSize);
       
        img_ = QImage(newData, width, height, oldStripeSize, QImage::Format_RGB888, [](void* d) {
            delete[] reinterpret_cast<uint8_t*>(d);
        });
        if (!img_.isNull()) {
            return true;
        }
        delete[] newData;
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

QImage QtImage::toQImage() const
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

