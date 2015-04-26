#ifndef IU_CORE_UPLOAD_THREADSYNCPRIVATE_H
#define IU_CORE_UPLOAD_THREADSYNCPRIVATE_H

#pragma once
#include <mutex>
#include <memory>
#include <string>

class ThreadSyncPrivate
{
public:
	std::map<std::string, std::string> data_;
	std::mutex dataMutex_;
};

#endif
