#pragma once

class FFmpegInputArgs;
class FFmpegSettings;

class FFmpegSource {
    virtual void apply(const FFmpegSettings& settings, FFmpegInputArgs& outputArgs, GlobalFFmpegArgs& args) = 0;
};