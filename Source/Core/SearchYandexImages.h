#ifndef FUNC_SEARCHYANDEXIMAGES_H
#define FUNC_SEARCHYANDEXIMAGES_H

#include "SearchByImage.h"
#include <string>
#include <condition_variable>
#include <mutex>

class UploadTask;
class NetworkClient;

class SearchYandexImages: public SearchByImage  {

    public:
        explicit SearchYandexImages(const std::string& fileName);
protected:
    void run() override;
    void onFileFinished(UploadTask*  task, bool ok);
    std::mutex uploadFinishSignalMutex_;
    std::condition_variable uploadFinishSignal_;
    bool uploadOk_;
    std::string uploadErrorMessage_;
    std::string uploadedImageUrl_;
    bool uploadFinished_;
};

#endif