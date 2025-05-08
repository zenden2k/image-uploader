#pragma once

#include "ScreenRecorder.h"

class FFmpegScreenRecorder: public ScreenRecorder
{
public:
    FFmpegScreenRecorder(std::string ffmpegPath, std::string outDirectory, CRect rect);
    ~FFmpegScreenRecorder() override;
    void start();
    void stop();
    void pause();
    void cancel();
    bool isRunning() const;

    template<typename F>
    boost::signals2::connection addStatusChangeCallback(F&& f) {
        return onStatusChange_.connect(std::forward<F>(f));
    }

private:
    std::string ffmpegPath_;
    //std::thread thread_;
    bool isRunning_ = false;
    std::unique_ptr<boost::process::opstream> inStream_;
    std::unique_ptr<boost::process::child> child_;
    std::string fileNoExt_;
    std::string currentOutFilePath_;
    std::vector<std::string> parts_;
    std::future<int> future_;
    boost::signals2::signal<void(Status)> onStatusChange_;
    void sendStopSignal();
    std::future<int> launchFFmpeg(const std::vector<std::string>& args, std::function<void(int)> onFinish);
    void cleanupAfter();

};
