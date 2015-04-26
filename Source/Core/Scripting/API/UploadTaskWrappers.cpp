#include "UploadTaskWrappers.h"
#include "Core/Scripting/Squirrelnc.h"
#include <Core/Upload/FileUploadTask.h>
#include <boost/current_function.hpp>
#include <Core/Upload/UrlShorteningTask.h>

namespace ScriptAPI {;

UploadTaskWrapperBase::UploadTaskWrapperBase()
{
}

const std::string UploadTaskWrapperBase::role()
{
	return UploadTask::EnumToString(task_->role());
}

void UploadTaskWrapperBase::setRole(const std::string& role)
{
	task_->setRole(UploadTask::StringToEnum(role.c_str()));
}

const std::string UploadTaskWrapperBase::getMimeType()
{   
    return task_->getMimeType();
}

int64_t UploadTaskWrapperBase::getDataLength()
{
    return task_->getDataLength();
}

UploadTaskWrapper UploadTaskWrapperBase::parentTask()
{
    return UploadTaskWrapper(task_->parentTask());
}

UploadTaskWrapper UploadTaskWrapperBase::child(int index)
{
    return task_->child(index);
}

int UploadTaskWrapperBase::childCount()
{
    return task_->childCount();
}

UploadResult* UploadTaskWrapperBase::uploadResult()
{
    return task_->uploadResult();
}

std::string UploadTaskWrapperBase::serverName() const
{
    return task_->serverName();
}

std::string UploadTaskWrapperBase::toString()
{
    return task_->toString();
}

void UploadTaskWrapperBase::addChildTask(UploadTaskWrapper child)
{
    task_->addChildTask(child.task_);
}

std::string FileUploadTaskWrapper::getFileName() const
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task_.get());
    if (!fileTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a FileUploadTask";
        return std::string();
    }
    return fileTask->getFileName();
}

int64_t FileUploadTaskWrapper::getFileSize() const
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task_.get());
    if (!fileTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a FileUploadTask";
        return 0;
    }
    return fileTask->getFileSize();
}

void FileUploadTaskWrapper::setFileName(const std::string& fileName)
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task_.get());
    if (!fileTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a FileUploadTask";
        return;
    }
    fileTask->setFileName(fileName);
}

std::string FileUploadTaskWrapper::getDisplayName() const
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task_.get());
    if (!fileTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a FileUploadTask";
        return std::string();
    }
    return fileTask->getDisplayName();
}

void FileUploadTaskWrapper::setDisplayName(const std::string& name)
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task_.get());
    if (!fileTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a FileUploadTask";
        return;
    }
    fileTask->setDisplayName(name);
}

std::string FileUploadTaskWrapper::originalFileName() const
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task_.get());
    if (!fileTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a FileUploadTask";
        return std::string();
    }
    return fileTask->originalFileName();
}

std::string UrlShorteningTaskWrapper::getUrl() const
{
    UrlShorteningTask* urlShorteningTask = dynamic_cast<UrlShorteningTask*>(task_.get());
    if (!urlShorteningTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a UrlShorteningTask";
        return std::string();
    }
    return urlShorteningTask->getUrl();
}

void UrlShorteningTaskWrapper::setParentUrlType(const std::string& type)
{
    UrlShorteningTask* urlShorteningTask = dynamic_cast<UrlShorteningTask*>(task_.get());
    if (!urlShorteningTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a UrlShorteningTask";
        return;
    }
    urlShorteningTask->setParentUrlType(UrlShorteningTask::StringToEnum(type.c_str())); 
}

const std::string UrlShorteningTaskWrapper::parentUrlType()
{
    UrlShorteningTask* urlShorteningTask = dynamic_cast<UrlShorteningTask*>(task_.get());
    if (!urlShorteningTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a UrlShorteningTask";
        return std::string();
    }
    return UrlShorteningTask::EnumToString(urlShorteningTask->parentUrlType());
}

UploadTaskWrapper::UploadTaskWrapper()
{
    //task_.reset(new 
}

UploadTaskWrapper::UploadTaskWrapper(UploadTask* task)
{
    release_deleter<UploadTask> deleter;
    deleter.release();
    task_.reset(task, deleter);
}

UploadTaskWrapper::UploadTaskWrapper(std::shared_ptr<UploadTask> task)
{
    task_ = task;
}


void RegisterUploadTaskWrappers(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
    RootTable& root = vm.GetRootTable();
    HSQUIRRELVM hvm = vm.GetVM();
    root.Bind("UploadTaskBase", Class<UploadTaskWrapperBase>(hvm, "UploadTaskBase").
        Func("role", &UploadTaskWrapperBase::role).
        Func("setRole", &UploadTaskWrapperBase::setRole).
        Func("getMimeType", &UploadTaskWrapperBase::getMimeType).
        Func("getDataLength", &UploadTaskWrapperBase::getDataLength).
        Func("parentTask", &UploadTaskWrapperBase::parentTask)
        );

    root.Bind("FileUploadTask", DerivedClass<FileUploadTaskWrapper, UploadTaskWrapperBase>(hvm, "FileUploadTask").
        Func("getFileName", &FileUploadTaskWrapper::getFileName).
        Func("getFileSize", &FileUploadTaskWrapper::getFileSize).
        Func("setFileName", &FileUploadTaskWrapper::setFileName).
        Func("getDisplayName", &FileUploadTaskWrapper::getDisplayName).
        Func("setDisplayName", &FileUploadTaskWrapper::setDisplayName).
        Func("originalFileName", &FileUploadTaskWrapper::originalFileName)
        );

    root.Bind("UrlShorteningTask", DerivedClass<UrlShorteningTaskWrapper, FileUploadTaskWrapper>(hvm, "UrlShorteningTask").
        Func("getUrl", &UrlShorteningTaskWrapper::getUrl).
        Func("setParentUrlType", &UrlShorteningTaskWrapper::setParentUrlType).
        Func("parentUrlType", &UrlShorteningTaskWrapper::parentUrlType)
        );

    DerivedClass<UploadTaskWrapper, UrlShorteningTaskWrapper> UploadTaskWrapperClass(hvm, "UploadTask");
    root.Bind("UploadTask", UploadTaskWrapperClass);
}


}