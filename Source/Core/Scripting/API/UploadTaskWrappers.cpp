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
    checkNull(BOOST_CURRENT_FUNCTION);
	return UploadTask::EnumToString(task_->role());
}

void UploadTaskWrapperBase::setRole(const std::string& role)
{
    checkNull(BOOST_CURRENT_FUNCTION);
	task_->setRole(UploadTask::StringToEnumRole(role.c_str()));
}

const std::string UploadTaskWrapperBase::type()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return UploadTask::EnumToString(task_->type());
}


const std::string UploadTaskWrapperBase::getMimeType()
{   
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->getMimeType();
}

int64_t UploadTaskWrapperBase::getDataLength()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->getDataLength();
}

UploadTaskWrapper  UploadTaskWrapperBase::parentTask()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return UploadTaskWrapper(task_->parentTask());
}

UploadTaskWrapper UploadTaskWrapperBase::child(int index)
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->child(index);
}

int UploadTaskWrapperBase::childCount()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->childCount();
}

UploadResult* UploadTaskWrapperBase::uploadResult()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->uploadResult();
}

std::string UploadTaskWrapperBase::serverName() const
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->serverName();
}

const std::string UploadTaskWrapperBase::profileName()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->serverProfile().profileName();
}

void UploadTaskWrapperBase::setStatusText(const std::string& status)
{
    checkNull(BOOST_CURRENT_FUNCTION);
    task_->setStatusText(status);
}

ServerProfile UploadTaskWrapperBase::serverProfile()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->serverProfile();
}

void UploadTaskWrapperBase::setServerProfile(const ServerProfile& profile)
{
    checkNull(BOOST_CURRENT_FUNCTION);
    task_->setServerProfile(profile);
}

std::string UploadTaskWrapperBase::toString()
{
    checkNull(BOOST_CURRENT_FUNCTION);
    return task_->toString();
}

void UploadTaskWrapperBase::addChildTask(UploadTaskWrapperBase* child)
{
    checkNull(BOOST_CURRENT_FUNCTION);
    task_->addChildTask(child->task_);
}

void UploadTaskWrapperBase::addTempFile(const std::string& fileName)
{
    checkNull(BOOST_CURRENT_FUNCTION);
    task_->addTempFile(fileName);
}

bool UploadTaskWrapperBase::isNull()
{
    return !task_;
}

void UploadTaskWrapperBase::checkNull(const char* func) const
{
    if (!task_)
    {
        throw std::runtime_error(std::string(func) + "\r\nUploadTask is null");
    }
}

FileUploadTaskWrapper::FileUploadTaskWrapper()
{

}

FileUploadTaskWrapper::FileUploadTaskWrapper(const std::string& fileName, const std::string& displayName)
{
    task_.reset(new FileUploadTask(fileName, displayName));
}

std::string FileUploadTaskWrapper::getFileName() const
{
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
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
    checkNull(BOOST_CURRENT_FUNCTION);
    UrlShorteningTask* urlShorteningTask = dynamic_cast<UrlShorteningTask*>(task_.get());
    if (!urlShorteningTask)
    {
        LOG(ERROR) << BOOST_CURRENT_FUNCTION << std::endl << "this is not a UrlShorteningTask";
        return;
    }
    urlShorteningTask->setParentUrlType(UrlShorteningTask::StringToEnumParentUrlType(type.c_str())); 
}

const std::string UrlShorteningTaskWrapper::parentUrlType()
{
    checkNull(BOOST_CURRENT_FUNCTION);
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

UploadTaskWrapper::UploadTaskWrapper(const std::string& type)
{
    /*UploadTask::Type type = UploadTask::StringToEnumType(type);
    UploadTask* task = 0;
    switch (type)
    {
    case UploadTask::TypeUrl:
        task = new UrlShorteningTask()
    }*/
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
    Class<ServerProfile> serverProfileClass(hvm, "ServerProfile");
    root.Bind("ServerProfile", serverProfileClass);
    root.Bind("UploadTaskBase", Class<UploadTaskWrapperBase>(hvm, "UploadTaskBase").
        Func("role", &UploadTaskWrapperBase::role).
        Func("type", &UploadTaskWrapperBase::type).
        Func("setRole", &UploadTaskWrapperBase::setRole).
        Func("getMimeType", &UploadTaskWrapperBase::getMimeType).
        Func("getDataLength", &UploadTaskWrapperBase::getDataLength).
        Func("parentTask", &UploadTaskWrapperBase::parentTask).
        Func("addTempFile", &UploadTaskWrapperBase::addTempFile).
        Func("addChildTask", &UploadTaskWrapperBase::addChildTask).
        Func("serverName", &UploadTaskWrapperBase::serverName).
        Func("profileName", &UploadTaskWrapperBase::profileName).
        Func("serverProfile", &UploadTaskWrapperBase::serverProfile).
        Func("isNull", &UploadTaskWrapperBase::isNull).
        Func("setStatusText", &UploadTaskWrapperBase::setStatusText)
        );

    root.Bind("FileUploadTask", DerivedClass<FileUploadTaskWrapper, UploadTaskWrapperBase>(hvm, "FileUploadTask").
        Ctor().
        Ctor<const std::string&, const std::string&>().
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