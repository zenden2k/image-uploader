#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include "AbstractFrameGrabber.h"

class AvcodecFrameGrabberPrivate;

class AvcodecFrameGrabber: public AbstractFrameGrabber {
    public:
        AvcodecFrameGrabber();
        ~AvcodecFrameGrabber();
        bool open(const Utf8String& fileName);
        bool seek(int64_t time);
        AbstractVideoFrame* grabCurrentFrame();
        int64_t duration();
protected:
    AvcodecFrameGrabberPrivate * const d_ptr;

};

#endif // FRAMEGRABBER_H
