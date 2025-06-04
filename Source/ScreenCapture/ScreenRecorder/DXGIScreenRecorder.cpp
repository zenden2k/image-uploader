#include "DXGIScreenRecorder.h"

#include <numeric>

#include "atlheaders.h"
#include <ComDef.h>

#include "Core/Utils/CoreUtils.h"
#include "../3rdpart/capture.hpp"
#include "Core/3rdpart/dxerr.h"

namespace {

bool StringToGUID(const std::string& str, GUID& guid) {
    HRESULT hr = CLSIDFromString(IuCoreUtils::Utf8ToWstring(str).c_str(), &guid);
    return SUCCEEDED(hr);
}

CString GetMessageForHresult(HRESULT hr) {
    //_com_error error(hr);
    CString cs;
    WCHAR descr[1024] = L"";
    DXGetErrorDescriptionW(hr, descr, std::size(descr));
    cs.Format(_T("\r\nError 0x%08x: %s\r\n%s"), hr, DXGetErrorStringW(hr), descr);
    return cs;
}

}

DXGIScreenRecorder::DXGIScreenRecorder(std::string outFile, CRect rect, DXGIOptions options)
    : ScreenRecorder(std::move(outFile), rect), options_(std::move(options))
{
    dp_ = std::make_unique<DESKTOPCAPTUREPARAMS>();

    std::string ext = IuCoreUtils::ExtractFileExt(outFilePath_);

    dp_->VIDEO_ENCODING_FORMAT = MFVideoFormat_H264;
    dp_->AUDIO_ENCODING_FORMAT = MFAudioFormat_AAC;

    StringToGUID(options_.codec, dp_->VIDEO_ENCODING_FORMAT);
    StringToGUID(options_.audioCodec, dp_->AUDIO_ENCODING_FORMAT);
    bool isWmv = dp_->VIDEO_ENCODING_FORMAT == MFVideoFormat_WMV3;
    if (ext.empty()) {
        ext = isWmv ? "wmv" : "mp4";
        outFilePath_ += "." + ext;
    }
    dp_->f = IuCoreUtils::Utf8ToWstring(outFilePath_);

    dp_->rx = rect;
    dp_->Cursor = options_.showCursor;
    if (options_.useQuality) {
        dp_->vbrm = 2;
        dp_->vbrq = options_.quality;
    } else {
        dp_->BR = options_.bitrate;
    }

    dp_->ABR = options_.audioBitrate;
    dp_->fps = options_.framerate;
    dp_->HasVideo = true;


    GUID containerType = MFTranscodeContainerType_MPEG4;

    if (isWmv) {
        containerType = MFTranscodeContainerType_ASF;
    }
    dp_->PrepareAttributes = [containerType](IMFAttributes* attrs) {
        attrs->SetGUID(MF_TRANSCODE_CONTAINERTYPE, containerType);
    };

    dp_->HasAudio = false;

    if (!options_.audioSource.empty()) {
        auto audioSourceInfo = optionsManager_.getAudioSourceInfo(options_.audioSource);
        if (audioSourceInfo) {
            int numChannels = audioSourceInfo->NumChannels;
            std::vector<int> vec(numChannels);
            std::iota(vec.begin(), vec.end(), 0);
            dp_->AudioFrom.push_back({ IuCoreUtils::Utf8ToWstring(options_.audioSource), std::move(vec) });
            dp_->HasAudio = true;
        }
    }
}

DXGIScreenRecorder::~DXGIScreenRecorder() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void DXGIScreenRecorder::start() {
    if (isRunning_) {
        dp_->Pause = false;
        changeStatus(Status::Recording);
        return;
    }

    isRunning_ = true;
    thread_ = std::thread([this] {
        changeStatus(Status::Recording);
        auto [res, hr] = DesktopCapture(*dp_);
        if (res != 0) {
            LOG(ERROR) << "Return code:" << res << std::endl
                       //<< _com_error(hr).ErrorMessage() << std::endl
                       << GetMessageForHresult(hr).GetString(); 
        }
        if (cancelRequested_) {
            changeStatus(Status::Canceled);
        } else /*if (dp_->MustEnd)*/ {
            changeStatus(res == 0 ? Status::Finished : Status::Failed);
        }
        isRunning_ = false;
    });
}

void DXGIScreenRecorder::stop() {
    dp_->MustEnd = true;
}

void DXGIScreenRecorder::pause() {
    dp_->Pause = true;
    changeStatus(Status::Paused);
}

void DXGIScreenRecorder::cancel() {
    cancelRequested_ = true;
    dp_->MustEnd = true;
}
