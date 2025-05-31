#include "FFMpegOptionsManager.h"

#include <algorithm>

#include "Core/Utils/StringUtils.h"
#include "VideoCodecs/X264VideoCodec.h"
#include "VideoCodecs/NvencVideoCodec.h"
#include "VideoCodecs/VP9VideoCodec.h"


FFMpegOptionsManager::FFMpegOptionsManager() {
    videoCodecFactories_[X264VideoCodec::CODEC_ID] = [] { return std::make_unique<X264VideoCodec>(); };
    videoCodecFactories_[VP9VideoCodec::CODEC_ID] = [] { return std::make_unique<VP9VideoCodec>(); };
    videoCodecFactories_[NvencVideoCodec::H264_CODEC_ID] = [] { return NvencVideoCodec::createH264(); };
    videoCodecFactories_[NvencVideoCodec::HEVC_CODEC_ID] = [] { return NvencVideoCodec::createHevc(); };
}

std::optional<FFMpegOptionsManager::VideoCodecInfo> FFMpegOptionsManager::getVideoCodecInfo(const std::string codecId) {
    auto codec = createVideoCodec(codecId);
    if (!codec) {
        return {};
    }
    VideoCodecInfo info;
    info.CodecId = codecId;
    info.CodecName = codec->name();
    info.Presets = codec->presets();
    info.DefaultPresetId = codec->defaultPreset();
    info.CanUseBitrate = codec->canUseBitrate();
    info.CanUseQuality = codec->canUseQuality();
    return info;
}

std::unique_ptr<FFmpegVideoCodec> FFMpegOptionsManager::createVideoCodec(const std::string codecId) {
    auto it = videoCodecFactories_.find(codecId);
    if (it != videoCodecFactories_.end()) {
        return it->second();
    }
    return {};
}

std::vector<std::pair<std::string, std::string>> FFMpegOptionsManager::getVideoCodecs() {
    std::vector<std::pair<std::string, std::string>> result;

    for (const auto& [codecId, v] : videoCodecFactories_) {
        auto codec = createVideoCodec(codecId);
        if (!codec) {
            continue;
        }
        result.push_back(std::make_pair(codecId, codec->name()));
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return IuStringUtils::stricmp(a.second.c_str(), b.second.c_str()) < 0;
    });

    return result;
}
