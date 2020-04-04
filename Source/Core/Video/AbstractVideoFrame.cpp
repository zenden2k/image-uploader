#include "AbstractVideoFrame.h"

#include "AbstractImage.h"
#include "Core/Logging.h"

AbstractVideoFrame::AbstractVideoFrame() : width_(0), height_(0), time_(0) {
}

int AbstractVideoFrame::getWidth() const {
    return width_;
}

int AbstractVideoFrame::getHeight() const {
    return height_;
}

int64_t AbstractVideoFrame::getTime() const {
    return time_;
}

std::unique_ptr<AbstractImage> AbstractVideoFrame::toImage() const {
    LOG(ERROR) << "toImage not implemented";
    return nullptr;
}
