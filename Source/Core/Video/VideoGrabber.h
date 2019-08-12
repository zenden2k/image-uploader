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
    enum VideoEngine { veAuto = 0, veDirectShow, veAvcodec, veDirectShow2  };
    explicit VideoGrabber();
    ~VideoGrabber();
    void setVideoEngine(VideoEngine engine);
    VideoEngine videoEngine() const;
    void grab(const std::string& fileName);
    void abort();
    bool isRunning() const;
    void setFrameCount(int frameCount);
    using FrameGrabbedCallback = std::function<void(const std::string&, int64_t, std::shared_ptr<AbstractImage>)>;
    using VoidCallback = std::function<void()>;
    void setOnFrameGrabbed(FrameGrabbedCallback cb);
    void setOnFinished(VoidCallback cb);
private:
    std::string fileName_;
    std::unique_ptr<AbstractFrameGrabber> createGrabber();
    VideoEngine videoEngine_;
    int frameCount_;
    friend class VideoGrabberRunnable;
    std::unique_ptr<VideoGrabberRunnable> worker_;
    FrameGrabbedCallback onFrameGrabbed_;
    VoidCallback onFinished_;
    DISALLOW_COPY_AND_ASSIGN(VideoGrabber);
};

#endif // VIDEOGRABBER_H
