#include "UploadTask.h"
#include "UploadSession.h"

UploadTask::UploadTask()
{
	parentTask_ = 0;
	init();
}

void UploadTask::init()
{
	isRunning_ = false;
	isFinished_ = false;
	userData_ = NULL;
	uploadSuccess_ = false;
	session_ = 0;
	role_ = DefaultRole;
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
	if (OnFileFinished) 
	{
		OnFileFinished(this, uploadSuccess()); // invoke callback
	}
	if (session_)
	{
		session_->taskFinished(this);
	}
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

bool UploadTask::isRunning()
{
	return isRunning_;
}

bool UploadTask::isRunningItself()
{
	return isRunning_;
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
	std::lock_guard<std::mutex> lock(tasksMutex_);
	for (int i = 0; i < childTasks_.size(); i++)
	{
		if (!childTasks_[i]->isFinished())
		{
			return false;
		}
	}
	return isFinished_;
}

bool UploadTask::isFinishedItself()
{
	return isFinished_;
}

void UploadTask::setFinished(bool finished)
{
	isFinished_ = finished;
	if (finished && parentTask_)
	{
		parentTask_->childTaskFinished(this);
	}

	if (isFinished_ && isFinished())
	{
		taskFinished();
	}
}

void UploadTask::setRunning(bool running)
{
	isRunning_ = running;
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

UploadResult* UploadTask::uploadResult()
{
	return &uploadResult_;
}

UploadProgress* UploadTask::progress()
{
	return &progress_;
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

void UploadTask::setUserData(void* data)
{
	userData_ = data;
}

void* UploadTask::userData() const
{
	return userData_;
}

bool UploadTask::uploadSuccess()
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	for (int i = 0; i < childTasks_.size(); i++)
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

void UploadTask::uploadProgress(InfoProgress progress)
{
	progress_.uploaded = progress.Uploaded;
	progress_.totalUpload = progress.Total;
	progress_.isUploading = progress.IsUploading;
	if (OnUploadProgress)
	{
		OnUploadProgress(this); // invoke upload progress callback
	}
}

int UploadTask::getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask)
{
	std::lock_guard<std::mutex> lock(tasksMutex_);
	int count = 0;
	for (auto it = childTasks_.begin(); it != childTasks_.end(); it++)
	{
		if (!it->get()->isFinished() && !it->get()->isRunning() )
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
	std::lock_guard<std::mutex> lock(tasksMutex_);
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