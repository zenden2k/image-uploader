#pragma once

#include <atomic>
#include <string>
#include <memory>

#include <boost/signals2.hpp>

#include "Core/TaskDispatcher.h"
#include "Core/Network/INetworkClient.h"

namespace ServersListTool {

class CheckShortUrlTask : public CancellableTask {
public:
    explicit CheckShortUrlTask(std::shared_ptr<INetworkClientFactory> factory, std::string url, std::string shortUrl);
    void run() override;
    void cancel() override;
    bool isCanceled() override;
    bool isInProgress() override;

    boost::signals2::signal<void(CheckShortUrlTask*, bool)> onTaskFinished;

private:
    std::atomic<bool> isCanceled_;
    std::atomic<bool> isInProgress_;
    std::shared_ptr<INetworkClientFactory> factory_;
    std::string url_, shortUrl_;
};

}
