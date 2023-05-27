#ifndef IU_SCREENCAPTURE_SCREENRECORDER_SCREENRECORDER_H
#define IU_SCREENCAPTURE_SCREENRECORDER_SCREENRECORDER_H

#pragma once

#include <string>
#include <thread>
#include <future>

#include "atlheaders.h"
#include <boost/process.hpp>

class ScreenRecorder
{
public:
    ScreenRecorder(CString ffmpegPath, CRect rect);
    ~ScreenRecorder();
    void start();
    void stop();
    void pause();
    bool isRunning() const;
private:
    CString ffmpegPath_;
    CRect captureRect_;
    //std::thread thread_;
    bool isRunning_ = false;
    boost::process::opstream inStream_;
    std::unique_ptr<boost::process::child> child_;
    std::string fileNoExt_;
    std::string fileFull_;
    std::string outFilePath_;
    std::vector<std::string> parts_;
    std::future<int> future_;
    void sendStopSignal();
    void launchFfmpeg(const std::vector<std::string> args);
};


#endif