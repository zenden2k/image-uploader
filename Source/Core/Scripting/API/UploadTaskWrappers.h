#ifndef IU_CORE_UPLOADTTASKWRAPPERS_H
#define IU_CORE_UPLOADTTASKWRAPPERS_H

#pragma once
#include <memory>
#include "Core/Upload/UploadTask.h"
#include "Core/Scripting/Squirrelnc.h"

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
    const std::string type();
    const std::string getMimeType();
    int64_t getDataLength();
    UploadTaskWrapper parentTask();
    UploadTaskWrapper child(int index);
    int childCount();
    UploadResult* uploadResult();
    std::string serverName() const;
    const std::string profileName();
    void setStatusText(const std::string& status);
    ServerProfile serverProfile();
    void setServerProfile(const ServerProfile& profile);
    std::string toString();
    void addChildTask(UploadTaskWrapperBase* child);
    void addTempFile(const std::string& fileName);
    bool isNull();
protected:
    std::shared_ptr<UploadTask> task_;
    void checkNull(const char* func) const;
};

/**
* File upload task
*/
class FileUploadTaskWrapper : public  UploadTaskWrapperBase  {
public:
    FileUploadTaskWrapper();
    explicit FileUploadTaskWrapper(const std::string& fileName, const std::string& displayName);
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
    /**
     * Possible values: DirectUrl, DownloadUrl
     */
    void setParentUrlType(const std::string& type);
    const std::string parentUrlType();
};

class UploadTaskWrapper : public UrlShorteningTaskWrapper {
public:
    UploadTaskWrapper();
    UploadTaskWrapper(const std::string& type);
    explicit UploadTaskWrapper(UploadTask* task);
    UploadTaskWrapper(std::shared_ptr<UploadTask> task);
};

/* @cond PRIVATE */
void RegisterUploadTaskWrappers(Sqrat::SqratVM& vm);
/* @endcond */

}

#endif

