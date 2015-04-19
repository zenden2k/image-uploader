#ifndef IU_CORE_UPLOAD_UPLOADSESSION_H
#define IU_CORE_UPLOAD_UPLOADSESSION_H

#pragma once
#include "UploadTask.h"
#include <mutex>
#include "Func/HistoryManager.h"

class UploadSession
{
	public:
		UploadSession();
		typedef fastdelegate::FastDelegate2<UploadSession*, UploadTask*> TaskAddedCallback;
		typedef fastdelegate::FastDelegate1<UploadSession*> SessionFinishedCallback;
		void addTask(std::shared_ptr<UploadTask> task);
		void removeTask(std::shared_ptr<UploadTask> task);
		int getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask);
		bool isRunning();
		bool isFinished();
		int pendingTasksCount(UploadTaskAcceptor* acceptor);
		int taskCount();
		int finishedTaskCount();
		std::shared_ptr<UploadTask> getTask(int index);
		void addSessionFinishedCallback(const SessionFinishedCallback& callback);
		void addTaskAddedCallback(const TaskAddedCallback& callback);
		friend class UploadTask;
		friend class UploadManager;
		friend class HistoryUploadFilter;
	protected:
		std::vector<std::shared_ptr<UploadTask>> tasks_;
		bool isFinished_;
		void taskFinished(UploadTask* task);
		void childTaskAdded(UploadTask* task);
		std::mutex tasksMutex_;
		std::vector<TaskAddedCallback> taskAddedCallbacks_;
		std::vector<SessionFinishedCallback> sessionFinishedCallbacks_;
		void notifyTaskAdded(UploadTask* task);
		std_tr::shared_ptr<CHistorySession> historySession_;
		std::mutex historySessionMutex_;
};

#endif
