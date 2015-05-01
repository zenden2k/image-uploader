#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADERPRIVATE_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADERPRIVATE_H

#pragma once
#include "UploadTask.h"
#include "FileQueueUploader.h"
#include "ServerSync.h"
#include <mutex>
#include <Core/Scripting/ScriptsManager.h>

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
    TaskAcceptorBase(bool useMutex = true);
    bool canAcceptUploadTask(UploadTask* task) override;
    std::map<std::string, ServerThreadsInfo> serverThreads_;
    std::recursive_mutex serverThreadsMutex_;
    int fileCount;
    bool useMutex_;

};
class FileQueueUploaderPrivate : public  TaskAcceptorBase {
public:
    FileQueueUploaderPrivate(CFileQueueUploader* queueUploader, UploadEngineManager* uploadEngineManager, ScriptsManager* scriptsManager);
    virtual ~FileQueueUploaderPrivate();
    void start();
    virtual void run();
    std_tr::shared_ptr<UploadTask> getNextJob();
    void AddTask(std_tr::shared_ptr<UploadTask> task);
    void AddSession(std::shared_ptr<UploadSession> uploadSession);
    void removeSession(std::shared_ptr<UploadSession> uploadSession);
    void addUploadFilter(UploadFilter* filter);
    void removeUploadFilter(UploadFilter* filter);
    int sessionCount();
    std_tr::shared_ptr<UploadSession> session(int index);
    CFileQueueUploader *queueUploader_;
    volatile bool m_NeedStop;
    bool m_IsRunning;
    std::vector<std::shared_ptr<UploadSession>> sessions_;
    std::recursive_mutex sessionsMutex_;
    std::vector<UploadFilter*> filters_;
    int m_nThreadCount;
    int m_nRunningThreads;
    friend class CFileQueueUploader;
protected:
    bool onNeedStopHandler();
    void OnConfigureNetworkClient(CUploader*, NetworkClient* nm);
    void onErrorMessage(CUploader*, ErrorInfo);
    void onDebugMessage(CUploader*, const std::string& msg, bool isResponseBody);
    void onTaskAdded(UploadSession*, UploadTask*);
    std::recursive_mutex mutex_;
    std::recursive_mutex callMutex_;
    

    int pendingTasksCount();
    void taskAdded(UploadTask* task);
    //std::map<CUploader*, CFileQueueUploader::Task*> tasks_;

    bool autoStart_;
    int startFromSession_;
    UploadEngineManager* uploadEngineManager_;
    ScriptsManager* scriptsManager_;
};

#endif