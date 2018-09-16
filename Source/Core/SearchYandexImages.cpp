#include "SearchYandexImages.h"

#include <Core/Network/NetworkClient.h>
#include "Core/Upload/UploadManager.h"
#include "CoreFunctions.h"
#include "Utils/CryptoUtils.h"
#include "Core/Utils/DesktopUtils.h"
#include "Upload/FileUploadTask.h"
#include "Core/Settings.h"
#include "ServiceLocator.h"
#include <assert.h>  

SearchYandexImages::SearchYandexImages(const std::string& fileName) :SearchByImage(fileName) {
    uploadFinished_ = false;
    uploadOk_ = false;
}

/*void SearchYandexImages::run() {
    NetworkClient nc;
    CoreFunctions::ConfigureProxy(&nc);
    nc.setProgressCallback(NetworkClient::ProgressCallback(this, &SearchYandexImages::progressCallback));

    try {
        nc.setUrl("https://yandex.ru/images/search?rpt=imageview&cbird=5");
        nc.addQueryHeader("Origin", "https://yandex.ru");
        nc.addQueryParamFile("upfile", fileName_, IuCoreUtils::ExtractFileName(fileName_));
        nc.setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
        nc.setUserAgent("Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/68.0.3440.106 YaBrowser/18.9.0.3409 Yowser/2.5 Safari/537.36");
        nc.doUploadMultipartData();

        if (stopSignal_) {
            finish(false, "Aborted by user.");
            return;
        }
        if (nc.responseCode() < 301 || nc.responseCode() > 303) {
            finish(false, "Server sent unexpected result.");
            return;
        }

        std::string url = nc.responseHeaderByName("Location");

        if (url.empty()) {
            finish(false, "Server sent unexpected result.");
            return;
        }

        if (!DesktopUtils::ShellOpenUrl(url)) {
            finish(false, "Unable to launch default web browser.");
            return;
        }
    } catch (NetworkClient::AbortedException&){
        finish(false, "Aborted by user.");
        return;
    }
    
    finish(true);
}
*/

void SearchYandexImages::run() {
    UploadManager* uploadManager = ServiceLocator::instance()->uploadManager();
    assert(uploadManager != nullptr);
    FileUploadTask *  task(new FileUploadTask(fileName_, IuCoreUtils::ExtractFileName(fileName_)));
    task->setIsImage(true);
    //std::shared_ptr<UploadSession> uploadSession(new UploadSession());
    task->setServerProfile(Settings.quickScreenshotServer);
    task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &SearchYandexImages::onFileFinished));

    uploadManager->addTask(std::shared_ptr<UploadTask>(task));
    uploadManager->start();

    // Wait until upload session is finished
    std::unique_lock<std::mutex> lk(uploadFinishSignalMutex_);
    while (!uploadFinished_) {
        uploadFinishSignal_.wait(lk);
    }

    if (uploadOk_) {
        NetworkClient nc;
        std::string urlToOpen = "https://www.yandex.com/images/search?rpt=imageview&img_url=" + nc.urlEncode(uploadedImageUrl_);
        if (!DesktopUtils::ShellOpenUrl(urlToOpen)) {
            finish(false, "Unable to launch default web browser.");
            return;
        }
        
    } else {
        finish(false, uploadErrorMessage_);
    }

    finish(true);
}


void SearchYandexImages::onFileFinished(UploadTask* task, bool ok) {
    
    if (ok) {
        std::string url = task->uploadResult()->directUrl;
        if (url.empty()) {
            uploadOk_ = false;
            uploadErrorMessage_ = "Image hosting server doesn't support direct links.";
        } else {
            uploadOk_ = true;
            uploadedImageUrl_ = url;
        }

    } else {
        uploadOk_ = false;
        uploadErrorMessage_ = "Unable to upload image.";
    }
    {
        // LOG(ERROR) << "Sending finish signal" << std::endl;
        std::lock_guard<std::mutex> lk(uploadFinishSignalMutex_);
        uploadFinished_ = true;
    }

    uploadFinishSignal_.notify_one();

}