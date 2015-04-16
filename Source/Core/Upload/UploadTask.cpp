#include "UploadTask.h"

UploadTask::UploadTask()
{
	parentTask_ = 0;
	isRunning_ = false;
	isFinished_ = false;
}

UploadTask::UploadTask(UploadTask* parentTask)
{
	parentTask_ = parentTask;
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

UploadResult* UploadTask::uploadResult()
{
	return &uploadResult_;
}

UploadProgress* UploadTask::progress()
{
	return &progress_;
}