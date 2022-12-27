#include "AbstractImage.h"

#include "Core/Logging.h"

std::function<AbstractImage* ()> AbstractImage::factory_;
std::string  AbstractImage::factoryName_;

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
    if (factory_) {
        return factory_();
    } else {
        LOG(ERROR) << "No image factory registered";
        return nullptr;
    }
}

bool AbstractImage::loadFromFile(const std::string& fileName)
{
    LOG(ERROR) << "loadFromFile not implemented";
    return false;
}

const std::string& AbstractImage::factoryName() {
    return factoryName_;
}