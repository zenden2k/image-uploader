#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"
#include "Core/i18n/Translator.h"

class VP8VideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto CODEC_ID = "vp8";
    VP8VideoCodec()
        : FFmpegVideoCodec("WebM (VP8)", "webm", "Encode to WebM: VP8 with Opus audio")
    {
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("vcodec", "libvpx")
            .setFrameRate(Settings.framerate)
            .addArg("deadline", "realtime")
            .addArg("auto-alt-ref", "0")
            .addArg("lag-in-frames", "0");
        ;
       
        if (Settings.useQuality) {
            // quality: 63 (lowest) to 4 (highest) для VP8
            int crf = 4 + (59 * (100 - Settings.quality)) / 99;
            outputArgs.addArg("crf", crf)
                .addArg("b:v", 0);
        } else {
            outputArgs.addArg("b:v", std::to_string(Settings.bitrate) + "k");
        }
    }

    IdNameArray presets() const override {
        return {
            { "realtime", _("Real-time (fastest)") },
            { "good", _("Good (default)") },
            { "best", _("Best (slowest)") }
        };
    }
    private:
};
