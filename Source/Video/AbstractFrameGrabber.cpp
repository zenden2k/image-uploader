#include "AbstractFrameGrabber.h"

AbstractFrameGrabber::~AbstractFrameGrabber()
{

}

std::string AbstractFrameGrabber::error() const {
    return error_;
}

void AbstractFrameGrabber::abort() {

}

std::optional<AbstractFrameGrabber::StreamInfo> AbstractFrameGrabber::getInfo() {
    return std::nullopt;
}
