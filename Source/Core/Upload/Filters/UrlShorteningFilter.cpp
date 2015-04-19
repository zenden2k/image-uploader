#include "UrlShorteningFilter.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Upload/FileUploadTask.h"

bool UrlShorteningFilter::PreUpload(UploadTask* task)
{
	return true;
}

bool UrlShorteningFilter::PostUpload(UploadTask* task)
{
	if (task->parentTask() || !task->uploadSuccess(false) || task->shorteningStarted())
	{
		return false;
	}
	if (!task->serverProfile().shortenLinks())
	{
		return false;
	}
	ServerProfile & server = task->urlShorteningServer();
	if (server.isNull())
	{
		LOG(ERROR) << "Shortening server not set";
		return false;
	}

	FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
	std::string directUrl = fileTask->uploadResult()->directUrl;
	std::string downloadUrl = fileTask->uploadResult()->downloadUrl;
	bool res = false;
	if (!directUrl.empty()) {
		std::shared_ptr<UrlShorteningTask> shorteningTask(new UrlShorteningTask(directUrl, task));
		shorteningTask->setServerProfile(server);
		shorteningTask->setRole(UploadTask::UrlShorteningRole);
		shorteningTask->setParentUrlType(UrlShorteningTask::DirectUrl);
		task->setShorteningStarted(true);
		fileTask->addChildTask(shorteningTask);
		res = true;
	}
	return res;
}
