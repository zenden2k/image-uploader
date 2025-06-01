#include "FFMpegOptionsManager.h"

#include <algorithm>

#include <boost/process/v2/environment.hpp>

#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/StringUtils.h"
#include "VideoCodecs/X264VideoCodec.h"
#include "VideoCodecs/NvencVideoCodec.h"
#include "VideoCodecs/VP9VideoCodec.h"
#include "Sources/DDAGrabSource.h"
#include "Sources/GDIGrabSource.h"

#ifdef _WIN32
#include "Sources/DirectShowSource.h"
#include "Core/CommonDefs.h"
#include "atlheaders.h"
#include <dshow.h>

FFMpegOptionsManager::IdNameArray GetDirectshowVideoInputDevices()
{
    FFMpegOptionsManager::IdNameArray result;
    CComPtr<ICreateDevEnum> pDevEnum;
    HRESULT hr = pDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr)) {
        LOG(ERROR) << "Failed to create device enumerator!" << std::endl;
        return result;
    }

    CComPtr<IEnumMoniker> pEnum;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_FALSE) {
        return result;
    } else if (FAILED(hr)) {
        LOG(ERROR) << "Failed to enumerate video devices!" << std::endl;
        return result;
    }

    CComPtr<IMoniker> pMoniker;
    while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
        CComPtr<IPropertyBag> pPropBag;
        hr = pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&pPropBag);
        if (SUCCEEDED(hr)) {
            std::string id, name;
            CComVariant varName;
            hr = pPropBag->Read(L"FriendlyName", &varName, nullptr);
            if (SUCCEEDED(hr) && varName.vt == VT_BSTR) {
                name = W2U(varName.bstrVal);
            }

            CComVariant varDevicePath;
            hr = pPropBag->Read(L"DevicePath", &varDevicePath, nullptr);
            if (SUCCEEDED(hr) && varDevicePath.vt == VT_BSTR) {
                id = W2U(varDevicePath.bstrVal);
            }

            result.push_back({ "[directshow]" + id, name.empty() ? id : name });
        }
        pMoniker.Release(); 
    }
    return result;
}

#endif

FFMpegOptionsManager::FFMpegOptionsManager() {
    videoCodecFactories_[X264VideoCodec::CODEC_ID] = [] { return std::make_unique<X264VideoCodec>(); };
    videoCodecFactories_[VP9VideoCodec::CODEC_ID] = [] { return std::make_unique<VP9VideoCodec>(); };
    videoCodecFactories_[NvencVideoCodec::H264_CODEC_ID] = [] { return NvencVideoCodec::createH264(); };
    videoCodecFactories_[NvencVideoCodec::HEVC_CODEC_ID] = [] { return NvencVideoCodec::createHevc(); };

    videoSourceFactories_[GDIGrabSource::SOURCE_ID] = [](const std::string& param) { return std::make_unique<GDIGrabSource>(); };
    videoSourceFactories_[DDAGrabSource::SOURCE_ID] = [](const std::string & param) { return std::make_unique<DDAGrabSource>(); };
    videoSourceFactories_[DirectShowSource::SOURCE_ID] = [](const std::string& deviceId) { return std::make_unique<DirectShowSource>(deviceId); };
}

std::optional<FFMpegOptionsManager::VideoCodecInfo> FFMpegOptionsManager::getVideoCodecInfo(const std::string& codecId) {
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

std::unique_ptr<FFmpegVideoCodec> FFMpegOptionsManager::createVideoCodec(const std::string& codecId) {
    auto it = videoCodecFactories_.find(codecId);
    if (it != videoCodecFactories_.end()) {
        return it->second();
    }
    return {};
}

std::unique_ptr<FFmpegSource> FFMpegOptionsManager::createSource(const std::string& id) {
    const auto [sourceId, param] = FFmpegSource::parseSourceId(id);
   
    auto it = videoSourceFactories_.find(sourceId);
    if (it != videoSourceFactories_.end()) {
        return it->second(param);
    }
    return {};
}

FFMpegOptionsManager::IdNameArray FFMpegOptionsManager::getVideoCodecs() {
    IdNameArray result;

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

FFMpegOptionsManager::IdNameArray FFMpegOptionsManager::getVideoSources() {
    IdNameArray result;

    auto compareFunc = [](const IdNamePair& a, const IdNamePair& b) {
        return IuStringUtils::stricmp(a.second.c_str(), b.second.c_str()) < 0;
    };

    for (const auto& [sourceId, v] : videoSourceFactories_) {
        auto source = createSource(sourceId);
        if (!source || source->hidden()) {
            continue;
        }
        result.push_back(std::make_pair(sourceId, source->name()));
    }
    std::sort(result.begin(), result.end(), compareFunc);

#ifdef _WIN32
    IdNameArray inputDevices = GetDirectshowVideoInputDevices();

    std::sort(inputDevices.begin(), inputDevices.end(), compareFunc);

    result.insert(result.end(), inputDevices.begin(), inputDevices.end());
#endif

    return result;
}

std::string FFMpegOptionsManager::findFFmpegExecutable() {
    return boost::process::v2::environment::find_executable("ffmpeg").string();
}
