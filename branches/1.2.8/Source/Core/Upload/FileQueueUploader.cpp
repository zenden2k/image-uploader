/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FileQueueUploader.h"
#include <algorithm>
#include <zthread/Thread.h>
#include <zthread/Mutex.h>

#ifndef IU_CLI
	#include "Gui/Dialogs/LogWindow.h"
#endif
#include "Core/Upload/Uploader.h"

struct ServerThreadsInfo {
	//int maxThreads;
	int runningThreads;
	int waitingFileCount;
};
class CFileQueueUploader::Impl {
	public:
		Impl(CFileQueueUploader* queueUploader);
		virtual ~Impl();
		void AddFile(const std::string& fileName, const std::string& displayName, void* user_data,CAbstractUploadEngine *uploadEngine);
		void AddFile(UploadTask task);
		void start();
		virtual void run();
		bool getNextJob(UploadTask* item);

		ZThread::Mutex mutex_;
		ZThread::Mutex callMutex_;
		Callback* callback_;
		CFileQueueUploader *queueUploader_;
		volatile bool m_NeedStop;
		bool m_IsRunning;
		CAbstractUploadEngine* m_engine;
		ServerSettingsStruct m_serverSettings;
		std::vector<UploadTask> m_fileList;
		int m_nThreadCount;
		int m_nRunningThreads;
		friend class CFileQueueUploader;
	protected:
		class Runnable;
		bool onNeedStopHandler();
		void OnConfigureNetworkManager(NetworkManager* nm);
		void onProgress(CUploader*, InfoProgress progress );
		std::map<CUploader*, UploadTask*> tasks_;
		std::map<std::string, ServerThreadsInfo> serverThreads_;
		
};

class CFileQueueUploader::Impl::Runnable : public ZThread::Runnable {
	public:
		CFileQueueUploader::Impl* uploader_;
		Runnable(CFileQueueUploader::Impl* uploader)
		{
			uploader_ = uploader;
		}

		virtual void run()
		{
			uploader_->run();
		}
};

/* private CFileQueueUploader::Impl class */

CFileQueueUploader::Impl::Impl(CFileQueueUploader* queueUploader) {
	m_nThreadCount = 1;
	callback_ = 0;
	m_NeedStop = false;
	m_IsRunning = false;
	m_nRunningThreads = 0;
	m_engine = 0;
	queueUploader_ = queueUploader;
}

CFileQueueUploader::Impl::~Impl() {
}

bool CFileQueueUploader::Impl::onNeedStopHandler() {
	return m_NeedStop;
}

void CFileQueueUploader::Impl::onProgress(CUploader* uploader, InfoProgress progress ) {
	UploadProgress prog;
	prog.totalUpload = progress.Total;
	prog.uploaded    = progress.Uploaded;
	if ( callback_ ) {
		callback_->OnUploadProgress(prog, tasks_[uploader], 0);
	}
}

void CFileQueueUploader::Impl::OnConfigureNetworkManager(NetworkManager* nm)
{
	if (callback_) {
		callback_->OnConfigureNetworkManager(queueUploader_,nm);
	}
}

bool CFileQueueUploader::Impl::getNextJob(UploadTask* item) {
	if (m_NeedStop)
		return false;
	std::string url;
	mutex_.acquire();
	bool result = false;
	if (!m_fileList.empty() && !m_NeedStop)
	{
		*item = *m_fileList.begin();
		m_fileList.erase(m_fileList.begin());
		result = true;
	}
	mutex_.release();
	return result;
}

void CFileQueueUploader::Impl::AddFile(const std::string& fileName, const std::string& displayName, void* user_data, CAbstractUploadEngine *uploadEngine) {
	UploadTask newTask;
	newTask.userData = user_data;
	newTask.displayFileName = displayName;
	newTask.fileName = fileName;
	newTask.uploadEngine = uploadEngine;
	newTask.fileSize = IuCoreUtils::getFileSize(fileName);
	newTask.serverName = uploadEngine->getUploadData()->Name;
	AddFile( newTask );
}

void CFileQueueUploader::Impl::start() {
	mutex_.acquire();
	m_NeedStop = false;
	m_IsRunning = true;
	int numThreads = std::min<int>(size_t(m_nThreadCount-m_nRunningThreads), m_fileList.size());
	
	for (int i = 0; i < numThreads; i++)
	{
		m_nRunningThreads++;
		ZThread::Thread t1(new Runnable(this)); // starting new thread
	}
	mutex_.release();
}

void CFileQueueUploader::Impl::AddFile(UploadTask task){
	m_fileList.push_back(task);
	serverThreads_[task.serverName].waitingFileCount++;
}

void CFileQueueUploader::Impl::run()
{
	CUploader uploader;
	uploader.onConfigureNetworkManager.bind(this, &CFileQueueUploader::Impl::OnConfigureNetworkManager);
#ifndef IU_CLI
	// TODO
	uploader.onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
#endif
	for (;; )
	{
		UploadTask it;
		if (!getNextJob(&it))
			break;
		serverThreads_[it.serverName].waitingFileCount--;

		std::string serverName = it.serverName;
		serverThreads_[serverName].runningThreads ++;	
		uploader.setUploadEngine(it.uploadEngine);
		uploader.onNeedStop.bind(this, &Impl::onNeedStopHandler);
		uploader.onProgress.bind(this, &Impl::onProgress);
		mutex_.acquire();
		tasks_[&uploader] = &it;
mutex_.release();
		bool res = uploader.UploadFile(it.fileName, it.displayFileName.c_str());

		mutex_.acquire();
		serverThreads_[serverName].runningThreads --;
		mutex_.release();
		
		// m_CS.Lock();
		callMutex_.acquire();
		FileListItem result;
		result.uploadTask = &it;
		if (res) {
			
			result.imageUrl = (uploader.getDirectUrl());
			result.downloadUrl = (uploader.getDownloadUrl());
			result.thumbUrl = (uploader.getThumbUrl());
			if (callback_) {
				callback_->OnFileFinished(true, result);
			}
		}
		else
		{
			if (callback_) {
				callback_->OnFileFinished(false, result);
			}
		}
		callMutex_.release();
	}
	mutex_.acquire();
	m_nRunningThreads--;
	mutex_.release();
	if (!m_nRunningThreads)
	{
		m_IsRunning = false;
		if (callback_) {
			callback_->OnQueueFinished(queueUploader_);
		}
	}
	
}

/* public CFileQueueUploader class */

CFileQueueUploader::CFileQueueUploader()
{
	_impl = new Impl(this);
}

void CFileQueueUploader::AddFile(const std::string& fileName, const std::string& displayName, void* user_data, CAbstractUploadEngine *uploadEngine)
{
	_impl->AddFile(fileName, displayName, user_data, uploadEngine);
}

bool CFileQueueUploader::start()
{
	_impl->start();
	return true;
}

void CFileQueueUploader::setCallback(Callback* callback)
{
	_impl->callback_ = callback;
}

void CFileQueueUploader::stop()
{
	_impl->m_NeedStop = true;
}

bool CFileQueueUploader::IsRunning() const {
	return _impl->m_IsRunning;
}

void CFileQueueUploader::setUploadSettings(CAbstractUploadEngine* engine)
{
	_impl->m_engine = engine;
}

CFileQueueUploader::~CFileQueueUploader() {
	delete _impl;
}


void CFileQueueUploader::setMaxThreadCount(int threadCount) {
	_impl->m_nThreadCount = threadCount;
}

bool CFileQueueUploader::isSlotAvailableForServer(std::string serverName, int maxThreads) {
	int threads = _impl->serverThreads_[serverName].runningThreads + _impl->serverThreads_[serverName].waitingFileCount;
	return threads < maxThreads && threads < _impl->m_nThreadCount;
}