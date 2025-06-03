#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "DXGIOptions.h"


class DXGIOptionsManager {
public:
    struct CodecInfo {
        std::string CodecId, CodecName;
        bool CanUseBitrate = true, CanUseQuality = true;
    };

    struct VideoCodecInfo : CodecInfo {
        std::string DefaultPresetId;
        IdNameArray Presets;   
    };

    struct AudioCodecInfo : CodecInfo {
        std::string DefaultQuality;
        IdNameArray Qualities;
    };

    struct AudioSource {
        std::string Name;
        int NumChannels = 2;
    };


    DXGIOptionsManager();

    std::optional<VideoCodecInfo> getVideoCodecInfo(const std::string& codecId);
    std::optional<AudioCodecInfo> getAudioCodecInfo(const std::string& codecId);
    std::optional<AudioSource> getAudioSourceInfo(const std::string& sourceId);

    IdNameArray getVideoCodecs();
    IdNameArray getAudioCodecs();
    IdNameArray getVideoSources();
    IdNameArray getAudioSources();

private:
    IdNameArray videoCodecs_, audioCodecs_;
    std::map<std::string, AudioSource> audioSources_;
};
