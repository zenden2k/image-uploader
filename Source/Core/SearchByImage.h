#ifndef CORE_SEARCHBYIMAGE_H
#define CORE_SEARCHBYIMAGE_H

#include <string>
#include <atomic>
#include <memory>
#include <functional>

#include "Core/Utils/CoreTypes.h"
#include "Network/INetworkClient.h"
#include "Core/Upload/UploadEngine.h"

class INetworkClient;
class UploadManager;

class SearchByImage  {
    public:
        enum SearchEngine { seGoogle, seYandex};
        explicit SearchByImage(const std::string& fileName);
        virtual ~SearchByImage() = default;
        void start();
        virtual void stop();
        bool isRunning() const;
        static std::unique_ptr<SearchByImage> createSearchEngine(std::shared_ptr<INetworkClientFactory> networkClientFactory, 
            UploadManager* uploadManager, SearchEngine se, const ServerProfile& temporaryServer, const std::string& fileName);
        static std::string getSearchEngineDisplayName(SearchEngine se);
        static std::string searchEngineTypeToString(SearchEngine se);
        static SearchEngine searchEngineTypeFromString(const std::string& name); 
        typedef std::function<void(bool, const std::string&)> FinishedDelegate;
        void setOnFinished(const FinishedDelegate& fd);
protected:
    std::string fileName_;
    std::atomic<bool> isRunning_;
    std::atomic<bool> stopSignal_;
    FinishedDelegate onFinished_;
    virtual void run() = 0;
    void finish(bool success, const std::string &msg = std::string());
    int progressCallback(INetworkClient *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    DISALLOW_COPY_AND_ASSIGN(SearchByImage);
};

#endif