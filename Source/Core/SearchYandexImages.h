#ifndef FUNC_SEARCHYANDEXIMAGES_H
#define FUNC_SEARCHYANDEXIMAGES_H

#include <string>
#include <condition_variable>
#include <mutex>
#include <memory>

#include "SearchByImage.h"
#include "Core/Upload/ServerProfile.h"

class UploadTask;
class NetworkClient;
class UploadManager;

class SearchYandexImages: public SearchByImageTask  {
public:
    explicit SearchYandexImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, UploadManager* uploadManager, const std::string& fileName, ServerProfile temporaryServer);
    void cancel() override;
protected:
    void run() override;
    void onFileFinished(UploadTask*  task, bool ok);
    std::mutex uploadFinishSignalMutex_;
    std::condition_variable uploadFinishSignal_;
    bool uploadOk_;
    std::string uploadErrorMessage_;
    std::string uploadedImageUrl_;
    bool uploadFinished_;
    std::shared_ptr<UploadTask> currentUploadTask_;
    ServerProfile temporaryServer_;
    UploadManager* uploadManager_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
};

#endif