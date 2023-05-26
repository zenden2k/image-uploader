#include "ScreenRecorder.h"

#include <boost/process.hpp>

#include "Core/Logging.h"
#include "Core/Utils/CoreUtils.h"

ScreenRecorder::ScreenRecorder(CString ffmpegPath, CRect rect): ffmpegPath_(ffmpegPath), captureRect_(rect){
    
}

ScreenRecorder::~ScreenRecorder() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void ScreenRecorder::start() {
    namespace bp = boost::process;
    std::string command = IuCoreUtils::WstringToUtf8(ffmpegPath_.GetString());

    thread_ = std::thread([&,command] {
        
        std::vector<std::string> args = {"--version"};
        try {
            bp::ipstream stream;
            bp::opstream inStream;
            bp::child c(command, args, /*bp::std_in < inStream, */bp::std_err > stream);

            std::string line;
            while (c.running() && getline(stream, line)) {
                LOG(WARNING) << line << std::endl;
            }

            c.wait(); // reap PID
        }catch (const bp::process_error& ex) {
            LOG(ERROR) << IuCoreUtils::SystemLocaleToUtf8(ex.what()) << std::endl;
        }catch  (const std::exception& ex) {
            LOG(ERROR) << ex.what() << std::endl;
        }

    });
}