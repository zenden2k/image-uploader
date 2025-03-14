#include "QtImage.h"

#include <cassert>
#include <QDebug>

#include "Core/Logging.h"
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
    if (dt == dfRGB888 || dt == dfRGB32bpp) {
        size_t oldStripeSize = reinterpret_cast<size_t>(parameter);
        size_t newDataSize = oldStripeSize * height;
        uint8_t* newData{};
        assert(newDataSize <= dataSize);
        try {
            newData = new uint8_t[newDataSize];
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
            return false;
        }
        memcpy(newData, data, newDataSize);
        std::unordered_map<DataFormat, QImage::Format> mapping = {
            {dfRGB888, QImage::Format_RGB888},
            {dfRGB32bpp,QImage::Format_RGB32 }
        };

        auto it = mapping.find(dt);
        if (it == mapping.end()) {
            LOG(ERROR) << "Image format not supported";
        }
        img_ = QImage(newData, width, height, oldStripeSize, it->second, [](void* d) {
            delete[] static_cast<uint8_t*>(d);
        }, newData);
        /*img_.save("/home/user/test.jpg");
        return true;*/
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

