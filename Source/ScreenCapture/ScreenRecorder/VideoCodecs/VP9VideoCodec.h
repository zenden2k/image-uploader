#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class VP9VideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto CODEC_ID = "vp9";

    VP9VideoCodec()
        : FFmpegVideoCodec("WebM (Vp9)", "webm", "Encode to WebM: Vp9 with Opus audio")
    {
    }

    //public override FFmpegAudioArgsProvider AudioArgsProvider => FFmpegAudioItem.Opus;

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("vcodec", "libvpx-vp9")
            .addArg("deadline", "realtime");
       
        if (Settings.useQuality) {
            // quality: 63 (lowest) to 0 (highest)
            int crf = (63 * (100 - Settings.quality)) / 99;
            outputArgs.addArg("crf", crf)
                .addArg("b:v", 0);
        } else {
            outputArgs.addArg("b:v", std::to_string(Settings.bitrate) + "k");
        }

    }

    std::vector<std::pair<std::string, std::string>> presets() const override {
        return {};
    }

    private:
};
