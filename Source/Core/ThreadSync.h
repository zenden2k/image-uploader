#ifndef IU_CORE_UPLOAD_THREADSYNC_H
#define IU_CORE_UPLOAD_THREADSYNC_H

#pragma once
#include <mutex>
#include <memory>
#include "Core/Utils/CoreTypes.h"

class ThreadSyncPrivate;

/**
	@brief ThreadSync class contains script-specific data and tools for syncronization 
	threads of the same script
*/
class ThreadSync
{
	public:
        ThreadSync();
        explicit ThreadSync(ThreadSyncPrivate* priv);
        virtual ~ThreadSync();
		void setValue(const std::string& name, const std::string& value);
		const std::string getValue(const std::string& name);
	protected:
		std::shared_ptr<ThreadSyncPrivate> d_ptr; // ServerSync should not be copied, but we need to use pimpl idiom for Squirrel binding
        Q_DECLARE_PRIVATE_PTR(ThreadSync);
};

#endif
