#include "SearchByImage.h"

#include "SearchGoogleImages.h"
#include "SearchYandexImages.h"


std::unique_ptr<SearchByImageTask> SearchByImage::createSearchEngine(std::shared_ptr<INetworkClientFactory> networkClientFactory,UploadManager* uploadManager,
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