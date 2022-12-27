#ifndef ABSTRACTFRAMEGRABBER_H
#define ABSTRACTFRAMEGRABBER_H

#include <string>
#include "AbstractVideoFrame.h"
#include "Core/Utils/CoreTypes.h"

class AbstractFrameGrabber
{
public:
    AbstractFrameGrabber() = default;
    virtual ~AbstractFrameGrabber();
    virtual bool open(const std::string& fileName)=0;
    virtual bool seek(int64_t time)=0;
    virtual AbstractVideoFrame* grabCurrentFrame()=0;
    virtual int64_t duration()=0;
    std::string error() const;
    virtual void abort();
protected:
    std::string error_;
private:
    DISALLOW_COPY_AND_ASSIGN(AbstractFrameGrabber);
};

#endif // ABSTRACTVIDEOGRABBER_H
