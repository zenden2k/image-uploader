#ifndef DIRECTSHOWFRAMEGRABBER_H
#define DIRECTSHOWFRAMEGRABBER_H

#include <memory>

#include "AbstractFrameGrabber.h"

class DirectshowFrameGrabberPrivate;

class DirectshowFrameGrabber : public AbstractFrameGrabber
{
public:
    DirectshowFrameGrabber();
    ~DirectshowFrameGrabber();
    bool open(const std::string& fileName) override;
    bool seek(int64_t time) override;
    AbstractVideoFrame* grabCurrentFrame() override;
    int64_t duration() override;
    void abort() override;
protected:
    int64_t duration_;
    std::unique_ptr<DirectshowFrameGrabberPrivate> d_ptr;
    DISALLOW_COPY_AND_ASSIGN(DirectshowFrameGrabber);
};

#endif // DIRECTSHOWFRAMEGRABBER_H
