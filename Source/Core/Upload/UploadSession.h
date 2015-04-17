#ifndef IU_CORE_UPLOAD_UPLOADSESSION_H
#define IU_CORE_UPLOAD_UPLOADSESSION_H

#pragma once
#include "UploadTask.h"
#include <mutex>

class UploadSession
{
	public:
		UploadSession();
		void addTask(std::shared_ptr<UploadTask> task);
		void removeTask(std::shared_ptr<UploadTask> task);
		std::shared_ptr<UploadTask> getNextTask();
		bool isRunning();
		bool isFinished();
		int pendingTasksCount();
		fastdelegate::FastDelegate1<UploadSession*> OnSessionFinished;
		fastdelegate::FastDelegate2<UploadSession*, UploadTask*> OnTaskAdded;
		friend class UploadTask;
	protected:
		std::vector<std::shared_ptr<UploadTask>> tasks_;
		bool isFinished_;
		void taskFinished(UploadTask* task);
		std::mutex tasksMutex_;
};

#endif
