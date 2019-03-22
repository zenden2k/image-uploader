/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include <atomic>
#include <boost/format.hpp>

#include "AbstractFrameGrabber.h"
#ifdef _WIN32
#include "DirectshowFrameGrabber.h"
#endif
#include "AbstractVideoFrame.h"
#include "AvcodecFrameGrabber.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Logging.h"

class VideoGrabberRunnable {
public:
    VideoGrabberRunnable(VideoGrabber* videoGrabber)
    {
        videoGrabber_ = videoGrabber;
        canceled_ = false;
        isRunning_ = false;
    }

    virtual ~VideoGrabberRunnable() {
    }

    void cancel()
    {
        canceled_ = true;
    }

    virtual void run()
    {
        isRunning_ = true;
        if ( !IuCoreUtils::FileExists(videoGrabber_->fileName_) ) {
            LOG(ERROR) << "File "<<videoGrabber_->fileName_<< "not found";
            if ( videoGrabber_->onFinished ) {
                videoGrabber_->onFinished();
            }
            isRunning_ = false;
            return;
        }
        auto grabber = videoGrabber_->createGrabber();
        if ( !grabber ) {
            return;
        }
        if ( !grabber->open(videoGrabber_->fileName_) ) {

            if ( videoGrabber_->onFinished ) {
                videoGrabber_->onFinished();
            }
            isRunning_ = false;
            return;
        }

        int64_t duration = grabber->duration();
        int64_t step = duration / ( videoGrabber_->frameCount_ + 1 );
        for( int i = 0; i < videoGrabber_->frameCount_; i++ ) {
            if ( canceled_) {
                break;
            }
            int64_t curTime = static_cast<int64_t>(( i + 0.5 ) * step);
            
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
            int hours = int(SampleTime / 3600);
            int minutes = int((long(SampleTime) / 60) % 60);
            int seconds = int(long(long(SampleTime) % 60));
            std::string s = str(boost::format("%02d:%02d:%02d") % hours % minutes % seconds);

            if ( /*frame && */!videoGrabber_->onFrameGrabbed.empty() ) {
                videoGrabber_->onFrameGrabbed(s, SampleTime, frame->toImage());
            }
            delete frame;
        }
        grabber.reset();
        if ( videoGrabber_->onFinished ) {
            videoGrabber_->onFinished();
        }
        isRunning_ = false;
    }

    bool isRunning() const
    {
        return isRunning_;
    }

protected:
    VideoGrabber* videoGrabber_;
    std::atomic<bool> canceled_;
    std::atomic<bool> isRunning_;
};

VideoGrabber::VideoGrabber()
{
    videoEngine_ = veAuto;
    frameCount_ = 5;
}

VideoGrabber::~VideoGrabber()
{
}

void VideoGrabber::grab(const std::string& fileName) {
     if ( !IuCoreUtils::FileExists(fileName) ) {
         return;
     }
     std::string ext = IuCoreUtils::ExtractFileExt(fileName);
     fileName_ = fileName;
     worker_.reset(new VideoGrabberRunnable(this));
     std::thread t1(&VideoGrabberRunnable::run, worker_.get());
     t1.detach();
 }

void VideoGrabber::abort() {
    worker_->cancel();
}

bool VideoGrabber::isRunning() {
    return worker_->isRunning();
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

std::unique_ptr<AbstractFrameGrabber> VideoGrabber::createGrabber() {
    std::unique_ptr<AbstractFrameGrabber> grabber;
#ifdef _WIN32
    if ( videoEngine_ == veAvcodec ) {
        grabber.reset(new AvcodecFrameGrabber());
    } else {
        grabber.reset(new DirectshowFrameGrabber());
    }
#else
    grabber.reset(new AvcodecFrameGrabber());
#endif
    return grabber;
}
