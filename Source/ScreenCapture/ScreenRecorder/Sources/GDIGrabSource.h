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
        inputArgs.addArg("f", "gdigrab")
            .addArg("framerate", settings.framerate)
            .addArg("offset_x", settings.offsetX)
            .addArg("offset_y", settings.offsetY)
            .addArg("video_size", std::to_string(settings.width) + "x" + std::to_string(settings.height))
            .addArg("draw_mouse", settings.showCursor)
            .addArg("i", "desktop");
    }
}; 
