#ifndef FUNC_SEARCHYANDEXIMAGES_H
#define FUNC_SEARCHYANDEXIMAGES_H

#include <string>
#include <condition_variable>
#include <mutex>

#include "SearchByImage.h"
#include "Core/Upload/ServerProfile.h"

class UploadTask;
class NetworkClient;

class SearchYandexImages: public SearchByImage  {

    public:
        explicit SearchYandexImages(const std::string& fileName, const ServerProfile& temporaryServer);
        void stop() override;
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
};

#endif