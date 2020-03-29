#include "AbstractImage.h"

#ifdef IU_QT
    #include "QtImage.h"
#elif defined(_WIN32)
    #include "GdiPlusImage.h"
#endif
#include "Core/Logging.h"

AbstractImage::AbstractImage(): width_(0), height_(0) {
}

int AbstractImage::getWidth() const {
    return width_;
}

int AbstractImage::getHeight() const {
    return height_;
}

AbstractImage* AbstractImage::createImage()
{
    #ifdef IU_QT
        return new QtImage();
    #elif defined(_WIN32)
        return new GdiPlusImage();
    #else
        LOG(ERROR) << "No suitable image class found";
        return 0;
    #endif
}

bool AbstractImage::loadFromFile(const std::string& fileName)
{
    LOG(ERROR) << "loadFromFile not implemented";
    return false;
}
