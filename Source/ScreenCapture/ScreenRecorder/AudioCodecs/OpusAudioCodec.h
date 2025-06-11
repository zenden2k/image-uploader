#pragma once
#include <memory>
#include <string>

#include "Core/i18n/Translator.h"
#include "AudioCodec.h"
#include "ScreenCapture/ScreenRecorder/ArgsBuilder/FFmpegOutputArgs.h"
#include "ScreenCapture/ScreenRecorder/FFmpegOptions.h"

class OpusAudioCodec: public AudioCodec
{
public:
    inline static auto CODEC_ID = "opus";
    OpusAudioCodec()
        : AudioCodec("opus", "")
    {
    }
    void apply(const FFmpegOptions& settings, FFmpegOutputArgs& outputArgs) override {
        outputArgs.setAudioCodec("libopus")
            .addArg("b:a", settings.audioQuality.empty() ? defaultQuality() : settings.audioQuality);
            //.addArg("vbr", "on");*
    }
    IdNameArray qualities() const override {
        IdNameArray res;
        res.push_back({ "64k", "64 kbps" + std::string(_(" (speech)")) });
        res.push_back({ "96k", "96 kbps" + std::string(_(" (music, low)")) });
        res.push_back({ "128k", "128 kbps" + std::string(_(" (music, standard)")) });
        res.push_back({ "160k", "160 kbps" + std::string(_(" (music, high)")) });
        res.push_back({ "192k", "192 kbps" + std::string(_(" (music, very high)")) });
        res.push_back({ "256k", "256 kbps" + std::string(_(" (highest)")) });
        return res;
    }
    std::string defaultQuality() const override {
        return "128k";
    }
};
