#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADERPRIVATE_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADERPRIVATE_H

#pragma once
#include <mutex>
#include "UploadTask.h"
#include "FileQueueUploader.h"
#include "ServerSync.h"

#include "Core/Scripting/ScriptsManager.h"
#include "Core/Upload/UploadErrorHandler.h"

class CUploader;
class CAbstractUploadEngine;
class CFileQueueUploader;

struct ServerThreadsInfo {
    //int maxThreads;
    int runningThreads;
    int waitingFileCount;
    CUploadEngineData* ued;
    //bool fatalError;
    ServerThreadsInfo()
    {
        runningThreads = 0;
        waitingFileCount = 0;
        ued = 0;
    }
};

class TaskAcceptorBase : public UploadTaskAcceptor
{
public:
    explicit TaskAcceptorBase(bool useMutex = true);
    bool canAcceptUploadTask(UploadTask* task) override;
    std::map<std::string, ServerThreadsInfo> serverThreads_;
    std::recursive_mutex serverThreadsMutex_;
    int fileCount;
    bool useMutex_;

};
class FileQueueUploaderPrivate : public  TaskAcceptorBase {
public:
    FileQueueUploaderPrivate(CFileQueueUploader* queueUploader, UploadEngineManager* uploadEngineManager, ScriptsManager* scriptsManager,
        IUploadErrorHandler* uploadErrorHandler, std::shared_ptr<INetworkClientFactory> networkClientFactory
        );
    virtual ~FileQueueUploaderPrivate();
    void start();
    virtual void run();
    std::shared_ptr<UploadTask> getNextJob();
    void AddTask(std::shared_ptr<UploadTask> task);
    void AddSession(std::shared_ptr<UploadSession> uploadSession);
    void removeSession(std::shared_ptr<UploadSession> uploadSession);
    void addUploadFilter(UploadFilter* filter);
    void removeUploadFilter(UploadFilter* filter);
    void retrySession(std::shared_ptr<UploadSession> uploadSession);
    int sessionCount();
    std::shared_ptr<UploadSession> session(int index);
    CFileQueueUploader *queueUploader_;
    volatile bool stopSignal_;
    bool isRunning_;
    std::vector<std::shared_ptr<UploadSession>> sessions_;
    std::recursive_mutex sessionsMutex_;
    std::vector<UploadFilter*> filters_;
    int threadCount_;
    int runningThreads_;
    friend class CFileQueueUploader;
protected:
    bool onNeedStopHandler();
    void OnConfigureNetworkClient(CUploader*, INetworkClient* nm);
    void onErrorMessage(CUploader*, ErrorInfo);
    void onDebugMessage(CUploader*, const std::string& msg, bool isResponseBody);
    void onTaskAdded(UploadSession*, UploadTask*);
    std::recursive_mutex mutex_;
    std::recursive_mutex callMutex_;
    std::deque<std::shared_ptr<UploadTask>> queue_;

    int pendingTasksCount();
    void taskAdded(UploadTask* task);
    //std::map<CUploader*, CFileQueueUploader::Task*> tasks_;

    bool autoStart_;
    int startFromSession_;
    UploadEngineManager* uploadEngineManager_;
    ScriptsManager* scriptsManager_; 
    IUploadErrorHandler* uploadErrorHandler_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
};

#endif