#ifndef ABSTRACTFRAMEGRABBER_H
#define ABSTRACTFRAMEGRABBER_H

#include "AbstractVideoFrame.h"
#include <Core/Utils/CoreTypes.h>

class AbstractFrameGrabber
{
public:
	virtual ~AbstractFrameGrabber();
	virtual bool open(const Utf8String& fileName)=0;
    virtual bool seek(int64_t time)=0;
    virtual AbstractVideoFrame* grabCurrentFrame()=0;
    virtual int64_t duration()=0;
	Utf8String error();
    void frameGrabbed(AbstractVideoFrame *frame);
    
//public slots:
    virtual void abort();

protected:
    Utf8String error_;
    
};

#endif // ABSTRACTVIDEOGRABBER_H
