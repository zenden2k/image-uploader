#include "VideoGrabber.h"

#include "AbstractFrameGrabber.h"
#ifdef _WIN32
#include "DirectshowFrameGrabber.h"
#endif
#include "AbstractVideoFrame.h"
#include "AvcodecFrameGrabber.h"
#include <Core/Utils/CoreUtils.h>
#include <zthread/Thread.h>
#include <zthread/Mutex.h>

#if defined(_WIN32) && !defined(QT_VERSION)
#include <gdiplus.h>
#endif

class VideoGrabberRunnable: public ZThread::Runnable{
public:
	VideoGrabberRunnable(VideoGrabber* videoGrabber)
	{
		videoGrabber_ = videoGrabber;
		thread_ = 0;
		currentGrabber_ = 0;
	}
	~VideoGrabberRunnable() {
		#if defined(_WIN32) && !defined(QT_VERSION)
		//Gdiplus::GdiplusShutdown(gdiplusToken_);
#endif
		thread_ = 0;
	}

	virtual void run()
	{
		AbstractFrameGrabber* grabber = videoGrabber_->createGrabber();
		if ( !grabber ) {
			return;
		}
		if ( !grabber->open(videoGrabber_->fileName_) ) {

			if ( videoGrabber_->onFinished ) {
				videoGrabber_->onFinished();
			}
			delete grabber;
			return;
		}
		currentGrabber_ = grabber;
		#if defined(_WIN32) && !defined(QT_VERSION)
			/*Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			Gdiplus::GdiplusStartup( &gdiplusToken_, &gdiplusStartupInput, NULL );*/
		#endif

		int64_t duration = grabber->duration();
		int64_t step = duration / ( videoGrabber_->frameCount_ + 1 );
		for( int i = 0; i < videoGrabber_->frameCount_; i++ ) {
			if ( thread_ && thread_->canceled() ) {
				MessageBox(0,0,0,0);
				break;
			}
			grabber->seek(/*duration / 2*/( i + 0.5 ) * step);
			AbstractVideoFrame *frame =  grabber->grabCurrentFrame();
			if ( ! frame ) {
				break;
			}
			int64_t SampleTime = frame->getTime();
			Utf8String s;
			char buffer[100];
			sprintf(buffer,"%02d:%02d:%02d",int(SampleTime / 3600), (int)(long(SampleTime) / 60) % 60,
				(int)long(long(SampleTime) % 60) );
			s = buffer;


			if ( frame && !videoGrabber_->onFrameGrabbed.empty() ) {
				videoGrabber_->onFrameGrabbed(s, SampleTime, frame->toImage());
			}
			delete frame;
		}
		delete grabber;
		if ( videoGrabber_->onFinished ) {
			videoGrabber_->onFinished();
		}
	}

	void setThread(ZThread::Thread* thread) {
		thread_ = thread;
	}
protected:
	VideoGrabber* videoGrabber_;
	AbstractFrameGrabber* currentGrabber_;
	ZThread::Thread* thread_;
	#if defined(_WIN32) && !defined(QT_VERSION)
	ULONG_PTR gdiplusToken_;
	#endif
};

VideoGrabber::VideoGrabber()
{
    videoEngine_ = veAuto;
    frameCount_ = 5;
	currentThread_ = 0;
}

VideoGrabber::~VideoGrabber()
{
	delete currentThread_;
}

void VideoGrabber::grab(const Utf8String& fileName) {
	 if ( !IuCoreUtils::FileExists(fileName) ) {
         return;
     }
	 Utf8String ext = IuCoreUtils::ExtractFileExt(fileName);
     fileName_ = fileName;
	 worker_ = new VideoGrabberRunnable(this);
     currentThread_ =  new ZThread::Thread (worker_);
	 worker_->setThread(currentThread_);
 }

void VideoGrabber::abort() {
	currentThread_->cancel();
}

bool VideoGrabber::isRunning() {
    return currentThread_;//&& currentThread_->
}

void VideoGrabber::setVideoEngine(VideoEngine engine) {
    videoEngine_ = engine;
}

VideoGrabber::VideoEngine VideoGrabber::videoEngine() const {
    return videoEngine_;
}

void VideoGrabber::setFrameCount(int frameCount) {
    frameCount_ = frameCount;
}

AbstractFrameGrabber* VideoGrabber::createGrabber() {
    AbstractFrameGrabber* grabber = NULL;
    if ( videoEngine_ == veAvcodec ) {
        return new AvcodecFrameGrabber();
    }
#ifdef _WIN32
    else {
        grabber = new DirectshowFrameGrabber();
    }
#else

#endif
    return grabber;
}
