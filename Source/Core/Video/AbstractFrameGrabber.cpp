#include "AbstractFrameGrabber.h"

AbstractFrameGrabber::~AbstractFrameGrabber()
{

}

std::string AbstractFrameGrabber::error() {
    return error_;
}


 void AbstractFrameGrabber::abort() {

 }
