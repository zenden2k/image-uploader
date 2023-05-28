#pragma once

#include <string>

class FFmpegOutputArgs;
class FFmpegSettings;
class FFmpegAudioArgsProvider;

class FFmpegVideoCodec
{
public:
    FFmpegVideoCodec(std::string name, std::string extension, std::string description):
        name_(std::move(name)),
        extension_(std::move(extension)),
        description_(std::move(description))
    {
    }

    std::string name() const {
        return name_;
    }

    std::string extension() const {
        return extension_;
    }

    std::string description() const {
        return description_;
    }

    virtual void apply(const FFmpegSettings& settings, FFmpegOutputArgs& outputArgs) = 0;
    virtual FFmpegAudioArgsProvider* audioArgsProvider() = 0;
private:
    std::string name_;
    std::string extension_;
    std::string description_;
};
