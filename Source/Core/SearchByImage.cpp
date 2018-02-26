#include "SearchByImage.h"

#include <thread>
#include <Core/Network/NetworkClient.h>
#include "CoreFunctions.h"
#include "Utils/CryptoUtils.h"
#include "Core/Utils/DesktopUtils.h"

SearchByImage::SearchByImage(const std::string& fileName) {
    fileName_ = fileName;
    isRunning_ = false;
    stopSignal_ = false;
}

void SearchByImage::start() {
    isRunning_ = true;
    std::thread t(&SearchByImage::run, this);
    t.detach();
}

void SearchByImage::stop() {
    stopSignal_ = true;
}

bool SearchByImage::isRunning() const {
    return isRunning_;
}

void SearchByImage::setOnFinished(FinishedDelegate&& fd) {
    onFinished_ = std::move(fd);
}


int SearchByImage::progressCallback(NetworkClient *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (stopSignal_) {
        return -1;
    }
    return 0;
}

void SearchByImage::run() {
    NetworkClient nc;
    CoreFunctions::ConfigureProxy(&nc);
    nc.setProgressCallback(NetworkClient::ProgressCallback(this, &SearchByImage::progressCallback));

    try {
        nc.setUrl("https://images.google.com/searchbyimage/upload");
        nc.addQueryParam("filename", IuCoreUtils::ExtractFileName(fileName_));
        nc.addQueryParam("image_content", base64EncodeCompat(fileName_));
        nc.setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
        nc.doUploadMultipartData();

        if (stopSignal_) {
            finish(false, "Aborted by user.");
            return;
        }
        if (nc.responseCode() != 302) {
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

void SearchByImage::finish(bool success, const std::string& msg) {
    if (onFinished_) {
        onFinished_(success, msg);
    }
    isRunning_ = false;
}

std::string SearchByImage::base64EncodeCompat(const std::string& file) {
    std::string res = IuCoreUtils::CryptoUtils::Base64Encode(IuCoreUtils::GetFileContents(file));
    for (unsigned int i = 0; i < res.length(); i++) {
        if (res[i] == '+') {
            res[i] = '-';
        } else if (res[i] == '/') {
            res[i] = '_';
        }
    }
    return res;
}