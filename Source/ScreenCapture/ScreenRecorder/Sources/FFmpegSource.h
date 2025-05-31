#pragma once

class FFmpegInputArgs;
class FFmpegOptions;

class FFmpegSource {
    virtual void apply(const FFmpegOptions& settings, FFmpegInputArgs& outputArgs, GlobalFFmpegArgs& args) = 0;
};
