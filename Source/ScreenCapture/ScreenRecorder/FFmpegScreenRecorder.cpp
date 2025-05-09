#include "FFmpegScreenRecorder.h"

#include <filesystem>
#include <chrono>
#include <fstream>

#include <boost/process/v2/process.hpp>
#include <boost/process/v2/stdio.hpp>
#include <boost/process/v2/execute.hpp>
#ifdef _WIN32
#include <boost/process/v2/windows/default_launcher.hpp>
#endif
//#include <boost/process.hpp>
#include <boost/asio.hpp>
//#include <boost/process/windows.hpp>
#include <boost/format.hpp>

#include "ArgsBuilder/FFmpegArgsBuilder.h"
#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"
#include "Sources/DDAGrabSource.h"
#include "Sources/GDIGrabSource.h"
#include "VideoCodecs/NvencVideoCodec.h"
#include "VideoCodecs/X264VideoCodec.h"
#include "Core/TaskDispatcher.h"
/*
void async_wait_future(
    boost::asio::io_context& io,
    std::future<int> fut,
    std::function<void(int)> callback)
{
    auto timer = std::make_shared<boost::asio::steady_timer>(io);
    timer->expires_after(std::chrono::milliseconds(10));

    timer->async_wait([timer, &io, fut = std::move(fut), callback](auto ec) mutable {
        if (fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            try {
                callback(fut.get());
            } catch (const std::exception& ex) {
                LOG(ERROR) << ex.what();
            }

        } else {
            async_wait_future(io, std::move(fut), callback);
        }
    });
}*/

FFmpegScreenRecorder::FFmpegScreenRecorder(std::string ffmpegPath, std::string outFile, CRect rect)
     : ScreenRecorder(outFile, rect)
    , ffmpegPath_(std::move(ffmpegPath))
   // , taskDispatcher_(taskDispatcher)
{
    
}

FFmpegScreenRecorder::~FFmpegScreenRecorder() {
    timerCtx_.stop();
}

void FFmpegScreenRecorder::start() {
    if (isRunning()) {
        return;
    }
    std::string ext = IuCoreUtils::ExtractFileExt(outFilePath_);
    if (fileNoExt_.empty()) {
        
        std::string fileDir = IuCoreUtils::ExtractFilePath(outFilePath_);
        std::string fileNoExt = IuCoreUtils::ExtractFileNameNoExt(outFilePath_);
        std::filesystem::path outDirPath(fileDir);
        std::filesystem::path fileNoExtPath = outDirPath / (fileNoExt + std::to_string(time(nullptr)));
        fileNoExt_ = fileNoExtPath.make_preferred().string();
    }
     
    currentOutFilePath_ = str(boost::format("%s_part%02d.%s") % fileNoExt_ % parts_.size() % ext);

    //LOG(ERROR) << outFilePath_;
    //return;
    /*std::string filterComplex = str(boost::format("ddagrab=video_size=%dx%d:offset_x=%d:offset_y=%d") % captureRect_.Width() % captureRect_.Height()
        % captureRect_.left % captureRect_.top);*/

    FFMpegArgsBuilder argsBuilder;

    FFmpegSettings settings;

    settings.source = "gdigrab";
    //settings.codec = "h264_nvenc"; 
    settings.codec = "x264"; 

    if (settings.codec == "h264_nvenc") {
        settings.source = "ddagrab";
    }

    settings.width = captureRect_.Width() & ~1;
    settings.height = captureRect_.Height() & ~1;
    settings.offsetX = captureRect_.left;
    settings.offsetY = captureRect_.top;

    argsBuilder.globalArgs()
        .addArg("-hide_banner")
        .addArg("thread_queue_size", 1024)
        .addArg("rtbufsize", "256M");

    auto& input = argsBuilder.addInputFile(settings.source == "gdigrab" ? "desktop" : "");
    auto& output = argsBuilder.addOutputFile(currentOutFilePath_);
    //auto outputArgs = argsBuilder.

    if (settings.source == "ddagrab") {
        auto ddagrabSource = std::make_unique<DDAGrabSource>();
        ddagrabSource->apply(settings, input, argsBuilder.globalArgs());
    } else {
        GDIGrabSource gdigrab;
        gdigrab.apply(settings, input, argsBuilder.globalArgs());
    }

    if (settings.codec == "h264_nvenc" ) {
        auto nvenc = NvencVideoCodec::createH264();
        nvenc->apply(settings, output);
    } else if (settings.codec == "x264") {
        auto x264 = std::make_unique<X264VideoCodec>();
        x264->apply(settings, output);
    }
    /***/
        
    std::vector<std::string> args = argsBuilder.getArgs();

    std::string s;
    for (const auto& piece : args) {
        s += piece;
        s += " ";
    }
    LOG(INFO) << ffmpegPath_ << std::endl << s;

   /*-{
            "-hide_banner",
            "-init_hw_device", "d3d11va",
            //"-framerate","60",
            /*"-offset_x",std::to_string(captureRect_.left),
            "-offset_y",std::to_string(captureRect_.top),
            "-video_size",std::to_string(captureRect_.Width())+"x" + std::to_string(captureRect_.Height()),*
            "-filter_complex", filterComplex,
            /*,video_size = "+ std::to_string(captureRect_.Width()) + "x" + std::to_string(captureRect_.Height())
            + ",offset_x=" + std::to_string(captureRect_.left),
            + ",offset_y=" + std::to_string(captureRect_.top),**
            "-c:v", "h264_nvenc",
            "-cq:v", "20", outFilePath_
    };*/

    if (isRunning()) {
        return;
    }
    changeStatus(Status::Recording);
    if (isRunning()) {
        return;
    }
    future_ = launchFFmpeg(args, [this](int res) {
        if (res == 0) {
            parts_.push_back(currentOutFilePath_);
        }
        changeStatus(res == 0 ? Status::Paused : Status::Failed);
    });
}

std::future<int> FFmpegScreenRecorder::launchFFmpeg(const std::vector<std::string>& args, std::function<void(int)> onFinish) {
    namespace bp = boost::process::v2;
    namespace asio = boost::asio;

    std::string command = ffmpegPath_;
    isRunning_ = true;
    return std::async(std::launch::async, [this, command, args, onFinish] {
        std::string output;
        std::string errOutput;
        boost::asio::io_context ctx_;
        try {
            asio::readable_pipe outPipe(ctx_);
            asio::readable_pipe errPipe(ctx_);

            inStream_ = std::make_shared<asio::writable_pipe>(ctx_);
#ifdef _WIN32
            bp::windows::default_launcher dl;
            dl.creation_flags |= CREATE_NO_WINDOW;
            bp::process child { dl(ctx_, command, args, bp::process_stdio { *inStream_, outPipe, errPipe }) };
#else
            bp::process child(ctx_, command, args, bp::process_stdio { *inStream_, outPipe, errPipe });
#endif
   
            auto db = asio::dynamic_buffer(output);
            asio::async_read(
                outPipe,
                db,
                [&](const boost::system::error_code& ec, std::size_t bytes_read) {
                    if (ec == asio::error::eof || !ec) {
                        //LOG(INFO) << "STDOUT: " << output << std::endl;
                    } else if (ec != asio::error::broken_pipe) {
                        LOG(ERROR) << "Error reading stdout: " << IuCoreUtils::SystemLocaleToUtf8(ec.message()) << std::endl;
                    }
                });
            auto db2 = asio::dynamic_buffer(errOutput);
            asio::async_read(
                errPipe,
                db2,
                [&](const boost::system::error_code& ec, std::size_t bytes_read) {
                    if (ec == asio::error::eof || !ec) {
                        //LOG(INFO) << "STDERR: " << output << std::endl;
                    } else if (ec != asio::error::broken_pipe) {
                        LOG(ERROR) << "Error reading stderr: " << IuCoreUtils::SystemLocaleToUtf8(ec.message()) << std::endl;
                    }
                });

            ctx_.run();

            child.wait(); // reap PID
            int result = child.exit_code();
            if (result != 0) {
                LOG(ERROR) << "Exit code:" << result << std::endl
                           << output;
            }
            inStream_.reset();
            isRunning_ = false;

            if (onFinish) {
                onFinish(result);
            }

            return result;
        } catch (const std::exception& ex) {
            LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what()) << std::endl;
        }
        inStream_.reset();
        isRunning_ = false;
        if (onFinish) {
            onFinish(1);
        }
        
        return 1;
    });
}

bool FFmpegScreenRecorder::isRunning() const {
    return isRunning_;
    /* if (!future_.valid()) {
        return false;
    }
    using namespace std::chrono_literals;
    auto status = future_.wait_for(0ms);

    // Print status.
    return status != std::future_status::ready;*/
}

void FFmpegScreenRecorder::sendStopSignal() {
    if (!isRunning()) {
        return;
    }


    namespace asio = boost::asio;

    asio::async_write(
        *inStream_,
        asio::buffer(stopData_),
        [&](boost::system::error_code ec, std::size_t n) {
            if (!ec) {
                inStream_->close();
            } else {
                LOG(WARNING) << "Pipe write error: " << IuCoreUtils::SystemLocaleToUtf8(ec.message()) << "\n";
            }
        }
    );
}

void FFmpegScreenRecorder::stop() {  
    auto cb = [&](int) {
        if (parts_.empty()) {
            changeStatus(Status::Canceled);
            return;
        }

        if (parts_.size() == 1) {
            try {
                std::filesystem::rename(std::filesystem::u8path(currentOutFilePath_), std::filesystem::u8path(outFilePath_));
            } catch (const std::exception& ex) {
                LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what());
            }
            changeStatus(Status::Finished);
        } else {
            std::string tempFile = std::tmpnam(nullptr);
            std::ofstream f(/*std::filesystem::u8path*/ (tempFile), std::ios::out);

            if (!f) {
                LOG(ERROR) << "Failed to create temp file " << tempFile;
                //throw IOException("Failed to create output file: " + std::string(strerror(errno)), filenameUtf8);
            } else {
                for (const auto& item : parts_) {
                    f << "file '" << item << "'" << std::endl;
                }
            }
            f.close();

            changeStatus(Status::RunningConcatenation);
           
            future_ = launchFFmpeg({ "-f", "concat", "-safe", "0", "-i", tempFile, "-c", "copy", outFilePath_ }, [this](int res) {
                changeStatus(Status::Finished);
                if (res == 0) {
                    cleanupAfter();
                }
            });

            /*if (future.get() != 0) {
            LOG(ERROR) << "Failed to concatenate " << parts_.size() << " video files";
        }*/
        }
    };

    sendStopSignal();
    //async_wait_future(timerCtx_, std::move(future_), cb);

    std::thread([this, cb] {
        //future.wait();
        try {
            if (future_.valid()) {
                cb(future_.get());
            } else {
                cb(0);
            }
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }

        //timerCtx_.run();
    }).detach();
}

void FFmpegScreenRecorder::cancel() {
    if (isRunning()) {
        sendStopSignal();
    }

    auto cb = [&](int) {
        cleanupAfter();
        try {
            std::filesystem::remove(std::filesystem::u8path(outFilePath_));
        } catch (const std::exception&) {
            //LOG(WARNING) << IuCoreUtils::Utf8ToWstring(ex.what());
        }
        changeStatus(Status::Canceled);
    };
    std::thread([this, cb] {
        //future.wait();
        try {
            if (future_.valid()) {
                cb(future_.get());
            } else {
                cb(0);
            }
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }

        //timerCtx_.run();
    }).detach();
        
    /*});

    std::thread([&] {
        timerCtx_.run();
    }).detach();*/
}

void FFmpegScreenRecorder::pause() {
    if (status_ == Status::Recording && isRunning()) {
        sendStopSignal();
    }
}

void FFmpegScreenRecorder::cleanupAfter() {
    for (const auto& item : parts_) {
        try {
            std::filesystem::remove(std::filesystem::u8path(item));
        }
        catch (const std::exception& ex) {
            LOG(WARNING) << IuCoreUtils::Utf8ToWstring(ex.what());
        }
    }
}
