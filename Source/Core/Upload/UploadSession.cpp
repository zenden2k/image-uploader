#include "UploadSession.h"

UploadSession::UploadSession()
{
	isFinished_ = false;
	stopSignal_ = true;
}

void UploadSession::addTask(std::shared_ptr<UploadTask> task)
{
	tasksMutex_.lock();
	task->setSession(this);
	tasks_.push_back(task);
	tasksMutex_.unlock();
	notifyTaskAdded(task.get());
}

void UploadSession::removeTask(std::shared_ptr<UploadTask> task)
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	auto it = std::find(tasks_.begin(), tasks_.end(), task);
	if (it != tasks_.end() )
		tasks_.erase(it);
}

int  UploadSession::getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask)
{
	int count = 0;
	//LOG(ERROR) << "UploadSession::getNextTask()";
	std::lock_guard<std::mutex> lock(tasksMutex_);
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		UploadTask* uploadTask = it->get();
		if (!uploadTask->isFinishedItself() && !uploadTask->isRunningItself() &&  !uploadTask->isStopped()) {
			count++;
			if (acceptor->canAcceptUploadTask(uploadTask))
			{
				outTask = *it;
				it->get()->setRunning(true);
				return count;
			}
			
		}
		std::shared_ptr<UploadTask> task;
		int childCount = it->get()->getNextTask(acceptor, task);
		count += childCount;
		if (task)
		{
			task->setRunning(true);
			outTask = task;
			return count;
		}
	}
	return count;
}

bool UploadSession::isRunning()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
	{
		if (it->get()->isRunning())
		{
			return true;
		}
	}
	return false;
}

bool UploadSession::isFinished()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	if (!tasks_.size())
	{
		return false;
	}
	for (int i = 0; i < tasks_.size(); i ++ )
	{
		if (!tasks_[i]->isFinished())
		{
			return false;
		}
	}
	return true;
	//return isFinished_;
}

int UploadSession::pendingTasksCount(UploadTaskAcceptor* acceptor)
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	int res = 0;
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		UploadTask* uploadTask = it->get();
		if (!uploadTask->isFinishedItself() && !uploadTask->isRunningItself() && acceptor->canAcceptUploadTask(uploadTask))
		{
			res++;
		}
		res += it->get()->pendingTasksCount(acceptor);
	}
	return res;
}

int UploadSession::taskCount()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	return tasks_.size();
}

int UploadSession::finishedTaskCount()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	int res = 0;
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		if (it->get()->isFinished())
		{
			res++;
		}
	}
	return res;
}

bool UploadSession::isStopped()
{
	return isStopped_;
}

void UploadSession::addSessionFinishedCallback(const SessionFinishedCallback& callback)
{
	sessionFinishedCallbacks_.push_back(callback);
}

void UploadSession::addTaskAddedCallback(const TaskAddedCallback& callback)
{
	taskAddedCallbacks_.push_back(callback);
}

void UploadSession::stop()
{
	stopSignal_ = true;
	std::lock_guard<std::mutex> lock(tasksMutex_);
	int res = 0;
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		it->get()->stop();

	}
}

std::shared_ptr<UploadTask> UploadSession::getTask(int index)
{
	return tasks_[index];
}

void UploadSession::taskFinished(UploadTask* task)
{
	if (isFinished())
	{
		for (int i = 0; i < sessionFinishedCallbacks_.size(); i++)
		{
			sessionFinishedCallbacks_[i](this);
		}
	}
}

void UploadSession::childTaskAdded(UploadTask* task)
{
	notifyTaskAdded(task);
}

bool UploadSession::stopSignal()
{
	return stopSignal_;
}

void UploadSession::notifyTaskAdded(UploadTask* task)
{
	for (int i = 0; i < taskAddedCallbacks_.size(); i++)
	{
		taskAddedCallbacks_[i](this, task);
	}
}