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
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	auto it = std::find(tasks_.begin(), tasks_.end(), task);
	if (it != tasks_.end() )
		tasks_.erase(it);
}

int  UploadSession::getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask)
{
	int count = 0;
	//LOG(ERROR) << "UploadSession::getNextTask()";
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		UploadTask* uploadTask = it->get();
		if (!uploadTask->isFinishedItself() && !uploadTask->isRunningItself() &&  !uploadTask->isStopped()) {
			count++;
			if (acceptor->canAcceptUploadTask(uploadTask))
			{
				outTask = *it;
				it->get()->setStatus(UploadTask::StatusRunning);
				return count;
			}
			
		}
		std::shared_ptr<UploadTask> task;
		int childCount = it->get()->getNextTask(acceptor, task);
		count += childCount;
		if (task)
		{
			task->setStatus(UploadTask::StatusRunning);
			outTask = task;
			return count;
		}
	}
	return count;
}

bool UploadSession::isRunning()
{
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
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
	try {
		std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
		if (!tasks_.size())
		{
			return false;
		}
		for (int i = 0; i < tasks_.size(); i++)
		{
			if (!tasks_[i]->isFinished())
			{
				return false;
			}
		}
		return true;
	}
	catch (std::exception& ex)
	{
		LOG(ERROR) << ex.what();
	}
	return false;
	//return isFinished_;
}

int UploadSession::pendingTasksCount(UploadTaskAcceptor* acceptor)
{
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
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
	try {
		std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
		return tasks_.size();
	} catch (std::exception& ex)
	{
		LOG(ERROR) << ex.what();
	}
	return tasks_.size();
}

int UploadSession::finishedTaskCount(UploadTask::Status status)
{
	int res = 0;
	try {
		std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	
		for (auto it = tasks_.begin(); it != tasks_.end(); it++)
		{
            if (it->get()->isFinished() && it->get()->status() == status)
			{
				res++;
			}
		}
	}
	catch (std::exception& ex) {
		LOG(ERROR) << ex.what();
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
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	int res = 0;
	for (auto it = tasks_.begin(); it != tasks_.end(); it++)
	{
		it->get()->stop();

	}
}

bool UploadSession::isFatalErrorSet(const std::string& serverName, const std::string& profileName)
{
    std::lock_guard<std::mutex> lock(serverFatalErrorsMutex_);
    auto it = serverFatalErrors_.find(std::make_pair(serverName, profileName));
    if (it != serverFatalErrors_.end())
    {
        return it->second;
    }
    return false;
}

void UploadSession::setFatalErrorForServer(const std::string& serverName, const std::string& profileName)
{
    std::lock_guard<std::mutex> lock(serverFatalErrorsMutex_);
    serverFatalErrors_[std::make_pair(serverName, profileName)] = true;
}

void UploadSession::clearErrorsForServer(const std::string& serverName, const std::string& profileName)
{
    std::lock_guard<std::mutex> lock(serverFatalErrorsMutex_);
    auto it = serverFatalErrors_.find(std::make_pair(serverName, profileName));
    if (it != serverFatalErrors_.end())
    {
        serverFatalErrors_.erase(it);
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