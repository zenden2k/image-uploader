#ifndef CORE_SEARCHBYIMAGE_H
#define CORE_SEARCHBYIMAGE_H

#include <string>
#include <memory>

#include "Core/Upload/UploadEngine.h"

class UploadManager;

class SearchByImage  {
    public:
        //enum class SearchEngine { seGoogle, seYandex};

        static std::shared_ptr<UploadSession> search(const std::string& fileName, const ServerProfile& imageSearchServer, const ServerProfile& temporaryServer, UploadManager* uploadManager);

        /* static std::unique_ptr<SearchByImageTask> createSearchEngine(std::shared_ptr<INetworkClientFactory> networkClientFactory, 
            UploadManager* uploadManager, SearchEngine se, const ServerProfile& temporaryServer, const std::string& fileName);

        static std::string getSearchEngineDisplayName(SearchEngine se);
        static std::string searchEngineTypeToString(SearchEngine se);
        static SearchEngine searchEngineTypeFromString(const std::string& name); */
        SearchByImage() = delete;
};

#endif
