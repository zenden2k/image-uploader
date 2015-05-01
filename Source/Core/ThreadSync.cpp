#include "ThreadSync.h"
#include "Core/Logging.h"
#include <map>
#include <atomic>
#include "ThreadSyncPrivate.h"


ThreadSync::ThreadSync() : d_ptr(new ThreadSyncPrivate())
{
}

ThreadSync::ThreadSync(ThreadSyncPrivate* priv) : d_ptr(priv)
{

}

ThreadSync::~ThreadSync()
{
}

void ThreadSync::setValue(const std::string& name, const std::string& value)
{
    std::lock_guard<std::mutex> lock(d_ptr->dataMutex_);
    d_ptr->data_[name] = value;
}

const std::string ThreadSync::getValue(const std::string& name)
{
    std::lock_guard<std::mutex> lock(d_ptr->dataMutex_);
    auto it = d_ptr->data_.find(name);
    if (it == d_ptr->data_.end()) {
        return std::string();
    } 
    return it->second;
}
