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
        if (inputArgs.isSourceApplied(SOURCE_ID)) {
            return;
        }
        std::string input;
       
        if (!settings.source.empty()) {
            const auto [id, deviceId] = parseSourceId(settings.source);
            if (!deviceId.empty()) {
                input = "video=" + deviceId + "";
            }
        }

        if (!settings.audioSource.empty()) {
            const auto [id, deviceId] = parseSourceId(settings.audioSource);
            if (!deviceId.empty()) {
                if (!input.empty()) {
                    input += ":";
                }
                input += "audio=" + deviceId;
                globalArgs.addArg("audio_buffer_size", 80);
            }
        }

        if (input.empty()) {
            return;
        }
        inputArgs.addArg("f", "dshow")
            .addArg("i", input);

        inputArgs.setSourceApplied(SOURCE_ID);
    }

private:
    std::string deviceId_;
}; 
