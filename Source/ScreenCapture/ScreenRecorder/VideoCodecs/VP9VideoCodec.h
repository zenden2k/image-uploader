#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegSettings.h"

class VP9VideoCodec: public FFmpegVideoCodec
{
public:
    VP9VideoCodec()
        : FFmpegVideoCodec("WebM (Vp9, Opus)", ".webm", "Encode to WebM: Vp9 with Opus audio")
    {
    }

    FFmpegAudioArgsProvider* audioArgsProvider() override {
        return {};
    }

    //public override FFmpegAudioArgsProvider AudioArgsProvider => FFmpegAudioItem.Opus;

    void apply(const FFmpegSettings& Settings, FFmpegOutputArgs& outputArgs) override {
        // quality: 63 (lowest) to 0 (highest)
        int crf = (63 * (100 - Settings.quality)) / 99;

        outputArgs.addArg("vcodec", "libvpx-vp9")
            .addArg("crf", crf)
            .addArg("b:v", 0);
    }

    std::vector<std::pair<std::string, std::string>> presets() const override {
        return {};
    }

    private:
};
