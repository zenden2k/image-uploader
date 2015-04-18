#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADERPRIVATE_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADERPRIVATE_H

#pragma once
#include "UploadTask.h"
#include "FileQueueUploader.h"
#include <mutex>

class CUploader;
class CAbstractUploadEngine;
class CFileQueueUploader;

struct ServerThreadsInfo {
	//int maxThreads;
	int runningThreads;
	int waitingFileCount;
	CUploadEngineData* ued;
};

class TaskAcceptorBase : public UploadTaskAcceptor
{
public:
	TaskAcceptorBase(bool useMutex = true);
	bool canAcceptUploadTask(UploadTask* task) override;
	std::map<std::string, ServerThreadsInfo> serverThreads_;
	std::mutex serverThreadsMutex_;
	int fileCount;
	bool useMutex_;

};
class FileQueueUploaderPrivate : public  TaskAcceptorBase {
public:
	FileQueueUploaderPrivate(CFileQueueUploader* queueUploader, UploadEngineManager* uploadEngineManager);
	virtual ~FileQueueUploaderPrivate();
	void start();
	virtual void run();
	std_tr::shared_ptr<UploadTask> getNextJob();
	void AddTask(std_tr::shared_ptr<UploadTask> task);
	void AddSession(std::shared_ptr<UploadSession> uploadSession);
	void addUploadFilter(UploadFilter* filter);
	void removeUploadFilter(UploadFilter* filter);
#ifndef IU_CLI
	std::mutex mutex_;
	std::mutex callMutex_;
#endif
	CFileQueueUploader *queueUploader_;
	volatile bool m_NeedStop;
	bool m_IsRunning;
	std::vector<std::shared_ptr<UploadSession>> sessions_;
	std::mutex sessionsMutex_;
	std::vector<UploadFilter*> filters_;
	int m_nThreadCount;
	int m_nRunningThreads;
	friend class CFileQueueUploader;
protected:
	class Runnable;
	bool onNeedStopHandler();
	void OnConfigureNetworkClient(CUploader*, NetworkClient* nm);
	void onProgress(CUploader*, InfoProgress progress);
	void onErrorMessage(CUploader*, ErrorInfo);
	void onTaskAdded(UploadSession*, UploadTask*);

	int pendingTasksCount();
	//std::map<CUploader*, CFileQueueUploader::Task*> tasks_;

	bool autoStart_;
	int startFromSession_;
	UploadEngineManager* uploadEngineManager_;
};

#endif