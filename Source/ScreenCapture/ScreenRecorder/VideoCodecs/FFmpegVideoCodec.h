#pragma once

#include <string>
#include <vector>

class FFmpegOutputArgs;
class FFmpegOptions;
class FFmpegAudioArgsProvider;

class FFmpegVideoCodec
{
public:
    FFmpegVideoCodec(std::string name, std::string extension, std::string description, bool canUseBitrate = true, bool canUseQuality = true)
        :
        name_(std::move(name)),
        extension_(std::move(extension)), description_(std::move(description))
        , canUseBitrate_(canUseBitrate)
        , canUseQuality_(canUseQuality)
    {
    }

    virtual ~FFmpegVideoCodec() = default;

    std::string name() const {
        return name_;
    }

    std::string extension() const {
        return extension_;
    }

    std::string description() const {
        return description_;
    }

    virtual std::vector<std::pair<std::string,std::string>> presets() const
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
    virtual FFmpegAudioArgsProvider* audioArgsProvider() = 0;
private:
    std::string name_;
    std::string extension_;
    std::string description_;
    bool canUseBitrate_, canUseQuality_;
};
