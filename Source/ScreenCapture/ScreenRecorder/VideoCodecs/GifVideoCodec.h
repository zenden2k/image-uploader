#pragma once
#include <memory>
#include <string>

#include "FFmpegVideoCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class GifVideoCodec : public FFmpegVideoCodec {
public:
    inline static auto CODEC_ID = "gif";

    GifVideoCodec()
        : FFmpegVideoCodec("GIF", "mp4", "", false, false){
        //name_ = "GIF";
    }

    void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override
    {
        outputArgs.addArg("vcodec", "libx264");

        /* if (Settings.useQuality) {
            // quality: 51 (lowest) to 0 (highest)
            int crf = (51 * (100 - Settings.quality)) / 99;
            outputArgs.addArg("crf", crf);
        } else {
            outputArgs.addArg("b:v", std::to_string(Settings.bitrate) + "k");
        }*/
        outputArgs.addArg("qp", 0);
        outputArgs.setFrameRate(Settings.framerate)
            .addArg("pix_fmt", "yuv420p")
            .addArg("preset", "fast")
            .addArg("movflags", "+faststart");
    }


    /*void apply(const FFmpegOptions& Settings, FFmpegOutputArgs& outputArgs) override
    {
        outputArgs.addArg("c:v", "gif")
            .addArg("r", Settings.framerate);
            //.addArg("filter_complex", "[0:v] framestep=2,split [a][b];[a] palettegen=stats_mode=diff [p];[b][p] paletteuse=dither=bayer:bayer_scale=5:diff_mode=rectangle");
    }

    std::vector<std::pair<std::string, std::string>> presets() const override {
        return {};
    }*/

    private:
};
