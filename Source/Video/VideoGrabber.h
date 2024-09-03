#ifndef IU_CORE_VIDEO_VIDEOGRABBER_H
#define IU_CORE_VIDEO_VIDEOGRABBER_H

#include "Core/Utils/CoreUtils.h"

class AbstractFrameGrabber;
class VideoGrabberRunnable;
class AbstractImage;

class VideoGrabber
{
public:
    enum VideoEngine { veAuto = 0, veDirectShow, veAvcodec, veDirectShow2  };
    explicit VideoGrabber(bool async = true, bool logErrors = true);
    ~VideoGrabber();
    void setVideoEngine(VideoEngine engine);
    VideoEngine videoEngine() const;
    void grab(const std::string& fileName);
    void abort();
    bool isRunning() const;
    void setFrameCount(int frameCount);
    using FrameGrabbedCallback = std::function<void(const std::string&, int64_t, std::shared_ptr<AbstractImage>)>;
    using FinishCallback = std::function<void(bool)>;
    void setOnFrameGrabbed(FrameGrabbedCallback cb);
    void setOnFinished(FinishCallback cb);
private:
    std::string fileName_;
    std::unique_ptr<AbstractFrameGrabber> createGrabber();
    VideoEngine videoEngine_;
    int frameCount_;
    friend class VideoGrabberRunnable;
    std::unique_ptr<VideoGrabberRunnable> worker_;
    FrameGrabbedCallback onFrameGrabbed_;
    FinishCallback onFinished_;
    bool async_, logErrors_;
    DISALLOW_COPY_AND_ASSIGN(VideoGrabber);
};

#endif // VIDEOGRABBER_H
