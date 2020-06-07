#ifndef IU_CORE_DOWNLOADTASK_H
#define IU_CORE_DOWNLOADTASK_H

#include <atomic>

#include <boost/signals2.hpp>

#include "TaskDispatcher.h"
#include "Core/Network/INetworkClient.h"

class DownloadTask: public CancellableTask {
public:
    struct DownloadItem
    {
        std::string fileName;
        std::string displayName;
        std::string url;
        std::string referer;
        void* id; // pointer to user data
    };

    explicit DownloadTask(std::shared_ptr<INetworkClientFactory> factory, const std::string& tempDirectory, const std::vector<DownloadItem>& files);
    void run() override;
    void cancel() override;
    bool isCanceled() override;
    bool isInProgress() override;

    boost::signals2::signal<void(bool, int, const DownloadItem&)> onFileFinished;
    boost::signals2::signal<void(DownloadTask*)> onTaskFinished;

private:
    std::atomic<bool> isCanceled_;
    std::atomic<bool> isInProgress_;
    std::shared_ptr<INetworkClientFactory> factory_;
    std::vector<DownloadItem> files_;
    std::string tempDirectory_;

    int progressCallback(INetworkClient* userData, double dltotal, double dlnow, double ultotal, double ulnow);
};

#endif