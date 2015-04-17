#include "FileQueueUploaderPrivate.h"
#include "DefaultUploadEngine.h"
#include "FileQueueUploader.h"
#include <zthread/Runnable.h>
#include "Uploader.h"
#include <Core/Upload/FileUploadTask.h>
#include <algorithm>
#include <zthread/Thread.h>
#include <Gui/Dialogs/LogWindow.h>
#include "UploadEngineManager.h"
#ifndef IU_CLI
#include <zthread/Thread.h>
#include <zthread/Mutex.h>
#endif
class FileQueueUploaderPrivate::Runnable
#ifndef IU_CLI
	: public ZThread::Runnable
#endif

{
public:
	FileQueueUploaderPrivate* uploader_;
	Runnable(FileQueueUploaderPrivate* uploader)
	{
		uploader_ = uploader;
	}

	virtual void run()
	{
		uploader_->run();
	}
};

/* private CFileQueueUploaderPrivate class */

FileQueueUploaderPrivate::FileQueueUploaderPrivate(CFileQueueUploader* queueUploader, UploadEngineManager* uploadEngineManager) {
	m_nThreadCount = 1;
	m_NeedStop = false;
	m_IsRunning = false;
	m_nRunningThreads = 0;
	queueUploader_ = queueUploader;
	startFromSession_ = 0;
	uploadEngineManager_ = uploadEngineManager;
}

FileQueueUploaderPrivate::~FileQueueUploaderPrivate() {
}

bool FileQueueUploaderPrivate::onNeedStopHandler() {
	return m_NeedStop;
}

void FileQueueUploaderPrivate::onProgress(CUploader* uploader, InfoProgress progress) {
	auto task = uploader->currentTask();
	UploadProgress* prog = task->progress();
	prog->totalUpload = progress.Total;
	prog->uploaded = progress.Uploaded;
	prog->isUploading = progress.IsUploading;
	
	if (task->OnUploadProgress) {
		task->OnUploadProgress(task);
	}
}

void FileQueueUploaderPrivate::onErrorMessage(CUploader*, ErrorInfo ei)
{
	DefaultErrorHandling::ErrorMessage(ei);
}

int FileQueueUploaderPrivate::pendingTasksCount()
{
	int res = 0;
	for (size_t i = startFromSession_; i < sessions_.size(); i++)
	{
		res += sessions_[i]->pendingTasksCount();
	}
	return res;
}

void FileQueueUploaderPrivate::OnConfigureNetworkClient(CUploader*, NetworkClient* nm)
{
	/*if (callback_) {
		callback_->OnConfigureNetworkClient(queueUploader_, nm);
	}*/
}

std_tr::shared_ptr<UploadTask> FileQueueUploaderPrivate::getNextJob() {
	if (m_NeedStop)
		return std_tr::shared_ptr<UploadTask>();
#ifndef IU_CLI
	mutex_.acquire();
#endif
	if (!sessions_.empty() && !m_NeedStop)
	{
		for (size_t i = startFromSession_; i < sessions_.size(); i++)
		{
			std_tr::shared_ptr<UploadTask> task = sessions_[i]->getNextTask();
			if (task) {
				task->setRunning(true);
				task->setFinished(false);
				if (task)
				{
					mutex_.release();
					return task;
				}
			}
			startFromSession_ = i + 1;
		}
	}
#ifndef IU_CLI
	mutex_.release();
#endif
	return std_tr::shared_ptr<UploadTask>();
}

void FileQueueUploaderPrivate::AddTask(std_tr::shared_ptr<UploadTask>  task) {
	std::shared_ptr<UploadSession> session(new UploadSession());
	session->addTask(task);
	AddSession(session);
	//serverThreads_[task->serv].waitingFileCount++;
	//AddFile(newTask);
}

void FileQueueUploaderPrivate::AddSession(std::shared_ptr<UploadSession> uploadSession)
{
	sessions_.push_back(uploadSession);
}

void FileQueueUploaderPrivate::addUploadFilter(UploadFilter* filter)
{
	filters_.push_back(filter);
}

void FileQueueUploaderPrivate::removeUploadFilter(UploadFilter* filter)
{
	auto it = std::find(filters_.begin(), filters_.end(), filter);
	if (it != filters_.end())
	{
		filters_.erase(it);
	}
}

void FileQueueUploaderPrivate::start() {
#ifndef IU_CLI
	mutex_.acquire();
#endif
	m_NeedStop = false;
	m_IsRunning = true;
	int numThreads = std::min<int>(size_t(m_nThreadCount - m_nRunningThreads), pendingTasksCount());

	for (int i = 0; i < numThreads; i++)
	{
		m_nRunningThreads++;
#ifdef IU_CLI
		(new Runnable(this))->run();
#else
		ZThread::Thread t1(new Runnable(this));// starting new thread
#endif
	}
#ifndef IU_CLI
	mutex_.release();
#endif
}


void FileQueueUploaderPrivate::run()
{
	CUploader uploader;
	uploader.onConfigureNetworkClient.bind(this, &FileQueueUploaderPrivate::OnConfigureNetworkClient);
#ifndef IU_CLI
	// TODO
	uploader.onErrorMessage.bind(this, &FileQueueUploaderPrivate::onErrorMessage);
#endif
	for (;;)
	{
		auto it = getNextJob();
		if (!it)
			break;
		serverThreads_[it->serverName()].waitingFileCount--;

		std::string serverName = it->serverName();
		serverThreads_[serverName].runningThreads++;
		CAbstractUploadEngine *engine = uploadEngineManager_->getUploadEngine(it->serverProfile());
		uploader.setUploadEngine(engine);
		uploader.onNeedStop.bind(this, &FileQueueUploaderPrivate::onNeedStopHandler);
		uploader.onProgress.bind(this, &FileQueueUploaderPrivate::onProgress);
#ifndef IU_CLI
		mutex_.acquire();
#endif
		//tasks_[&uploader] = &it;
#ifndef IU_CLI
		mutex_.release();
#endif
		bool res = uploader.Upload(it);
		it->setFinished(true);
		it->setRunning(false);
#ifndef IU_CLI
		mutex_.acquire();
#endif
		serverThreads_[serverName].runningThreads--;
#ifndef IU_CLI
		mutex_.release();
#endif

		// m_CS.Lock();
#ifndef IU_CLI
		callMutex_.acquire();
#endif
		UploadResult* result = it->uploadResult();
		result->serverName = serverName;

		if (res) {
			result->directUrl = (uploader.getDirectUrl());
			result->downloadUrl = (uploader.getDownloadUrl());
			result->thumbUrl = (uploader.getThumbUrl());

			if (it->OnFileFinished) {
				it->OnFileFinished(it, true);
			}
		}
		else
		{
			if (it->OnFileFinished) {
				it->OnFileFinished(it, false);
			}
		}
#ifndef IU_CLI
		callMutex_.release();
#endif
	}
#ifndef IU_CLI
	mutex_.acquire();
#endif
	m_nRunningThreads--;
#ifndef IU_CLI
	mutex_.release();
#endif
	if (!m_nRunningThreads)
	{
		m_IsRunning = false;
		if (queueUploader_->OnQueueFinished) {
			queueUploader_->OnQueueFinished(queueUploader_);
		}
	}

}
