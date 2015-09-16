#ifndef IU_CORE_VIDEO_VIDEOGRABBER_H
#define IU_CORE_VIDEO_VIDEOGRABBER_H

#include "Core/Utils/CoreUtils.h"
#include "Core/3rdpart/FastDelegate.h"

class AbstractFrameGrabber;
class VideoGrabberRunnable;
class AbstractImage;

class VideoGrabber
{
public:
    enum VideoEngine { veAuto = 0, veDirectShow, veAvcodec };
    explicit VideoGrabber();
    ~VideoGrabber();
    void setVideoEngine(VideoEngine engine);
    VideoEngine videoEngine() const;
    void grab(const std::string& fileName);
    void abort();
    bool isRunning();
    void setFrameCount(int frameCount);
    fastdelegate::FastDelegate3<const std::string&, int64_t, AbstractImage*> onFrameGrabbed;
    fastdelegate::FastDelegate0<void> onFinished;
private:
    std::string fileName_;
    //void _frameGrabbed(const std::string& fileName, AbstractImage image);
    AbstractFrameGrabber* createGrabber();
    VideoEngine videoEngine_;
    int frameCount_;
    friend class VideoGrabberRunnable;
    std::unique_ptr<VideoGrabberRunnable> worker_;
    DISALLOW_COPY_AND_ASSIGN(VideoGrabber);
};

#endif // VIDEOGRABBER_H
