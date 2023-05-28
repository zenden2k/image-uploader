#pragma once

#include "FFmpegArgs.h"

class FFmpegInputArgs: public FFmpegArgs<FFmpegInputArgs>
{
public:
    std::vector<std::string> getArgs() override {
        std::vector<std::string> res = FFmpegArgsBase::getArgs();
        if (!input_.empty()) {
            res.emplace_back("-i");
            res.emplace_back(input_);
        }
        return res;
    }
    FFmpegInputArgs(std::string file);
    FFmpegInputArgs& setVideoSize(int Width, int Height);
    FFmpegInputArgs& setFrameRate(int FrameRate);
    FFmpegInputArgs& setFormat(const std::string& format);
    FFmpegInputArgs& setAudioCodec(const std::string& Codec);
    FFmpegInputArgs& setAudioFrequency(int Frequency);
    FFmpegInputArgs& setAudioChannels(int Channels);
    FFmpegInputArgs& disableVideo();
private:
    std::string input_;
};