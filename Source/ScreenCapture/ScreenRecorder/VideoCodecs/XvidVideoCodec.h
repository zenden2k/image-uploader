#pragma once
#include <memory>
#include <string>


#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class XvidVideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto CODEC_ID = "xvid";
    XvidVideoCodec()
        : FFmpegVideoCodec("MPEG4 (Xvid)", "avi", "Encode to AVI: Xvid with MP3 audio")
    {
    }

    std::string defaultPreset() const override {
        return "medium";
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("vcodec", "libxvid")
            .setFrameRate(Settings.framerate)
            .addArg("preset", Settings.preset.empty() ? defaultPreset() : Settings.preset);
       
        if (Settings.useQuality) {
            // quality: 31 (lowest) to 1 (highest) для Xvid
            int qscale = 1 + (30 * (100 - Settings.quality)) / 99;
            outputArgs.addArg("qscale:v", qscale);
        } else {
            outputArgs.addArg("b:v", std::to_string(Settings.bitrate) + "k");
        }
        
       // Additional parameters for better quality
        outputArgs.addArg("mbd", "2") // Better motion estimation algorithm
            .addArg("flags", "+mv4+aic") // 4MV + advanced intra coding
            .addArg("trellis", "2") // Quantization optimization
            .addArg("cmp", "2") // Comparison criterion for motion
            .addArg("subcmp", "2"); // Sub-pixel comparison criterion
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
            { "veryslow", "veryslow" }
        };
    }
    private:
};
