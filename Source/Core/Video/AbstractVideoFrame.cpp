#include "AbstractVideoFrame.h"
#include "AbstractImage.h"
#include <Core/logging.h>

AbstractVideoFrame::AbstractVideoFrame() {
}

int AbstractVideoFrame::getWidth() const {
    return width_;
}

int AbstractVideoFrame::getHeight() const {
    return height_;
}

AbstractVideoFrame::~AbstractVideoFrame() {

}

int64_t AbstractVideoFrame::getTime() const {
    return time_;
}

AbstractImage*  AbstractVideoFrame::toImage() const {
	LOG(ERROR) << "toImage not implemented";
    return NULL;
}
