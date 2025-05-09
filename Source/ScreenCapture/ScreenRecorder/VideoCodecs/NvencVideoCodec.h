#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegSettings.h"

class NvencVideoCodec: public FFmpegVideoCodec
{
public:
    NvencVideoCodec(std::string name, std::string fFmpegCodecName, std::string description)
        : FFmpegVideoCodec(std::move(name), ".mp4", std::move(description)),
        ffmpegCodecName_(std::move(fFmpegCodecName))
    {
    }

    FFmpegAudioArgsProvider* audioArgsProvider() override {
        return {};
    }
    //public override FFmpegAudioArgsProvider AudioArgsProvider = > FFmpegAudioItem.Aac;

    void apply(const FFmpegSettings& Settings, FFmpegOutputArgs& outputArgs) override
    {
        outputArgs.addArg("c:v", ffmpegCodecName_)
            .addArg("cq:v", 20)
            .addArg("pixel_format", "yuv444p")
            .addArg("preset", "fast")
            .addArg("movflags", /*Settings.X264.Preset*/"+faststart");
    }

    static std::unique_ptr<NvencVideoCodec> createH264()
    {
        return std::make_unique<NvencVideoCodec>("NVenc: Mp4 (H.264, AAC)", "h264_nvenc", "Encode to Mp4: H.264 with AAC audio using NVenc");
    }

    static std::unique_ptr<NvencVideoCodec> createHevc()
    {
        return std::make_unique<NvencVideoCodec>("NVenc: Mp4 (HEVC, AAC)", "hevc_nvenc", "Encode to Mp4: HEVC with AAC audio using NVenc");
    }

     std::vector<std::pair<std::string, std::string>> presets() const override
    {
        return {
            { "0", "default" }, // default
            { "1", "slow" }, // hq 2 passes
            { "2", "medium" }, // hq 1 pass
            { "3", "fast" }, // hp 1 pass
            { "4", "hp" }, // high performance
            { "5", "hq" }, // high quality
            { "6", "bd" }, // Blu-ray disc encoding
            { "7", "ll" }, // low latency
            { "8", "llhq" }, // low latency high quality
            { "9", "llhp" }, // low latency high performance
            { "10", "lossless" }, // lossless encoding
            { "11", "losslesshp" }, // lossless high performance
            { "12", "p1" }, // fastest (lowest quality)
            { "13", "p2" }, // faster (lower quality)
            { "14", "p3" }, // fast (low quality)
            { "15", "p4" }, // medium (default)
            { "16", "p5" }, // slow (good quality)
            { "17", "p6" }, // slower (better quality)
            { "18", "p7" } // slowest (best quality)
        };
    }

private:
    std::string ffmpegCodecName_;
};
