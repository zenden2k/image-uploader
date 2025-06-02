#pragma once
#include <memory>
#include <string>

#include "Core/i18n/Translator.h"
#include "AudioCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class Mp3AudioCodec: public AudioCodec
{
public:
    inline static auto CODEC_ID = "mp3";

    Mp3AudioCodec()
        : AudioCodec("mp3", "")
    {
    }

    void apply(const FFmpegOptions& settings, FFmpegOutputArgs& outputArgs) override {
        // quality: 9 (lowest) to 0 (highest)
        // var qscale = (100 - Quality) / 11;

        outputArgs.setAudioCodec("libmp3lame")
             .addArg("qscale:a", settings.audioQuality.empty() ? defaultQuality() : settings.audioQuality);
    }

    IdNameArray qualities() const override {
        IdNameArray res;
        for (int i = 9; i >= 0; i--) {
            std::string tip;
            if (i == 9) {
                tip = _(" (lowest)");
            } else if (i == 0) {
                tip = _(" (highest)");
            }
            res.push_back({ std::to_string(i), std::to_string(i) + tip });
        }
        return res;
    }

    virtual std::string defaultQuality() const {
        return "5";
    }
};
