#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class FFmpegVideoCodec;
class FFmpegSource;

class FFMpegOptionsManager {
public:
    using IdNamePair = std::pair<std::string, std::string>;
    using IdNameArray = std::vector<std::pair<std::string, std::string>>;
    struct VideoCodecInfo {
        std::string CodecId, CodecName, DefaultPresetId;
        bool CanUseBitrate = true, CanUseQuality = true;
        IdNameArray Presets;   
    };

    FFMpegOptionsManager();

    std::optional<VideoCodecInfo> getVideoCodecInfo(const std::string& codecId);
    std::unique_ptr<FFmpegVideoCodec> createVideoCodec(const std::string& codecId);
    std::unique_ptr<FFmpegSource> createSource(const std::string& sourceId);
    IdNameArray getVideoCodecs();
    IdNameArray getVideoSources();
    IdNameArray getAudioSources();

    static std::string findFFmpegExecutable();

private:
    using VideoCodecFactoryFunc = std::function<std::unique_ptr<FFmpegVideoCodec>()>;
    using SourceFactoryFunc = std::function<std::unique_ptr<FFmpegSource>(const std::string& param)>;
    std::map<std::string, VideoCodecFactoryFunc> videoCodecFactories_;
    std::map<std::string, SourceFactoryFunc> videoSourceFactories_;
    std::map<std::string, SourceFactoryFunc> audioSourceFactories_;

};
