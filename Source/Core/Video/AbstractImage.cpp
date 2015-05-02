#include "AbstractImage.h"

#ifdef QT_VERSION 
    #include "QtImage.h"
#elif defined(_WIN32)
    #include "GdiPlusImage.h"
#endif
#include "Core/logging.h"

AbstractImage::AbstractImage() {
}

int AbstractImage::getWidth() const {
    return width_;
}

int AbstractImage::getHeight() const {
    return height_;
}

AbstractImage* AbstractImage::createImage()
{
    #ifdef QT_VERSION 
        return new QtImage();
    #elif defined(_WIN32)
        return new GdiPlusImage();
    #else
        LOG(ERROR) << "No suitable image class found";
    #endif
    return 0;
}

AbstractImage::~AbstractImage() {

}

bool AbstractImage::loadFromFile(const std::string& fileName)
{
    LOG(ERROR) << "loadFromFile not implemented";
    return false;
}
