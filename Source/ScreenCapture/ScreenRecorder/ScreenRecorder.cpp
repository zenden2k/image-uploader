#include "ScreenRecorder.h"


#include <filesystem>
#include <chrono>
#include <fstream>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/process/windows.hpp>
#include <boost/format.hpp>


#include "ArgsBuilder/FFmpegArgsBuilder.h"
#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"
#include "Sources/DDAGrabSource.h"
#include "Sources/GDIGrabSource.h"
#include "VideoCodecs/NvencVideoCodec.h"
#include "VideoCodecs/X264VideoCodec.h"

ScreenRecorder::ScreenRecorder(std::string ffmpegPath, std::string outDirectory, CRect rect):
                                    ffmpegPath_(std::move(ffmpegPath)),
                                    outDirectory_(std::move(outDirectory)),
                                    captureRect_(rect){
    
}

ScreenRecorder::~ScreenRecorder() {
    /*if (thread_.joinable()) {
        thread_.join();
    }*/
}

void ScreenRecorder::start() {
    if (isRunning()) {
        return;
    }
    if (fileNoExt_.empty()) {
        std::filesystem::path outDirPath(outDirectory_);
        std::filesystem::path fileNoExtPath = outDirPath / ("screenrecorder_" + std::to_string(time(nullptr)));
        fileNoExt_ = fileNoExtPath.make_preferred().string();
        fileFull_ = fileNoExt_ + ".mp4";
    }
     
    outFilePath_ = str(boost::format("%s_part%02d.mp4") % fileNoExt_ % parts_.size());

    //LOG(ERROR) << outFilePath_;
    //return;
    /*std::string filterComplex = str(boost::format("ddagrab=video_size=%dx%d:offset_x=%d:offset_y=%d") % captureRect_.Width() % captureRect_.Height()
        % captureRect_.left % captureRect_.top);*/

    FFMpegArgsBuilder argsBuilder;

    FFmpegSettings settings;

    settings.source = "gdigrab";
    settings.codec = "x264";//
    settings.width = captureRect_.Width() & ~1;
    settings.height = captureRect_.Height() & ~1;
    settings.offsetX = captureRect_.left;
    settings.offsetY = captureRect_.top;

    argsBuilder.globalArgs()
        .addArg("-hide_banner")
        .addArg("thread_queue_size", 1024)
        .addArg("rtbufsize", "256M");

    auto& input = argsBuilder.addInputFile(settings.source == "gdigrab" ? "desktop" : "");
    auto& output = argsBuilder.addOutputFile(outFilePath_);
    //auto outputArgs = argsBuilder.

    GDIGrabSource gdigrab;
    gdigrab.apply(settings, input, argsBuilder.globalArgs());

    /*auto ddagrabSource = std::make_unique<DDAGrabSource>();
    ddagrabSource->apply(settings, input, argsBuilder.globalArgs());*/
   
    /*auto nvenc = NvencVideoCodec::createH264();
    nvenc->apply(settings, output);**/
        
    auto x264 = std::make_unique<X264VideoCodec>();
    x264->apply(settings, output);

    std::vector<std::string> args = argsBuilder.getArgs();

    std::string s;
    for (const auto& piece : args) {
        s += piece;
        s += " ";
    }
    LOG(ERROR) << ffmpegPath_ << std::endl << s;

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


    changeStatus(Status::Recording);
    future_ = launchFFmpeg(args, [this](int res) {
        if (res == 0) {
            parts_.push_back(outFilePath_);
        }
        changeStatus(Status::Paused);
    });
}

std::future<int> ScreenRecorder::launchFFmpeg(const std::vector<std::string>& args, std::function<void(int)> onFinish) {
    namespace bp = boost::process;
    std::string command = ffmpegPath_;

    return std::async(std::launch::async, [&, command, args, onFinish] { 
        try {
            boost::asio::io_service ios;
            std::vector<char> buf;

            std::future<std::string> data;
            inStream_ = std::make_unique<bp::opstream>();

            child_ = std::make_unique<bp::child>(command, args, bp::std_in < *inStream_.get(),
                bp::std_out > bp::null, //so it can be written without anything
                bp::std_err > data,
                ios,
                ::boost::process::windows::create_no_window);

            ios.run();

            child_->wait(); // reap PID
            int result = child_->exit_code();
            if (result != 0) {
                LOG(ERROR) << "Exit code:"<<  result << std::endl << data.get();
            }

            if (onFinish) {
                onFinish(result);
            }
            return result;
            //LOG(ERROR) << result << std::endl << std::string(buf.data(), buf.size());
            //LOG(ERROR) << result << std::endl << std::string(buf.data(), buf.size());
        }
        catch (const bp::process_error& ex) {
            LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what()) << std::endl;
        }
        catch (const std::exception& ex) {
            LOG(ERROR) << ex.what() << std::endl;
        }

        if (onFinish) {
            onFinish(1);
        }

        return 1;
    });

}

bool ScreenRecorder::isRunning() const {
    if (!future_.valid()) {
        return false;
    }
    using namespace std::chrono_literals;
    auto status = future_.wait_for(0ms);

    // Print status.
    return status != std::future_status::ready;
}

void ScreenRecorder::sendStopSignal() {
    if (!isRunning()) {
        return;
    }
    *inStream_ << "q" << std::endl;
}

void ScreenRecorder::stop() {
    sendStopSignal();


    future_.wait();

    if (parts_.empty()) {
        return;
    }

    if (parts_.size() == 1) {
        try {
            std::filesystem::rename(std::filesystem::u8path(outFilePath_), std::filesystem::u8path(fileFull_));
        } catch (const std::exception& ex) {
            LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what());
        }
        changeStatus(Status::Finished);
    } else {
        std::string tempFile = std::tmpnam(nullptr);
        std::ofstream f(/*std::filesystem::u8path*/(tempFile), std::ios::out);

        if (!f) {
            LOG(ERROR) << "Failed to create temp file " << tempFile;
            //throw IOException("Failed to create output file: " + std::string(strerror(errno)), filenameUtf8);
        } else {
            for(const auto& item: parts_) {
                f << "file '" << item << "'" << std::endl;
            }
        }
        f.close();

        changeStatus(Status::RunningConcatenation);

        auto future = launchFFmpeg({ "-f","concat", "-safe","0",  "-i", tempFile,"-c", "copy", fileFull_ }, [this](int res) {
            changeStatus(Status::Finished);
            if (res == 0) {
                cleanupAfter();
            }
        });

        /*if (future.get() != 0) {
            LOG(ERROR) << "Failed to concatenate " << parts_.size() << " video files";
        }*/
    }
}

void ScreenRecorder::cancel() {
    sendStopSignal();

    future_.wait();
    cleanupAfter();
    try {
        std::filesystem::remove(std::filesystem::u8path(outFilePath_));
    }
    catch (const std::exception& ) {
        //LOG(WARNING) << IuCoreUtils::Utf8ToWstring(ex.what());
    }
    changeStatus(Status::Canceled);
}

void ScreenRecorder::pause() {
    sendStopSignal();
}

void ScreenRecorder::changeStatus(Status status) {
    status_ = status;
    onStatusChange_(status);
}

void ScreenRecorder::setOffset(int x, int y) {
    captureRect_ = CRect(x, y, x + captureRect_.Width(), y + captureRect_.Height());
}

void ScreenRecorder::cleanupAfter() {
    for (const auto& item : parts_) {
        try {
            std::filesystem::remove(std::filesystem::u8path(item));
        }
        catch (const std::exception& ex) {
            LOG(WARNING) << IuCoreUtils::Utf8ToWstring(ex.what());
        }
    }
}

ScreenRecorder::Status ScreenRecorder::status() const {
    return status_;
}

std::string ScreenRecorder::outFileName() const {
    return status_ == Status::Finished ? fileFull_ : std::string();
}