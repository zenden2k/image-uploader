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

#include <boost/asio.hpp>
#include <boost/format.hpp>

#include "ArgsBuilder/FFmpegArgsBuilder.h"
#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"
#include "Sources/DDAGrabSource.h"
#include "Sources/GDIGrabSource.h"
#include "VideoCodecs/FFmpegVideoCodec.h"
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

constexpr auto FFMPEG_RETURN_CODE_NO_OUTPUT = -22;

FFmpegScreenRecorder::FFmpegScreenRecorder(std::string ffmpegPath, std::string outFile, CRect rect, FFmpegOptions options)
     : ScreenRecorder(outFile, rect)
    , ffmpegPath_(std::move(ffmpegPath))
    , options_(std::move(options))
{
}

FFmpegScreenRecorder::~FFmpegScreenRecorder() {
    if (future_.valid()) {
        try {
            future_.wait();
        } catch (const std::exception& ex) {
            LOG(WARNING) << ex.what();
        }
    }
}

void FFmpegScreenRecorder::start() {
    if (isRunning()) {
        return;
    }

    auto codec = optionsManager_.createVideoCodec(options_.codec);

    if (!codec) {
        LOG(ERROR) << "Failed to create codec " << options_.codec;
        return;
    }
    std::string ext = IuCoreUtils::ExtractFileExt(outFilePath_);

    if (ext.empty()) {
        ext = codec->extension();
        outFilePath_ += "." + ext;
    }

    if (fileNoExt_.empty()) { 
        std::string fileDir = IuCoreUtils::ExtractFilePath(outFilePath_);
        std::string fileNoExt = IuCoreUtils::ExtractFileNameNoExt(outFilePath_);
        std::filesystem::path outDirPath(fileDir);
        std::filesystem::path fileNoExtPath = outDirPath / fileNoExt;
        fileNoExt_ = fileNoExtPath.make_preferred().string();
    }
     
    currentOutFilePath_ = str(boost::format("%s_part%02d.%s") % fileNoExt_ % parts_.size() % ext);

    /*std::string filterComplex = str(boost::format("ddagrab=video_size=%dx%d:offset_x=%d:offset_y=%d") % captureRect_.Width() % captureRect_.Height()
        % captureRect_.left % captureRect_.top);*/


    FFMpegArgsBuilder argsBuilder;

    options_.width = captureRect_.Width() & ~1;
    options_.height = captureRect_.Height() & ~1;
    options_.offsetX = captureRect_.left;
    options_.offsetY = captureRect_.top;

    argsBuilder.globalArgs()
        .addArg("-hide_banner")
        .addArg("thread_queue_size", 1024)
        .addArg("rtbufsize", "256M");

    auto& input = argsBuilder.addInputFile(options_.source == GDIGrabSource::SOURCE_ID ? "desktop" : "");
    auto& output = argsBuilder.addOutputFile(currentOutFilePath_);
    //auto outputArgs = argsBuilder.

    auto videoSource = optionsManager_.createSource(options_.source);

    if (!videoSource) {
        LOG(ERROR) << "Failed to create video source " << options_.source;
        return;
    }

    videoSource->apply(options_, input, argsBuilder.globalArgs());


    codec->apply(options_, output);

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

    future_ = launchFFmpeg(args, [this](int res) {
        if (res == 0 && IuCoreUtils::FileExists(currentOutFilePath_)) {
            parts_.push_back(currentOutFilePath_);
        }
        changeStatus((res == 0 || res == FFMPEG_RETURN_CODE_NO_OUTPUT) ? Status::Paused : Status::Failed);
    });
}

std::future<int> FFmpegScreenRecorder::launchFFmpeg(const std::vector<std::string>& args, std::function<void(int)> onFinish) {
    namespace bp = boost::process::v2;
    namespace asio = boost::asio;

    std::string command = ffmpegPath_;
    isRunning_ = true;
    //auto self = shared_from_this();
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
                bool warning = false;
                int actualResultCode = result;

                if (result == FFMPEG_RETURN_CODE_NO_OUTPUT) {
                    //result = 0;
                    warning = true;
                }
                (warning ? LOG(WARNING) : LOG(ERROR)) << "Exit code:" << actualResultCode << std::endl
                                  << errOutput << std::endl
                    << result << std::endl << output;
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
    //return isRunning_;
    if (!future_.valid()) {
        return false;
    }
    using namespace std::chrono_literals;
    auto status = future_.wait_for(0ms);

    // Print status.
    return status != std::future_status::ready;
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
    auto self = shared_from_this();
    auto cb = [self](int) {
        if (self->parts_.empty()) {
            self->changeStatus(Status::Canceled);
            return;
        }

        if (self->parts_.size() == 1) {
            try {
                std::filesystem::rename(std::filesystem::u8path(self->currentOutFilePath_), std::filesystem::u8path(self->outFilePath_));
            } catch (const std::exception& ex) {
                LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what());
            }
            self->changeStatus(Status::Finished);
        } else {
            const char* tempFileName = std::tmpnam(nullptr);

            if (!tempFileName) {
                LOG(ERROR) << "Failed to generate temp file path.";
                return;
            }

            const std::string tempFile = tempFileName;
            std::ofstream f(/*std::filesystem::u8path*/ (tempFile), std::ios::out);

            if (!f) {
                LOG(ERROR) << "Failed to create temp file " << tempFile;
                //throw IOException("Failed to create output file: " + std::string(strerror(errno)), filenameUtf8);
            } else {
                for (const auto& item : self->parts_) {
                    f << "file '" << item << "'" << std::endl;
                }
            }
            f.close();

            self->changeStatus(Status::RunningConcatenation);
           
            self->future_ = self->launchFFmpeg({ "-f", "concat", "-safe", "0", "-i", tempFile, "-c", "copy", self->outFilePath_ }, [self](int res) {
                self->changeStatus(res == 0 ? Status::Finished : Status::Failed);
                if (res == 0) {
                    self->cleanupAfter();
                }
            });
        }
    };

    sendStopSignal();
    //async_wait_future(timerCtx_, std::move(future_), cb);

    std::thread([self, cb] {
        int res = 0;
        try {
            if (self->future_.valid()) {
                res = self->future_.get();
            }
        } catch (const std::exception& ex) {
            LOG(ERROR) << "Future error: " << ex.what();
        }
        try {
            cb(res);
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }
    }).detach();
}

void FFmpegScreenRecorder::cancel() {
    if (isRunning()) {
        sendStopSignal();
    }

    auto self = shared_from_this();

    auto cb = [self](int) {
        self->cleanupAfter();
        try {
            std::filesystem::remove(std::filesystem::u8path(self->outFilePath_));
        } catch (const std::exception&) {
            //LOG(WARNING) << IuCoreUtils::Utf8ToWstring(ex.what());
        }
        self->changeStatus(Status::Canceled);
    };
    std::thread([self, cb] {
        int res = 0;
        try {
            if (self->future_.valid()) {
                res = self->future_.get();
            }
        } catch (const std::exception& ex) {
            LOG(ERROR) << "Future error: " << ex.what();
        }
        try {
            cb(res);
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }
    }).detach();
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
