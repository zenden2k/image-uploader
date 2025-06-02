#pragma once
#include <memory>
#include <string>

#include "../FFmpegCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class AudioCodec: public FFmpegCodec
{
public:
    inline static auto CODEC_ID = "aac";

    AudioCodec(std::string name, std::string description, bool canUseBitrate = true, bool canUseQuality = true)
        : FFmpegCodec(std::move(name), std::move(description), canUseBitrate, canUseQuality)
    {
    }

    virtual IdNameArray qualities() const {
        return {};
    }

    virtual std::string defaultQuality() const {
        return {};
    }

    private:
};
