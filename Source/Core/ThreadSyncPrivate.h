#ifndef IU_CORE_UPLOAD_THREADSYNCPRIVATE_H
#define IU_CORE_UPLOAD_THREADSYNCPRIVATE_H

#pragma once
#include <mutex>
#include <memory>
#include <string>
#include <map>
#include "Network/CurlShare.h"

class ThreadSyncPrivate
{
public:
	std::map<std::string, std::string> data_;
	std::mutex dataMutex_;
    CurlShare curlShare_;
    int threadCount_;
    std::mutex threadCountMutex_;
    ThreadSyncPrivate()
    {
        threadCount_ = 0;
    }
};

#endif
