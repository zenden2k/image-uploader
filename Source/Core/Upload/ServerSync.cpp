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
    std::map<std::string, std::string> constVars_;
    std::mutex  constVarsMutex_;
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
    if (d->authPerformed_ && !d->authPerformedSuccess_)
    {
        d->loginMutex_.unlock();
        throw ServerSyncException("Upload aborted: Authentication failed");
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

void ServerSync::resetAuthorization()
{
    Q_D(ServerSync);
    d->authPerformed_ = false;
    d->authPerformedSuccess_ = false;
}

void ServerSync::resetFailedAuthorization()
{
    Q_D(ServerSync);
    std::lock_guard<std::mutex> lock(d_ptr->threadCountMutex_);
    if (!d_ptr->threadCount_ && d->authPerformed_ && !d->authPerformedSuccess_ )
    {
        d->authPerformed_ = false;
    }
}

void ServerSync::setConstVar(const std::string& name, const std::string& value)
{
    Q_D(ServerSync);
    std::lock_guard<std::mutex> lock(d->constVarsMutex_);
    d->constVars_[name] = value;
}

std::string ServerSync::getConstVar(const std::string& name)
{
    Q_D(ServerSync);
    std::lock_guard<std::mutex> lock(d->constVarsMutex_);
    auto it = d->constVars_.find(name);
    if (it != d->constVars_.end())
    {
        return it->second;
    }
    return std::string();
}