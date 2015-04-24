#include "UploadTask.h"
#include "UploadSession.h"
#include "Core/Upload/ScriptUploadEngine.h"

UploadTask::UploadTask()
{
	parentTask_ = 0;
	init();
}

void UploadTask::init()
{
	status_ = StatusInQueue;
	userData_ = NULL;
	uploadSuccess_ = false;
	session_ = 0;
	role_ = DefaultRole;
	shorteningStarted_ = false;
	stopSignal_ = false;
	currentUploadEngine_ = 0;
}

void UploadTask::childTaskFinished(UploadTask* child)
{
	if (isFinished())
	{
		taskFinished();
	}
}

void UploadTask::taskFinished()
{
	for (int i = 0; i < taskFinishedCallbacks_.size(); i++)
	{
		taskFinishedCallbacks_[i](this, uploadSuccess()); // invoke callback
	}
	if (session_)
	{
		session_->taskFinished(this);
	}
}

void UploadTask::setCurrentUploadEngine(CAbstractUploadEngine* currentUploadEngine)
{
	currentUploadEngine_ = currentUploadEngine;
}

bool UploadTask::stopSignal() const
{
	return stopSignal_;
}

UploadTask::UploadTask(UploadTask* parentTask)
{
	parentTask_ = parentTask;
	init();
}
UploadTask::~UploadTask() {

}

UploadTask* UploadTask::parentTask() const
{
	return parentTask_;
}

std::shared_ptr<UploadTask> UploadTask::child(int index)
{
	std::lock_guard<std::recursive_mutex> guard(tasksMutex_);
	return childTasks_[index];
}

bool UploadTask::isRunning()
{
	std::lock_guard<std::recursive_mutex> guard(tasksMutex_);
	for ( auto& it : childTasks_ )
	{
		if (it->isRunning())
		{
			return true;
		}
	}
	return status_ == StatusRunning;
}

bool UploadTask::isRunningItself()
{
	return status_ == StatusRunning;
}

void UploadTask::setSession(UploadSession* session)
{
	session_ = session;
}

UploadSession* UploadTask::session()
{
	return session_;
}

bool UploadTask::isFinished()
{
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	for (int i = 0; i < childTasks_.size(); i++)
	{
		if (!childTasks_[i]->isFinished())
		{
			return false;
		}
	}
	return isFinishedItself();
}

bool UploadTask::isFinishedItself()
{
	return status_ == StatusFinished || status_ == StatusFailure || status_ ==  StatusStopped;
}

void UploadTask::finishTask(Status status)
{
	status_ = status;
	if (parentTask_)
	{
		parentTask_->childTaskFinished(this);
	}

	if (isFinished())
	{
		taskFinished();
	}
}

void UploadTask::addChildTask(std::shared_ptr<UploadTask> child)
{
	tasksMutex_.lock();
	childTasks_.push_back(child);
	tasksMutex_.unlock();
	if (session_) {
		session_->childTaskAdded(child.get());
	}
	if (OnChildTaskAdded)
	{
		OnChildTaskAdded(child.get());
	}
}

int UploadTask::childCount()
{
	std::lock_guard<std::recursive_mutex> guard(tasksMutex_);
	return childTasks_.size();
}

UploadResult* UploadTask::uploadResult()
{
	return &uploadResult_;
}

UploadProgress* UploadTask::progress()
{
	return &progress_;
}

void UploadTask::addTaskFinishedCallback(const TaskFinishedCallback& callback)
{
	taskFinishedCallbacks_.push_back(callback);
}

std::string UploadTask::serverName() const
{
	return serverProfile_.serverName();
}

ServerProfile& UploadTask::serverProfile()
{
	return serverProfile_;
}

void UploadTask::setServerProfile(ServerProfile profile)
{
	serverProfile_ = profile;
}

ServerProfile& UploadTask::urlShorteningServer()
{
	return urlShorteningProfile_;
}

void UploadTask::setUrlShorteningServer(ServerProfile profile)
{
	urlShorteningProfile_ = profile;
}

void UploadTask::setUserData(void* data)
{
	userData_ = data;
}

void* UploadTask::userData() const
{
	return userData_;
}

bool UploadTask::uploadSuccess(bool withChilds)
{
	int count = childTasks_.size();
	if (!count || !withChilds )
	{
		return uploadSuccess_;
	}
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	for (int i = 0; i <count; i++)
	{
		if (!childTasks_[i]->isFinished() || !childTasks_[i]->uploadSuccess())
		{
			return false;
		}
	}
	return uploadSuccess_;
}

void UploadTask::setUploadSuccess(bool success)
{
	uploadSuccess_ = success;
}

UploadTask::Role UploadTask::role() const
{
	return role_;
}

void UploadTask::setRole(Role role)
{
	role_ = role;
}

bool UploadTask::shorteningStarted() const
{
	return shorteningStarted_;
}


void UploadTask::setShorteningStarted(bool started)
{
	shorteningStarted_ = started;
}

void UploadTask::stop()
{
	stopSignal_ = true;
	if (currentUploadEngine_)
	{
		currentUploadEngine_->stop();
	}
	if ( status_ != StatusRunning )
	{
		finishTask(StatusStopped);
	}
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	for (auto& it : childTasks_)
	{
		it->stop();
	}
}

bool UploadTask::isStopped()
{
	return status_ == StatusStopped;
}


void UploadTask::setStatus(Status status)
{
	status_ = status;
}

UploadTask::Status UploadTask::status() const
{
	return status_;
}

void UploadTask::uploadProgress(InfoProgress progress)
{
	progress_.uploaded = progress.Uploaded;
	progress_.totalUpload = progress.Total;
	progress_.isUploading = progress.IsUploading;

	struct timeval tp;
	gettimeofday(&tp, NULL);
	int64_t curTime = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	if (curTime - progress_.lastUpdateTime > 250 || progress.Uploaded == progress.Total) {
		int64_t Current = progress.Uploaded;

		int speed = 0;
		UploadProgressTimeInfo ti;
		ti.bytes = Current;
		ti.ms = curTime;
		if (progress_.timeBytes.size())
		{
			speed = int(((double)(ti.bytes - progress_.timeBytes[0].bytes) / (curTime - progress_.timeBytes[0].ms) * 1000));
		}

		progress_.timeBytes.push_back(ti);
		if (progress_.timeBytes.size() > 11)
		{
			progress_.timeBytes.pop_front(); //Deleting element at the beginning of the deque
		}
		if (speed > 0)
		{
			progress_.speed = IuCoreUtils::fileSizeToString(speed) + "/s";
		}
		else
		{
			progress_.speed.clear();
		}
		progress_.lastUpdateTime = curTime;
		if (OnUploadProgress)
		{
			OnUploadProgress(this); // invoke upload progress callback
		}
	}
}

int UploadTask::getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask)
{
	int taskCount = childTasks_.size();
	if (!taskCount)
	{
		return 0;
	}
	int count = 0;
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	for (auto it = childTasks_.begin(); it != childTasks_.end(); it++)
	{
		if (!it->get()->isFinished() && !it->get()->isRunning() && !it->get()->isStopped() )
		{
			count++;
			if (acceptor->canAcceptUploadTask(it->get()))
			{
				outTask = *it;
				return count;
			}
		}
	}
	return count;
}

int UploadTask::pendingTasksCount(UploadTaskAcceptor* acceptor)
{
	int taskCount = childTasks_.size();
	if (!taskCount)
	{
		return 0;
	}
	std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
	int res = 0;
	for (auto it = childTasks_.begin(); it != childTasks_.end(); it++)
	{
		UploadTask* task = it->get();
		if (!task->isFinished() && !task->isRunning() && acceptor->canAcceptUploadTask(task))
		{
			res++;
		}
	}
	return res;
}

/*void setCurrentUploadEngine(CAbstractUploadEngine* currentUploadEngine);*/