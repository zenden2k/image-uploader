#include "ScreenRecorder.h"


#include <filesystem>
#include <chrono>
#include <fstream>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/process/windows.hpp>
#include <boost/format.hpp>

#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"

ScreenRecorder::ScreenRecorder(CString ffmpegPath, CRect rect): ffmpegPath_(ffmpegPath), captureRect_(rect){
    
}

ScreenRecorder::~ScreenRecorder() {
    /*if (thread_.joinable()) {
        thread_.join();
    }*/
}

void ScreenRecorder::start() {
    if (fileNoExt_.empty()) {
        fileNoExt_ = "d:\\screenrecorder_" + std::to_string(time(nullptr));
        fileFull_ = fileNoExt_ + ".mkv";
    }

    outFilePath_ = str(boost::format("%s_part%02d.mkv") % fileNoExt_ % parts_.size());
    std::string filterComplex = str(boost::format("ddagrab=video_size=%dx%d:offset_x=%d:offset_y=%d") % captureRect_.Width() % captureRect_.Height()
        % captureRect_.left % captureRect_.top);

    std::vector<std::string> args = {
            "-init_hw_device", "d3d11va",
            "-framerate","30",
            /*"-offset_x",std::to_string(captureRect_.left),
            "-offset_y",std::to_string(captureRect_.top),
            "-video_size",std::to_string(captureRect_.Width())+"x" + std::to_string(captureRect_.Height()),*/
            "-filter_complex", filterComplex,
            /*,video_size = "+ std::to_string(captureRect_.Width()) + "x" + std::to_string(captureRect_.Height())
            + ",offset_x=" + std::to_string(captureRect_.left),
            + ",offset_y=" + std::to_string(captureRect_.top),**/
            "-c:v", "h264_nvenc",
            "-cq:v", "20", outFilePath_
    };
    launchFFmpeg(args);
    //thread_ = std::thread();
}

void ScreenRecorder::launchFFmpeg(const std::vector<std::string> args) {
    namespace bp = boost::process;
    std::string command = IuCoreUtils::WstringToUtf8(ffmpegPath_.GetString());


    future_ = std::async(std::launch::async, [&, command] {
        
        try {
            boost::asio::io_service ios;
            std::vector<char> buf;
            /*bp::ipstream stream;
            */
            //bp::async_pipe ap(ios);
            std::future<std::string> data;

            //bp::child c(command, args, /*bp::std_in < inStream, */ bp::std_in.close(), bp::std_out > bp::null, bp::std_err > boost::asio::buffer(buf), ios);
            child_ = std::make_unique<bp::child>(command, args, bp::std_in < inStream_,
                bp::std_out > bp::null, //so it can be written without anything
                bp::std_err > data,
                ios,
                ::boost::process::windows::create_no_window);

            ios.run();
            /*boost::asio::async_read(ap, boost::asio::buffer(buf),
                [](const boost::system::error_code& ec, std::size_t size) {});*/
                /*std::string line;
                while (c.running() && getline(stream, line)) {
                    LOG(WARNING) << line << std::endl;
                }*/

            child_->wait(); // reap PID
            int result = child_->exit_code();
            if (result != 0) {
                LOG(ERROR) << result << std::endl << data.get();
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
    inStream_ << "q" << std::endl;
}

void ScreenRecorder::stop() {
    sendStopSignal();
    parts_.push_back(outFilePath_);

    if (parts_.size() == 1) {
        try {
            std::filesystem::rename(std::filesystem::u8path(outFilePath_), std::filesystem::u8path(fileFull_));
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }
    } else {
        std::string tempFile = std::tmpnam(nullptr);
        std::ofstream f(/*std::filesystem::u8path*/(tempFile), std::ios::out);

        if (!f) {
            LOG(ERROR) << "Failed to create temp file " << tempFile;
            //throw IOException("Failed to create output file: " + std::string(strerror(errno)), filenameUtf8);
        } else {
            for(const auto& item: parts_) {
                f << item << std::endl;
            }
        }
        f.close();
    }
}

void ScreenRecorder::pause() {
    sendStopSignal();

    parts_.push_back(outFilePath_);

    //future_.wait();
    //child_->wait();
    /*if (thread_.joinable()) {
        thread_.join();
    }*/

    /*try {
        std::filesystem::rename(std::filesystem::u8path(outFilePath_), std::filesystem::u8path(fileNoExt_ + "_head.mkv"));
    } catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
    }*/
}