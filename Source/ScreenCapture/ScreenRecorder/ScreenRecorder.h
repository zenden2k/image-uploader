#ifndef IU_SCREENCAPTURE_SCREENRECORDER_SCREENRECORDER_H
#define IU_SCREENCAPTURE_SCREENRECORDER_SCREENRECORDER_H

#pragma once

#include <string>
#include <thread>
#include <future>
#include <functional>

#include "atlheaders.h"
#include <boost/process.hpp>

#include <boost/signals2.hpp>

class ScreenRecorder
{
public:
    enum class Status
    {
        Invalid, Recording, Paused, RunningConcatenation, Finished, Canceled
    };
    ScreenRecorder(CString ffmpegPath, CRect rect);
    ~ScreenRecorder();
    void start();
    void stop();
    void pause();
    void cancel();
    bool isRunning() const;
    void setOffset(int x, int y);
    Status status() const;
    std::string outFileName() const;

    template<typename F>
    boost::signals2::connection addStatusChangeCallback(F&& f) {
        return onStatusChange_.connect(std::forward<F>(f));
    }

private:
    CString ffmpegPath_;
    CRect captureRect_;
    //std::thread thread_;
    bool isRunning_ = false;
    std::unique_ptr<boost::process::opstream> inStream_;
    std::unique_ptr<boost::process::child> child_;
    std::string fileNoExt_;
    std::string fileFull_;
    std::string outFilePath_;
    std::vector<std::string> parts_;
    std::future<int> future_;
    Status status_;
    boost::signals2::signal<void(Status)> onStatusChange_;
    void sendStopSignal();
    std::future<int> launchFFmpeg(const std::vector<std::string> args, std::function<void(int)> onFinish);
    void changeStatus(Status status);
    void cleanupAfter();

};


#endif