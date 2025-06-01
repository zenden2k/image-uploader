#pragma once

#include <boost/format.hpp>

#include "FFmpegSource.h"
#include "../ArgsBuilder/FFmpegInputArgs.h"
#include "../FFmpegOptions.h"

class DDAGrabSource: public FFmpegSource {
public:
    inline static constexpr auto SOURCE_ID = "ddagrab";

    DDAGrabSource(): FFmpegSource("ddagrab")
    {
    }

    void apply(const FFmpegOptions& settings, FFmpegInputArgs& inputArgs, GlobalFFmpegArgs& globalArgs) override {
        bool hwEncoder = settings.codec.find("_nvenc") != std::string::npos;

        if (hwEncoder) {
            inputArgs.addArg("init_hw_device", "d3d11va");
        }

        std::string filterComplex = boost::str(boost::format("ddagrab=video_size=%dx%d:offset_x=%d:offset_y=%d") % settings.width % settings.height
            % settings.offsetX % settings.offsetY);

        if (!hwEncoder) {
            filterComplex += ",hwdownload,format=bgra";
        }

        inputArgs.addArg("filter_complex", filterComplex);
            std::vector<std::string> args = {
                   // "-hide_banner",
                    "init_hw_device", "d3d11va",
                    //"-framerate","60",
                    /*"-offset_x",std::to_string(captureRect_.left),
                    "-offset_y",std::to_string(captureRect_.top),
                    "-video_size",std::to_string(captureRect_.Width())+"x" + std::to_string(captureRect_.Height()),*/
                   
                    /*,video_size = "+ std::to_string(captureRect_.Width()) + "x" + std::to_string(captureRect_.Height())
                    + ",offset_x=" + std::to_string(captureRect_.left),
                    + ",offset_y=" + std::to_string(captureRect_.top),**/

                /*-c:v", "h264_nvenc",
                    "-cq:v", "20", outFilePath_*/
        };
    }
}; 
