#include "ServerSync.h"
#include "Core/Logging.h"
#include <map>
class ServerSyncPrivate
{
public:
	std::map<std::string, std::string> data_;
	std::mutex dataMutex_;
	std::mutex loginMutex_;
};
ServerSync::ServerSync() : d_(new ServerSyncPrivate())
{
}

bool ServerSync::beginLogin()
{
	try {
		d_->loginMutex_.lock();
	} catch (std::exception ex) {
		LOG(ERROR) << "ServerSync::beginLogin exception: " << ex.what();
		return false;
	}
	return true;
}

bool ServerSync::endLogin()
{
	try
	{
		d_->loginMutex_.unlock();
	} catch (std::exception ex)
	{
		LOG(ERROR) << "ServerSync::endLogin exception: " << ex.what();
		return false;
	}
	
	return true;
}

void ServerSync::setValue(const std::string& name, const std::string& value)
{
	std::lock_guard<std::mutex> lock(d_->dataMutex_);
	d_->data_[name] = value;
}

const std::string ServerSync::getValue(const std::string& name)
{
	std::lock_guard<std::mutex> lock(d_->dataMutex_);
	auto it = d_->data_.find(name);
	if (it == d_->data_.end()) {
		return std::string();
	} 
	return it->second;
}
