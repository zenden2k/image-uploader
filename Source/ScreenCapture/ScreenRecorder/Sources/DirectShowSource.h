#pragma once

#include "FFmpegSource.h"
#include "../ArgsBuilder/FFmpegInputArgs.h"
#include "../FFmpegOptions.h"

class DirectShowSource: public FFmpegSource {
public:
    inline static constexpr auto SOURCE_ID = "directshow";

    DirectShowSource(std::string deviceId)
        : FFmpegSource("directshow", true)
        ,deviceId_(deviceId) {
    }

    void apply(const FFmpegOptions& settings, FFmpegInputArgs& inputArgs, GlobalFFmpegArgs& globalArgs) override {
        std::string input = "video=\"@device_pnp_" + deviceId_ + "\"";

        if (!settings.audioDevice.empty()) {
            input += ":audio=\"" + settings.audioDevice + "\"";
        }

        inputArgs.addArg("f", "dshow")
            .addArg("i", input);
    }

private:
    std::string deviceId_;
}; 
