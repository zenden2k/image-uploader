#ifndef IU_CORE_UPLOAD_UPLOADSESSION_H
#define IU_CORE_UPLOAD_UPLOADSESSION_H

#pragma once
#include "UploadTask.h"

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
		fastdelegate::FastDelegate1<std::shared_ptr<UploadSession>> OnSessionFinished;
	protected:
		std::vector<std::shared_ptr<UploadTask>> tasks_;
		bool isFinished_;
};

#endif
