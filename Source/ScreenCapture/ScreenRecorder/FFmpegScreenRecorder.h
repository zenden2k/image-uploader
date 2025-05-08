#pragma once

#include <boost/asio.hpp>
#include <boost/process/v2/process.hpp>

#include "ScreenRecorder.h"

class FFmpegScreenRecorder: public ScreenRecorder
{
public:
    FFmpegScreenRecorder(std::string ffmpegPath, std::string outDirectory, CRect rect);
    ~FFmpegScreenRecorder() override;
    void start() override;
    void stop() override;
    void pause() override;
    void cancel() override;
    bool isRunning() const override;
private:
    std::string ffmpegPath_;
    //std::thread thread_;
    bool isRunning_ = false;
    std::unique_ptr<boost::asio::writable_pipe> inStream_;
    std::string fileNoExt_;
    std::string currentOutFilePath_;
    std::vector<std::string> parts_;
    std::future<int> future_;
    boost::asio::io_context ctx_;
    boost::asio::io_context timerCtx_;
    void sendStopSignal();
    std::future<int> launchFFmpeg(const std::vector<std::string>& args, std::function<void(int)> onFinish);
    void cleanupAfter();

};
