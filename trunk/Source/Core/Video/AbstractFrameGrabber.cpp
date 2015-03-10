#include "AbstractFrameGrabber.h"

AbstractFrameGrabber::~AbstractFrameGrabber()
{

}

Utf8String AbstractFrameGrabber::error() {
    return error_;
}


 void AbstractFrameGrabber::abort() {

 }
