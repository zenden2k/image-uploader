#ifndef IU_CORE_MYLOGSINK_H
#define IU_CORE_MYLOGSINK_H

#pragma once

#include "Core/Logging/Logger.h"

class GOOGLE_GLOG_DLL_DECL MyLogSink: public google::LogSink {
public:
    MyLogSink(ILogger* logger);
    virtual void send(google::LogSeverity severity, const char* full_filename,
        const char* base_filename, int line,
        const struct ::tm* tm_time,
        const char* message, size_t message_len) override;
protected:
    ILogger* logger_;
}; 

#endif