#pragma once

#include "FFmpegArgs.h"

class FFmpegOutputArgs: public FFmpegArgs<FFmpegOutputArgs>
{
public:
    FFmpegOutputArgs(std::string file): output_(std::move(file)) {
 
    }

    std::vector<std::string> getArgs() override {
        std::vector<std::string> res = FFmpegArgsBase::getArgs();
        res.emplace_back(output_);
        return res;
    }

    FFmpegOutputArgs& setVideoSize(int width, int height) {
        return addArg("video_size", std::to_string(width) + "x" + std::to_string(height));
    }

    FFmpegOutputArgs& setFrameRate(int frameRate)
    {
        return addArg("r", frameRate);
    }

    FFmpegOutputArgs& setAudioCodec(const std::string& codec)
    {
        return addArg("c:a", codec);
    }
private:
    std::string output_;
};