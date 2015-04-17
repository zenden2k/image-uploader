#include "UploadSession.h"

UploadSession::UploadSession()
{
	isFinished_ = false;
}

void UploadSession::addTask(std::shared_ptr<UploadTask> task)
{
	tasks_.push_back(task);
}

void UploadSession::removeTask(std::shared_ptr<UploadTask> task)
{
	auto it = std::find(tasks_.begin(), tasks_.end(), task);
	if (it != tasks_.end() )
		tasks_.erase(it);
}

std::shared_ptr<UploadTask> UploadSession::getNextTask()
{
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
	return isFinished_;
}

int UploadSession::pendingTasksCount()
{
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