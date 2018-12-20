#ifndef DIRECTSHOWFRAMEGRABBER_H
#define DIRECTSHOWFRAMEGRABBER_H

#include "AbstractFrameGrabber.h"

class DirectshowFrameGrabberPrivate;

class DirectshowFrameGrabber : public AbstractFrameGrabber
{
public:
    DirectshowFrameGrabber();
    ~DirectshowFrameGrabber();
    virtual bool open(const std::string& fileName) override;
    virtual bool seek(int64_t time) override;
    virtual AbstractVideoFrame* grabCurrentFrame() override;
    virtual int64_t duration() override;
    virtual void abort() override;
protected:
    int64_t duration_;
protected:
  DirectshowFrameGrabberPrivate * const d_ptr;
};

#endif // DIRECTSHOWFRAMEGRABBER_H
