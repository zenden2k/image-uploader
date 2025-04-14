#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include "AbstractFrameGrabber.h"

class AvcodecFrameGrabberPrivate;

class AvcodecFrameGrabber : public AbstractFrameGrabber
{
public:
    AvcodecFrameGrabber();
    ~AvcodecFrameGrabber() override;
    bool open(const std::string& fileName) override;
    bool seek(int64_t time) override;
    AbstractVideoFrame* grabCurrentFrame() override;
    int64_t duration() override;
    std::optional<StreamInfo> getInfo() override;

protected:
    AvcodecFrameGrabberPrivate* const d_ptr;
    DISALLOW_COPY_AND_ASSIGN(AvcodecFrameGrabber);
};

#endif // FRAMEGRABBER_H
