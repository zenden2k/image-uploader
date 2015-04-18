#ifndef IU_CORE_UPLOAD_UPLOADSESSION_H
#define IU_CORE_UPLOAD_UPLOADSESSION_H

#pragma once
#include "UploadTask.h"
#include <mutex>

class UploadSession
{
	public:
		UploadSession();
		typedef fastdelegate::FastDelegate2<UploadSession*, UploadTask*> TaskAddedCallback;
		void addTask(std::shared_ptr<UploadTask> task);
		void removeTask(std::shared_ptr<UploadTask> task);
		int getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask);
		bool isRunning();
		bool isFinished();
		int pendingTasksCount(UploadTaskAcceptor* acceptor);
		int taskCount();
		int finishedTaskCount();
		fastdelegate::FastDelegate1<UploadSession*> OnSessionFinished;
		void addTaskAddedCallback(const TaskAddedCallback& callback);
		friend class UploadTask;
	protected:
		std::vector<std::shared_ptr<UploadTask>> tasks_;
		bool isFinished_;
		void taskFinished(UploadTask* task);
		void childTaskAdded(UploadTask* task);
		std::mutex tasksMutex_;
		std::vector<TaskAddedCallback> taskAddedCallbacks_;
		void notifyTaskAdded(UploadTask* task);
};

#endif
