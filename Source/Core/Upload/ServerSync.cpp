#include "ServerSync.h"

#include "Core/Logging.h"
#include "Core/ThreadSyncPrivate.h"
#include <map>
#include <atomic>


class ServerSyncPrivate: public ThreadSyncPrivate
{
public:
	std::mutex loginMutex_;
    std::atomic<bool> authPerformed_;
    std::atomic<bool> authPerformedSuccess_;
};
ServerSync::ServerSync() : ThreadSync(new ServerSyncPrivate())
{
    Q_D(ServerSync);
    d->authPerformed_ = false;
    d->authPerformedSuccess_ = false;
}

bool ServerSync::beginAuth()
{
    Q_D(ServerSync);
	try {
		d->loginMutex_.lock();
	} catch (std::exception ex) {
		LOG(ERROR) << "ServerSync::beginLogin exception: " << ex.what();
		return false;
	}
	return true;
}

bool ServerSync::endAuth()
{
    Q_D(ServerSync);
	try
	{
		d->loginMutex_.unlock();
	} catch (std::exception ex)
	{
		LOG(ERROR) << "ServerSync::endLogin exception: " << ex.what();
		return false;
	}
	
	return true;
}

void ServerSync::setAuthPerformed(bool success)
{
    Q_D(ServerSync);
    d->authPerformed_ = true;
    d->authPerformedSuccess_ = success;
}

bool ServerSync::isAuthPerformed()
{
    Q_D(ServerSync);
    return d->authPerformed_;
}
