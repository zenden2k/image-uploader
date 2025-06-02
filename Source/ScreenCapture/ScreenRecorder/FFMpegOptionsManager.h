#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "FFmpegOptions.h"

class FFmpegVideoCodec;
class AudioCodec;
class FFmpegSource;

class FFMpegOptionsManager {
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


    FFMpegOptionsManager();

    std::optional<VideoCodecInfo> getVideoCodecInfo(const std::string& codecId);
    std::optional<AudioCodecInfo> getAudioCodecInfo(const std::string& codecId);
    std::unique_ptr<FFmpegVideoCodec> createVideoCodec(const std::string& codecId);
    std::unique_ptr<FFmpegSource> createSource(const std::string& sourceId);
    std::unique_ptr<AudioCodec> createAudioCodec(const std::string& codecId);
    IdNameArray getVideoCodecs();
    IdNameArray getAudioCodecs();
    IdNameArray getVideoSources();
    IdNameArray getAudioSources();

    static std::string findFFmpegExecutable();

private:
    using VideoCodecFactoryFunc = std::function<std::unique_ptr<FFmpegVideoCodec>()>;
    using AudioCodecFactoryFunc = std::function<std::unique_ptr<AudioCodec>()>;
    using SourceFactoryFunc = std::function<std::unique_ptr<FFmpegSource>(const std::string& param)>;
    std::map<std::string, VideoCodecFactoryFunc> videoCodecFactories_;
    std::map<std::string, AudioCodecFactoryFunc> audioCodecFactories_;
    std::map<std::string, SourceFactoryFunc> videoSourceFactories_;
    std::map<std::string, SourceFactoryFunc> audioSourceFactories_;

};
