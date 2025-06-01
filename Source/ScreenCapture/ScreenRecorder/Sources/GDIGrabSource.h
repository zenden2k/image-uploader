#pragma once

#include "FFmpegSource.h"
#include "../ArgsBuilder/FFmpegInputArgs.h"
#include "../FFmpegOptions.h"

class GDIGrabSource: public FFmpegSource {
public:
    inline static constexpr auto SOURCE_ID = "gdigrab";

    GDIGrabSource(): FFmpegSource("gdigrab") {
    }

    void apply(const FFmpegOptions& settings, FFmpegInputArgs& inputArgs, GlobalFFmpegArgs& globalArgs) override {
        inputArgs.addArg("f", "gdigrab");
        inputArgs.addArg("framerate", settings.framerate);
        inputArgs.addArg("offset_x", settings.offsetX);
        inputArgs.addArg("offset_y", settings.offsetY);
        inputArgs.addArg("video_size", std::to_string(settings.width) + "x" + std::to_string(settings.height));
        inputArgs.addArg("draw_mouse", settings.showCursor);
    }
}; 
