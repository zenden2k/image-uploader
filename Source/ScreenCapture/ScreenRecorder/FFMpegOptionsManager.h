#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

class FFmpegVideoCodec;

class FFMpegOptionsManager {
public:
    struct VideoCodecInfo {
        std::string CodecId, CodecName, DefaultPresetId;
        bool CanUseBitrate = true, CanUseQuality = true;
        std::vector<std::pair<std::string, std::string>> Presets;
        
    };

    FFMpegOptionsManager();

    std::optional<VideoCodecInfo> getVideoCodecInfo(const std::string codecId);
    std::unique_ptr<FFmpegVideoCodec> createVideoCodec(const std::string codecId);
    std::vector<std::pair<std::string, std::string>> getVideoCodecs();

private:
    using VideoCodecFactoryFunc = std::function<std::unique_ptr<FFmpegVideoCodec>()>;
    std::map<std::string, VideoCodecFactoryFunc> videoCodecFactories_;
};
