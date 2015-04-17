#include "UploadSession.h"

UploadSession::UploadSession()
{
	isFinished_ = false;
}

void UploadSession::addTask(std::shared_ptr<UploadTask> task)
{
	tasksMutex_.lock();
	task->setSession(this);
	tasks_.push_back(task);
	tasksMutex_.unlock();
	if (OnTaskAdded)
	{
		OnTaskAdded(this, task.get());
	}
}

void UploadSession::removeTask(std::shared_ptr<UploadTask> task)
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	auto it = std::find(tasks_.begin(), tasks_.end(), task);
	if (it != tasks_.end() )
		tasks_.erase(it);
}

std::shared_ptr<UploadTask> UploadSession::getNextTask()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		if (!it->get()->isFinished() && !it->get()->isRunning())
		{
			return *it;
		}
	}
	return std::shared_ptr<UploadTask>(0);
}

bool UploadSession::isRunning()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		if (!it->get()->isRunning())
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

int UploadSession::pendingTasksCount()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	int res = 0;
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		if (!it->get()->isFinished() && !it->get()->isRunning())
		{
			res++;
		}
	}
	return res;
}

void UploadSession::taskFinished(UploadTask* task)
{
	if (isFinished() && OnSessionFinished)
	{
		OnSessionFinished(this);
	}
}