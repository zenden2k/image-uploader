#pragma once
#include <memory>
#include <string>


#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class X264VideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto CODEC_ID = "x264";

    X264VideoCodec()
        : FFmpegVideoCodec("H.264", "mp4", "Encode to Mp4: H.264 with AAC audio using x264 encoder.")
    {
    }

    std::string defaultPreset() const override {
        return "fast";
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("vcodec", "libx264");

        if (Settings.useQuality) {
            // quality: 51 (lowest) to 0 (highest)
            int crf = (51 * (100 - Settings.quality)) / 99;
            outputArgs.addArg("crf", crf);
        } else {
            outputArgs.addArg("b:v", std::to_string(Settings.bitrate) + "k");
        }

        outputArgs.setFrameRate(Settings.framerate)
            .addArg("pix_fmt", /*Settings.X264.PixelFormat*/ "yuv420p")
            .addArg("preset", Settings.preset.empty() ? defaultPreset() : Settings.preset)
            .addArg("movflags", /*Settings.X264.Preset*/"+faststart");
    }

    IdNameArray presets() const override {
        return {
            { "ultrafast", "ultrafast" },
            { "superfast", "superfast" },
            { "veryfast", "veryfast" },
            { "faster", "faster" },
            { "fast", "fast" },
            { "medium", "medium" },
            { "slow", "slow" },
            { "slower", "slower" },
            { "veryslow", "veryslow" },
            { "placebo", "placebo" }
        };
    }

    private:
};
