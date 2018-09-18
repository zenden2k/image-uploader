#include "SearchByImage.h"

#include "SearchGoogleImages.h"
#include "SearchYandexImages.h"

#include <thread>
#include <Core/Network/NetworkClient.h>


SearchByImage::SearchByImage(const std::string& fileName) {
    fileName_ = fileName;
    isRunning_ = false;
    stopSignal_ = false;
}

SearchByImage::~SearchByImage() {
    
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

void SearchByImage::finish(bool success, const std::string& msg) {
    if (onFinished_) {
        onFinished_(success, msg);
    }
    isRunning_ = false;
}

std::unique_ptr<SearchByImage> SearchByImage::createSearchEngine(SearchEngine se, const std::string& fileName) {
    if (se == seGoogle) {
        return std::make_unique<SearchGoogleImages>(fileName);
    } else if (se == seYandex) {
        return std::make_unique<SearchYandexImages>(fileName);
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