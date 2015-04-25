#ifndef IU_CORE_UPLOADTTASKWRAPPERS_H
#define IU_CORE_UPLOADTTASKWRAPPERS_H

#pragma once
#include <memory>
#include "Core/Upload/UploadTask.h"
#include "Core/Squirrelnc.h"

namespace ScriptAPI {;

class UploadTaskWrapper;

/**
* Upload task base class
*/
class UploadTaskWrapperBase
{
public:
	UploadTaskWrapperBase();
	const std::string role();
	void setRole(const std::string& role);   
	const std::string getMimeType();
	int64_t getDataLength();
	UploadTaskWrapper parentTask();
	UploadTaskWrapper child(int index);
	int childCount();
	UploadResult* uploadResult();
	std::string serverName() const;
	std::string toString();
	void addChildTask(UploadTaskWrapper child);
protected:
	std::shared_ptr<UploadTask> task_;
};

/**
* File upload task
*/
class FileUploadTaskWrapper : public  UploadTaskWrapperBase  {
public:
	std::string getFileName() const;
	int64_t getFileSize() const;
    void setFileName(const std::string& fileName);
    std::string getDisplayName() const;
    void setDisplayName(const std::string& name);
    std::string originalFileName() const;
};

/**
* Url shortening task
*/
class UrlShorteningTaskWrapper : public  FileUploadTaskWrapper {
public:
    std::string getUrl() const;
    void setParentUrlType(const std::string& type);
    const std::string parentUrlType();
};

class UploadTaskWrapper : public UrlShorteningTaskWrapper {
public:
    UploadTaskWrapper();
    explicit UploadTaskWrapper(UploadTask* task);
    UploadTaskWrapper(std::shared_ptr<UploadTask> task);
};

void RegisterUploadTaskWrappers(Sqrat::SqratVM& vm);

}

#endif

