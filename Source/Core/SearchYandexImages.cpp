#include "SearchYandexImages.h"

#include "Core/Network/NetworkClient.h"
#include "Core/Upload/UploadManager.h"
#ifdef _WIN32
	#include "Core/Images/ImageLoader.h"
#endif

#include "Core/Utils/DesktopUtils.h"

SearchYandexImages::SearchYandexImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, const std::string& fileName)
    :SearchByImageTask(fileName),
	networkClientFactory_(std::move(networkClientFactory))
{
    uploadOk_ = false;
}

void SearchYandexImages::run() {
    int width = 500;
    int height = 500;
#ifdef _WIN32
    ImageLoader loader;
    auto img = loader.loadFromFile(IuCoreUtils::Utf8ToWstring(fileName_).c_str());
	if (img) {
        width = img->GetWidth();
        height = img->GetHeight();
	}
#endif

    auto nc = networkClientFactory_->create();
    using namespace std::placeholders;
    nc->setProgressCallback(std::bind(&SearchYandexImages::progressCallback, this, _1, _2, _3, _4, _5));

    try {
        nc->setUrl("https://yandex.ru/images/search?rpt=imageview");
        nc->addQueryHeader("Origin", "https://yandex.ru");
        nc->addQueryParamFile("upfile", fileName_, IuCoreUtils::ExtractFileName(fileName_), "");
        nc->addQueryParam("original_width", std::to_string(width));
        nc->addQueryParam("original_height", std::to_string(height));
        nc->addQueryParam("prg", "1");
        nc->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
        //nc->setUserAgent("Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/68.0.3440.106 YaBrowser/18.9.0.3409 Yowser/2.5 Safari/537.36");
        nc->doUploadMultipartData();

        if (stopSignal_) {
            finish(false, "Aborted by user.");
            return;
        }
        if (nc->responseCode() < 301 || nc->responseCode() > 303) {
            finish(false, "Server sent unexpected result.");
            return;
        }

        std::string url = nc->responseHeaderByName("Location");

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


/*void SearchYandexImages::run() {
    if (stopSignal_) {
        finish(false, "Aborted by user.");
        return;
    }
    FileUploadTask *  task(new FileUploadTask(fileName_, IuCoreUtils::ExtractFileName(fileName_)));
    task->setIsImage(true);
    std::shared_ptr<UploadSession> uploadSession = std::make_shared<UploadSession>(false);
    task->setServerProfile(temporaryServer_);
    using namespace std::placeholders;
    task->onTaskFinished.connect(std::bind(&SearchYandexImages::onFileFinished, this, _1, _2));

    currentUploadTask_.reset(task);
    uploadSession->addTask(currentUploadTask_);
    uploadManager_->addSession(uploadSession);

    // Wait until upload session is finished
    std::unique_lock<std::mutex> lk(uploadFinishSignalMutex_);
    while (!uploadFinished_) {
        uploadFinishSignal_.wait(lk);
    }

    if (stopSignal_) {
        finish(false, "Aborted by user.");
        return;
    }

    if (uploadOk_) {
        NetworkClient nc;
        const std::string urlToOpen = "https://yandex.com/images/search?rpt=imageview&img_url=" + nc.urlEncode(uploadedImageUrl_);
        if (!DesktopUtils::ShellOpenUrl(urlToOpen)) {
            finish(false, "Unable to launch default web browser.");
            return;
        }
        
    } else {
        finish(false, uploadErrorMessage_);
        return;
    }

    finish(true);
}*/

/*
void SearchYandexImages::onFileFinished(UploadTask* task, bool ok) {
    
    if (ok) {
        const std::string url = task->uploadResult()->directUrl;
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
}*/