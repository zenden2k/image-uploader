#ifndef FUNC_LOCALFILECACHE_H
#define FUNC_LOCALFILECACHE_H

#include <string>
#include <map>
#include <mutex>
#include "Core/Utils/Singleton.h"

class LocalFileCache : public Singleton<LocalFileCache> {
    public:
        bool ensureHistoryParsed();
        bool addFile(const std::string& url, const std::string& localFileName);
        bool addThumb(const std::string& url, const std::string& thumb);
        std::string get(const std::string& url);
        std::string getThumb(const std::string& url);
        friend class Singleton<LocalFileCache>;
    protected:
        bool historyParsed;
        std::map<std::string, std::string> cache_;
        std::map<std::string, std::string> thumbCache_;
        std::recursive_mutex mutex_;
        std::mutex cacheMutex_;
        bool parseHistory();
        bool parseHistoryFile(const std::string& fileName);
        LocalFileCache();
};

#endif