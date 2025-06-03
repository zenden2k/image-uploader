#pragma once

#include <memory>
#include <thread>

#include "ScreenRecorder.h"
#include "DXGIOptions.h"
#include "DXGIOptionsManager.h"

class TaskDispatcher;
struct DESKTOPCAPTUREPARAMS;

class DXGIScreenRecorder : public ScreenRecorder,
                             public std::enable_shared_from_this<DXGIScreenRecorder>
{
public:
    DXGIScreenRecorder(std::string outFile, CRect rect, DXGIOptions options);
    ~DXGIScreenRecorder() override;
    void start() override;
    void stop() override;
    void pause() override;
    void cancel() override;
private:
    std::thread thread_;

    std::string fileNoExt_;
    std::string currentOutFilePath_;
    std::shared_future<int> future_;
    //FFmpegOptions options_;
    DXGIOptionsManager optionsManager_;
    std::unique_ptr<DESKTOPCAPTUREPARAMS> dp_;
    DXGIOptions options_;
    bool cancelRequested_ = false;
};
