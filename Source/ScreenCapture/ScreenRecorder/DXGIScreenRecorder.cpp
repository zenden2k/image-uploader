#include "DXGIScreenRecorder.h"

#include <numeric>

#include "atlheaders.h"
#include <ComDef.h>

#include "Core/Utils/CoreUtils.h"
#include "../3rdpart/capture.hpp"
#include "Core/3rdpart/dxerr.h"
#include "Core/ScreenCapture/ScreenshotHelper.h"

namespace {

bool StringToGUID(const std::string& str, GUID& guid)
{
    HRESULT hr = CLSIDFromString(IuCoreUtils::Utf8ToWstring(str).c_str(), &guid);
    return SUCCEEDED(hr);
}

CString GetMessageForHresult(HRESULT hr)
{
    //_com_error error(hr);
    CString cs;
    WCHAR descr[1024] = L"";
    DXGetErrorDescriptionW(hr, descr, std::size(descr));
    cs.Format(_T("\r\nError 0x%08x: %s\r\n%s"), hr, DXGetErrorStringW(hr), descr);
    return cs;
}

CComPtr<IDXGIAdapter1> getAdapterByIndex(int index) {
    CComPtr<IDXGIAdapter1> pAdapter;
    if (index < 0) {
        return pAdapter;
    }

    CComPtr<IDXGIFactory1> pFactory;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);

    if (FAILED(hr)) {
        return pAdapter;
    }

    for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        if (i == index) {
            DXGI_ADAPTER_DESC1 desc;
            hr = pAdapter->GetDesc1(&desc);
            if (SUCCEEDED(hr)) {
                LOG(ERROR) << "Adapter " << i <<  desc.Description << std::endl;
                /*wprintf(L"VendorId: %04X, DeviceId: %04X\n",
                    desc.VendorId, desc.DeviceId);
                */
            }
            return pAdapter;
        }
        

        pAdapter.Release();
    }
    return pAdapter;
}


CComPtr<IDXGIAdapter1> GetAdapterForMonitor(HMONITOR hMonitor) {
    CComPtr<IDXGIFactory1> pFactory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory);

    CComPtr<IDXGIAdapter1> pAdapter;
    for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        CComPtr<IDXGIOutput> pOutput;
        for (UINT j = 0; pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND; ++j) {
            DXGI_OUTPUT_DESC outputDesc;
            pOutput->GetDesc(&outputDesc);

            if (outputDesc.Monitor == hMonitor) {
                return pAdapter; 
            }
            pOutput.Release();
        }
        pAdapter.Release();
    }

    return nullptr; 
}

}

DXGIScreenRecorder::DXGIScreenRecorder(std::string outFile, HWND wnd, CRect rect, HMONITOR monitor, DXGIOptions options)
    : ScreenRecorder(std::move(outFile), rect), options_(std::move(options))
{
    dp_ = std::make_unique<DESKTOPCAPTUREPARAMS>();

    std::string ext = IuCoreUtils::ExtractFileExt(outFilePath_);

    adapter_ = GetAdapterForMonitor(monitor);

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
    if (wnd) {
        ScreenshotHelper::getActualWindowRect(wnd, &captureRect_);

        captureRect_.right = captureRect_.left + ((captureRect_.Width() + 1) & ~1); // Rounding an integer up to the earest even number
        captureRect_.bottom = captureRect_.top + ((captureRect_.Height() + 1) & ~1); 
    }

    //dp_->hWnd = wnd;
    dp_->rx = captureRect_;
    dp_->ad = adapter_;

    GUID containerType = MFTranscodeContainerType_MPEG4;

    if (isWmv) {
        containerType = MFTranscodeContainerType_ASF;
    }
    dp_->PrepareAttributes = [containerType](IMFAttributes* attrs) {
        attrs->SetGUID(MF_TRANSCODE_CONTAINERTYPE, containerType);
    };

    dp_->HasAudio = false;

    for (const auto& audioSource : options_.audioSources) {
        auto audioSourceInfo = optionsManager_.getAudioSourceInfo(audioSource);
        if (audioSourceInfo) {
            std::vector<int> vec(audioSourceInfo->NumChannels);
            std::iota(vec.begin(), vec.end(), 0);
            dp_->AudioFrom.push_back({ IuCoreUtils::Utf8ToWstring(audioSource), std::move(vec) });
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
            if (IuCoreUtils::FileExists(outFilePath_)) {
                IuCoreUtils::RemoveFile(outFilePath_);
            }
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

void DXGIScreenRecorder::setOffset(int x, int y) {
    dp_->rx = { x, y, x + captureRect_.Width(), y + captureRect_.Height() };
}
