#include "SearchByImage.h"

#include <thread>

#include "SearchGoogleImages.h"
#include "SearchYandexImages.h"

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

void SearchByImage::setOnFinished(const FinishedDelegate& fd) {
    onFinished_ = fd;
}

int SearchByImage::progressCallback(INetworkClient *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (stopSignal_) {
        return -1;
    }
    return 0;
}

void SearchByImage::finish(bool success, const std::string& msg) {
    if (onFinished_) {
        onFinished_(success, msg);
    }
    isRunning_ = false;
}

std::unique_ptr<SearchByImage> SearchByImage::createSearchEngine(std::shared_ptr<INetworkClientFactory> networkClientFactory,UploadManager* uploadManager,
    SearchEngine se, const ServerProfile& temporaryServer, const std::string& fileName) {
    if (se == seGoogle) {
        return std::make_unique<SearchGoogleImages>(networkClientFactory, fileName);
    } else if (se == seYandex) {
        return std::make_unique<SearchYandexImages>(uploadManager, fileName, temporaryServer);
    }
    return nullptr;
}

std::string SearchByImage::getSearchEngineDisplayName(SearchEngine se) {
    if (se == seGoogle) {
        return "Google";
    } else if (se == seYandex) {
        return "Yandex";
    }
    return std::string();
}

std::string SearchByImage::searchEngineTypeToString(SearchEngine se) {
    if (se == seGoogle) {
        return "google";
    } else if (se == seYandex) {
        return "yandex";
    }
    return std::string();
}

SearchByImage::SearchEngine SearchByImage::searchEngineTypeFromString(const std::string& name) {
    if (name == "yandex") {
        return seYandex;
    }
    return seGoogle;
}