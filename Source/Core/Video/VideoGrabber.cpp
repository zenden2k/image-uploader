/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

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
#include <Core/Logging.h>
class VideoGrabberRunnable: public ZThread::Runnable{
public:
	VideoGrabberRunnable(VideoGrabber* videoGrabber)
	{
		videoGrabber_ = videoGrabber;
		thread_ = 0;
		currentGrabber_ = 0;
	}
	~VideoGrabberRunnable() {
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
		int64_t duration = grabber->duration();
		int64_t step = duration / ( videoGrabber_->frameCount_ + 1 );
		for( int i = 0; i < videoGrabber_->frameCount_; i++ ) {
			if ( thread_ && thread_->canceled() ) {
				break;
			}
			int64_t curTime = ( i + 0.5 ) * step;

			
			grabber->seek(curTime);
			AbstractVideoFrame *frame =  grabber->grabCurrentFrame();
			if (!frame ) {
				grabber->seek(curTime);
				frame =  grabber->grabCurrentFrame();
			}
			if ( ! frame ) {
				LOG(WARNING) <<"grabber->grabCurrentFrame returned NULL";
				continue;
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
