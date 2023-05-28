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
private:
    std::string ffmpegCodecName_;
};
