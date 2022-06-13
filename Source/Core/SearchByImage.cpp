#include "SearchByImage.h"

#include "SearchGoogleImages.h"
#include "SearchYandexImages.h"

std::unique_ptr<SearchByImageTask> SearchByImage::createSearchEngine(std::shared_ptr<INetworkClientFactory> networkClientFactory, UploadManager* uploadManager,
    SearchEngine se, const ServerProfile& temporaryServer, const std::string& fileName) {
    if (se == SearchEngine::seGoogle) {
        return std::make_unique<SearchGoogleImages>(std::move(networkClientFactory), fileName);
    } else if (se == SearchEngine::seYandex) {
        return std::make_unique<SearchYandexImages>(std::move(networkClientFactory), uploadManager, fileName, temporaryServer);
    }
    return nullptr;
}

std::string SearchByImage::getSearchEngineDisplayName(SearchEngine se) {
    if (se == SearchEngine::seGoogle) {
        return "Google";
    } else if (se == SearchEngine::seYandex) {
        return "Yandex";
    }
    return {};
}

std::string SearchByImage::searchEngineTypeToString(SearchEngine se) {
    if (se == SearchEngine::seGoogle) {
        return "google";
    } else if (se == SearchEngine::seYandex) {
        return "yandex";
    }
    return {};
}

SearchByImage::SearchEngine SearchByImage::searchEngineTypeFromString(const std::string& name) {
    if (name == "yandex") {
        return SearchEngine::seYandex;
    }
    return SearchEngine::seGoogle;
}