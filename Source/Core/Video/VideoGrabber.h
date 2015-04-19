#ifndef IU_CORE_VIDEO_VIDEOGRABBER_H
#define IU_CORE_VIDEO_VIDEOGRABBER_H

#include "Core/Utils/CoreUtils.h"
#include "Core/3rdpart/FastDelegate.h"

class AbstractFrameGrabber;
class VideoGrabberRunnable;
class AbstractImage;

namespace ZThread {
	class Thread;
};

class VideoGrabber
{
public:
    enum VideoEngine { veAuto = 0, veDirectShow, veAvcodec };
    explicit VideoGrabber();
	~VideoGrabber();
    void setVideoEngine(VideoEngine engine);
    VideoEngine videoEngine() const;
    void grab(const Utf8String& fileName);
    void abort();
    bool isRunning();
    void setFrameCount(int frameCount);

	fastdelegate::FastDelegate3<const Utf8String&, int64_t, AbstractImage*> onFrameGrabbed;
	fastdelegate::FastDelegate0<void> onFinished;

    void run();
private:
    Utf8String fileName_;
    //void _frameGrabbed(const Utf8String& fileName, AbstractImage image);
    AbstractFrameGrabber* createGrabber();
    ZThread::Thread* currentThread_;
    VideoEngine videoEngine_;
    int frameCount_;
	friend class VideoGrabberRunnable;
	VideoGrabberRunnable* worker_;
};

#endif // VIDEOGRABBER_H
