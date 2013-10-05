#ifndef FUNC_LOCALFILECACHE_H
#define FUNC_LOCALFILECACHE_H

#include <string>
#include <map>
#include <zthread/Mutex.h>

class LocalFileCache {
	public:
		static LocalFileCache& instance() {
			static LocalFileCache singleInstance;
			return singleInstance;
		}
		bool ensureHistoryParsed();
		bool addFile(const std::string& url, const std::string& localFileName);
		bool addThumb(const std::string& url, const std::string& thumb);
		std::string get(const std::string& url);
		std::string getThumb(const std::string& url);
	private:
		bool historyParsed;
		std::map<std::string, std::string> cache_;
		std::map<std::string, std::string> thumbCache_;
		ZThread::Mutex mutex_;
		ZThread::Mutex cacheMutex_;
		bool parseHistory();
		bool parseHistoryFile(const CString& fileName);
		LocalFileCache();
		LocalFileCache(const LocalFileCache& root);
		LocalFileCache& operator=(const LocalFileCache&);
};

#endif