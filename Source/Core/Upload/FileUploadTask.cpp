#include "FileUploadTask.h"

#include <Core/Utils/CoreUtils.h>
#include <assert.h>

FileUploadTask::FileUploadTask(const std::string& fileName, const std::string& displayName, UploadTask* parentTask) : UploadTask(parentTask) {
	fileName_ = fileName;
	tempFileDeleter_ = 0;
	originalFileName_ = fileName;
	if ( displayName.empty() ) {
		displayName_ = IuCoreUtils::ExtractFileName(fileName);
	} else {
		displayName_ = displayName;
	}	
}

FileUploadTask::~FileUploadTask()
{
	delete tempFileDeleter_;
}

std::string FileUploadTask::getType() const {
	return "file";
}

std::string FileUploadTask::getMimeType() const {
	return IuCoreUtils::GetFileMimeType(fileName_);
}

int64_t FileUploadTask::getDataLength() const {
	return IuCoreUtils::getFileSize(fileName_);
}

std::string FileUploadTask::getFileName() const {
	return fileName_;
}

void FileUploadTask::setFileName(const std::string& fileName)
{
	fileName_ = fileName;
}

std::string FileUploadTask::getDisplayName() const {
	return displayName_;
}

void FileUploadTask::setDisplayName(const std::string& name)
{
	displayName_ = name;
}

std::string FileUploadTask::originalFileName() const
{
	return originalFileName_;
}

void FileUploadTask::setFinished(bool finished)
{
	if (finished && role_ == ThumbRole)
	{
		assert(parentTask_);
		if (!parentTask_)
		{
			return;
		}
		//TODO: keep server thumbnail url
		parentTask_->uploadResult()->thumbUrl = uploadResult_.directUrl;
	}
	UploadTask::setFinished(finished);
}

TempFileDeleter* FileUploadTask::tempFileDeleter(bool create)
{
	if (!tempFileDeleter_ && create )
	{
		tempFileDeleter_ = new TempFileDeleter();
	}
	return tempFileDeleter_;
}