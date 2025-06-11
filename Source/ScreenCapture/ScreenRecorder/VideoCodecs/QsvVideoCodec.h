#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class QsvVideoCodec: public FFmpegVideoCodec
{
public:
    inline static auto H264_CODEC_ID = "h264_qsv";
    inline static auto HEVC_CODEC_ID = "hevc_qsv";

    QsvVideoCodec(std::string name, std::string fFmpegCodecName, std::string description)
        : FFmpegVideoCodec(std::move(name), "mp4", std::move(description), true, false),
        ffmpegCodecName_(std::move(fFmpegCodecName))
    {
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.addArg("c:v", ffmpegCodecName_)
            .setFrameRate(Settings.framerate)
            .addArg("b:v", std::to_string(Settings.bitrate) + "k")
            .addArg("preset", Settings.preset.empty() ? defaultPreset() : Settings.preset);
    }

    static std::unique_ptr<QsvVideoCodec> createH264() {
        return std::make_unique<QsvVideoCodec>("H.264 (Intel Quick Sync)", H264_CODEC_ID, "");
    }

    static std::unique_ptr<QsvVideoCodec> createHevc() {
        return std::make_unique<QsvVideoCodec>("HEVC (Intel Quick Sync)", HEVC_CODEC_ID, "");
    }

    IdNameArray presets() const override {
        return {
            { "6", "faster" },
            { "5", "fast" },
            { "4", "medium" },
            { "3", "slow" },
            { "2", "slower" },
            { "1", "veryslow" }
        };
    }

    std::string defaultPreset() const override {
        return "5";
    }

private:
    std::string ffmpegCodecName_;
};
