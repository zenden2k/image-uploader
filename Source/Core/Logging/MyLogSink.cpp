#include "MyLogSink.h"
#include "Core/Utils/CoreUtils.h"

MyLogSink::MyLogSink(ILogger* logger) {
    logger_ = logger;
}

void MyLogSink::send(google::LogSeverity severity, const char* full_filename, const char* base_filename, int line, const struct ::tm* tm_time, const char* message, size_t message_len)
{
    std::string sender = base_filename;
    sender += ":"+IuCoreUtils::int64_tToString(line);
    logger_->write(severity == google::GLOG_ERROR ? logError : logWarning, sender, message);
}

