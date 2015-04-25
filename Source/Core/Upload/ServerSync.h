#ifndef IU_CORE_UPLOAD_SERVERSYNC_H
#define IU_CORE_UPLOAD_SERVERSYNC_H

#pragma once
#include <mutex>
#include <memory>

class ServerSyncPrivate;

/**
	@brief ServerSync class contains server- (and account) specific data and tools for syncronization 
	threads which are uploading to the same server (and account)
*/
class ServerSync
{
	public:
		ServerSync();
		bool beginLogin();
		bool endLogin();
		void setValue(const std::string& name, const std::string& value);
		const std::string getValue(const std::string& name);
	protected:
		std::shared_ptr<ServerSyncPrivate> d_; // ServerSync should not be copied, but we need to use pimpl idiom for Squirrel binding
};

#endif
