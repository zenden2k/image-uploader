#ifndef IU_SCREENCAPTURE_SCREENRECORDER_SCREENRECORDER_H
#define IU_SCREENCAPTURE_SCREENRECORDER_SCREENRECORDER_H

#pragma once

#include <thread>

#include "atlheaders.h"

class ScreenRecorder
{
public:
    ScreenRecorder(CString ffmpegPath, CRect rect);
    ~ScreenRecorder();
    void start();
private:
    CString ffmpegPath_;
    CRect captureRect_;
    std::thread thread_;
};

#endif