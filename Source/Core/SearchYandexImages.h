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

class SearchYandexImages: public SearchByImageTask {
public:
    explicit SearchYandexImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, const std::string& fileName);
protected:
    BackgroundTaskResult doJob() override;
    bool uploadOk_;
    std::string uploadErrorMessage_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
};

#endif