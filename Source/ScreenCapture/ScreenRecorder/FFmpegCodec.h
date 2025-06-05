#pragma once

#include <string>
#include <vector>

#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class FFmpegOutputArgs;
class FFmpegOptions;
class FFmpegAudioArgsProvider;

class FFmpegCodec
{
public:
    FFmpegCodec(std::string name, std::string description, bool canUseBitrate = true, bool canUseQuality = true)
        :
        name_(std::move(name))
        , description_(std::move(description))
        , canUseBitrate_(canUseBitrate)
        , canUseQuality_(canUseQuality)
    {
    }

    virtual ~FFmpegCodec() = default;

    std::string name() const {
        return name_;
    }

    std::string extension() const {
        return extension_;
    }

    std::string description() const {
        return description_;
    }

    virtual IdNameArray presets() const
    {
        return {};
    }

    virtual std::string defaultPreset() const {
        return {};
    }

    bool canUseBitrate() const {
        return canUseBitrate_;
    }

    bool canUseQuality() const {
        return canUseQuality_;
    }

    virtual void apply(const FFmpegOptions& settings, FFmpegOutputArgs& outputArgs) = 0;
protected:
    std::string name_;
    std::string extension_;
    std::string description_;
    bool canUseBitrate_, canUseQuality_;
};
