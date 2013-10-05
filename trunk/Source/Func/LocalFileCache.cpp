#include "LocalFileCache.h"

#include "Settings.h"
#include "HistoryManager.h"
#include <Gui/Dialogs/LogWindow.h>

#include <zthread/Guard.h>
LocalFileCache::LocalFileCache() {
	historyParsed = false;
}

bool LocalFileCache::ensureHistoryParsed() {
	if ( !historyParsed ) {
		parseHistory();
		historyParsed = true;
	}

	return true;
}

bool LocalFileCache::parseHistory() {
	mutex_.acquire();
	std::vector<CString> files;
	CString historyFolder = Settings.SettingsFolder+_T("\\History\\");
	GetFolderFileList(files, historyFolder , _T("history*.xml"));

	for(size_t i=0; i<files.size(); i++) {
		parseHistoryFile(historyFolder + files[i]);
	}
	mutex_.release();
	return true;
}

bool LocalFileCache::parseHistoryFile(const CString& fileName) {
	CHistoryReader m_historyReader;
	if ( !m_historyReader.loadFromFile(WCstringToUtf8(fileName)) ) {
		return false;
	}
	int nSessionsCount = m_historyReader.getSessionCount();
	int nFilesCount = 0;

	for( int i = 0; i < nSessionsCount; i++ ) {
		CHistorySession* ses = m_historyReader.getSession(i);
		int nCount = ses->entriesCount();
		for ( int j = 0; j < nCount; j++ ){
			nFilesCount++;
			HistoryItem historyItem = ses->entry(j);
			addFile(historyItem.directUrl, historyItem.localFilePath);
		}
	}
	return true;
}

bool LocalFileCache::addFile(const std::string& url, const std::string& localFileName) {
	cacheMutex_.acquire();
	cache_[url] = localFileName; 
	cacheMutex_.release();
	return true;
}

std::string LocalFileCache::get(const std::string& url){
	ZThread::Guard<ZThread::Mutex> guard(cacheMutex_);
	std::map<std::string, std::string>::const_iterator foundItem = cache_.find(url);

	if ( foundItem != cache_.end() ) {
		if ( IuCoreUtils::FileExists(foundItem->second) ) {
			return foundItem->second;
		} else {
			// if file not found on disk, delete it from cache
			cache_.erase(foundItem);
		}
	}
	return std::string();
}