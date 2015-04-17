#include "UploadTask.h"

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

bool UploadTask::isFinished()
{
	return isFinished_;
}

void UploadTask::setFinished(bool finished)
{
	isFinished_ = finished;
}

void UploadTask::setRunning(bool running)
{
	isRunning_ = running;
}

void UploadTask::addChildTask(std::shared_ptr<UploadTask> child)
{
	childTasks_.push_back(child);
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
