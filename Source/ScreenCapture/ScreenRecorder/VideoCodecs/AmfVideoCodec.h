#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class AmfVideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto H264_CODEC_ID = "h264_amf";
    inline static auto HEVC_CODEC_ID = "hevc_amf";

    AmfVideoCodec(std::string name, std::string fFmpegCodecName, std::string description)
        : FFmpegVideoCodec(std::move(name), "mp4", std::move(description), true, false),
        ffmpegCodecName_(std::move(fFmpegCodecName))
    {
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("c:v", ffmpegCodecName_)
            .setFrameRate(Settings.framerate)
            .addArg("b:v", std::to_string(Settings.bitrate) + "k")
            .addArg("usage", "lowlatency")
            .addArg("pix_fmt", "yuv420p")
            .addArg("quality", Settings.preset.empty() ? defaultPreset() : Settings.preset);
    }

    static std::unique_ptr<AmfVideoCodec> createH264() {
        return std::make_unique<AmfVideoCodec>("H.264 (AMD AMF)", H264_CODEC_ID, "");
    }

    static std::unique_ptr<AmfVideoCodec> createHevc() {
        return std::make_unique<AmfVideoCodec>("HEVC (AMD AMF)", HEVC_CODEC_ID, "");
    }

    std::vector<std::pair<std::string, std::string>> presets() const override {
        return {
            { "speed", "Prefer speed" },
            { "balanced", "Balanced" }, 
            { "quality", "Prefer quality" }
        };
    }

    std::string defaultPreset() const override {
        return "speed";
    }

private:
    std::string ffmpegCodecName_;
};
