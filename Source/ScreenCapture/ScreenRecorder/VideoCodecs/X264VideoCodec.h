#pragma once
#include <memory>
#include <string>


#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegSettings.h"

class X264VideoCodec: public FFmpegVideoCodec
{
public:
    X264VideoCodec()
        : FFmpegVideoCodec("Mp4(H.264, AAC)", ".mp4", "Encode to Mp4: H.264 with AAC audio using x264 encoder.")
    {
    }

    FFmpegAudioArgsProvider* audioArgsProvider() override {
        return {};
    }
    //public override FFmpegAudioArgsProvider AudioArgsProvider = > FFmpegAudioItem.Aac;

    void apply(const FFmpegSettings& Settings, FFmpegOutputArgs& outputArgs) override
    {
        // quality: 51 (lowest) to 0 (highest)
        int crf = (51 * (100 - Settings.quality)) / 99;

        outputArgs.addArg("vcodec", "libx264")
            .addArg("crf", crf)
            .addArg("r", Settings.framerate)
            .addArg("pix_fmt", /*Settings.X264.PixelFormat*/ "yuv420p")
            .addArg("preset", /*Settings.X264.Preset*/"ultrafast")
            .addArg("movflags", /*Settings.X264.Preset*/"+faststart");

    }

    private:
    std::string ffmpegCodecName_;
};
