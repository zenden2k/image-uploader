#pragma once
#include <memory>
#include <string>


#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class WebPVideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto CODEC_ID = "webp";
    WebPVideoCodec()
        : FFmpegVideoCodec("WebP", "webp", "Encode to WebP: Animated WebP format")
    {
    }

    std::string defaultPreset() const override {
        return "default";
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("vcodec", "libwebp")
            .setFrameRate(Settings.framerate)
            .addArg("preset", Settings.preset.empty() ? defaultPreset() : Settings.preset);
       
        if (Settings.useQuality) {
            // quality: 0 (lowest) to 100 (highest) for WebP
            int quality = Settings.quality;
            outputArgs.addArg("quality", quality);
        } else {
            outputArgs.addArg("b:v", std::to_string(Settings.bitrate) + "k");
        }
        
        // Additional parameters for WebP optimization
        outputArgs.addArg("lossless", "0")      // Use lossy compression
                  .addArg("compression_level", "4") // Compression effort (0-6)
                  .addArg("method", "4")        // Compression method (0-6)
                  .addArg("loop", "0");         // Infinite loop for animation
    }

    IdNameArray presets() const override {
        return {
            { "default", "default" },
            { "picture", "picture" },
            { "photo", "photo" },
            { "drawing", "drawing" },
            { "icon", "icon" },
            { "text", "text" }
        };
    }
    private:
};
