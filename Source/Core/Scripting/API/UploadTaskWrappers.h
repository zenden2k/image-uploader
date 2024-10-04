#ifndef IU_CORE_UPLOADTTASKWRAPPERS_H
#define IU_CORE_UPLOADTTASKWRAPPERS_H

#pragma once
#include <memory>
#include "Core/Upload/UploadTask.h"
#include "Core/Scripting/Squirrelnc.h"

class FileUploadTask;
class UrlShorteningTask;

namespace ScriptAPI {;

class UploadTaskWrapper;

/**
* Upload task base class
*/
class UploadTaskWrapper
{
public:
    UploadTaskWrapper();
    explicit UploadTaskWrapper(UploadTask* task);
    UploadTaskWrapper(std::shared_ptr<UploadTask> task);
    virtual ~UploadTaskWrapper() = default;
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
    void addChildTask(UploadTaskWrapper* child);
    void addTempFile(const std::string& fileName);
    bool isNull();
protected:
    std::shared_ptr<UploadTask> task_;
    void checkNull(const char* func) const;
};

/**
* File upload task
*/
class FileUploadTaskWrapper : public UploadTaskWrapper  {
public:
    explicit FileUploadTaskWrapper(FileUploadTask* task);
    FileUploadTaskWrapper(std::shared_ptr<FileUploadTask> task);
    explicit FileUploadTaskWrapper(const std::string& fileName, const std::string& displayName);
    std::string getFileName() const;
    int64_t getFileSize() const;
    void setFileName(const std::string& fileName);
    std::string getDisplayName() const;
    void setDisplayName(const std::string& name);
    std::string originalFileName() const;
    bool isImage() const;
    bool isVideo() const;
};

/**
* Url shortening task
*/
class UrlShorteningTaskWrapper : public UploadTaskWrapper {
public:
    explicit UrlShorteningTaskWrapper(UrlShorteningTask* task);
    UrlShorteningTaskWrapper(std::shared_ptr<UrlShorteningTask> task);
    std::string getUrl() const;
    /**
     * Possible values: DirectUrl, DownloadUrl
     */
    void setParentUrlType(const std::string& type);
    const std::string parentUrlType();
};

class UploadTaskUnion {
public:
    UploadTaskUnion() = default;
    UploadTaskUnion(UploadTask* task);
    UploadTaskUnion(std::shared_ptr<UploadTask> task);
    std::string type() const;
    UploadTaskWrapper getTask();
    FileUploadTaskWrapper getFileTask();
    UrlShorteningTaskWrapper getUrlShorteningTask();
private:
    std::shared_ptr<UploadTask> task_;
};

/* @cond PRIVATE */
void RegisterUploadTaskWrappers(Sqrat::SqratVM& vm);
/* @endcond */

}

#endif

