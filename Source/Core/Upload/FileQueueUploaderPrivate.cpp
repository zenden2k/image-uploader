#include "FileQueueUploaderPrivate.h"

#include <thread>
#include <algorithm>

#include "FileQueueUploader.h"
#include "Uploader.h"
#include "Core/Upload/FileUploadTask.h"
#include "UploadEngineManager.h"
#include "Core/Upload/UploadFilter.h"
#include "Core/CommonDefs.h"
#include "Core/i18n/Translator.h"

TaskAcceptorBase::TaskAcceptorBase(bool useMutex )
{
    fileCount = 0;
    useMutex_ = useMutex;
}

bool TaskAcceptorBase::canAcceptUploadTask(UploadTask* task)
{
    if (useMutex_) {
        serverThreadsMutex_.lock();
    }
    std::string serverName = task->serverProfile().serverName();
    auto it = serverThreads_.find(serverName);
    if (it == serverThreads_.end())
    {
        ServerThreadsInfo sti;
        sti.ued = task->serverProfile().uploadEngineData();
        sti.runningThreads = 1;
        serverThreads_[serverName] = sti;
        fileCount++;
        if (useMutex_) {
            serverThreadsMutex_.unlock();
        }
        return true;
    }
    if (!it->second.ued )
    {
        it->second.ued = task->serverProfile().uploadEngineData(); // FIXME
    }

    UploadSession* session = task->session();
    bool isFatalError = session->isFatalErrorSet(task->serverName(), task->serverProfile().profileName());

    if (!isFatalError && it->second.ued && (!it->second.ued->MaxThreads || it->second.runningThreads < it->second.ued->MaxThreads) )
    {
        it->second.runningThreads++;
        fileCount++;
        if (useMutex_) {
            serverThreadsMutex_.unlock();
        }
        return true;
    }
    if (useMutex_) {
        serverThreadsMutex_.unlock();
    }
    return false;
}

FileQueueUploaderPrivate::FileQueueUploaderPrivate(CFileQueueUploader* queueUploader, UploadEngineManager* uploadEngineManager, 
    ScriptsManager* scriptsManager, IUploadErrorHandler* uploadErrorHandler, std::shared_ptr<INetworkClientFactory> networkClientFactory, int maxThreads) {
    threadCount_ = maxThreads;
    stopSignal_ = false;
    isRunning_ = false;
    queueUploader_ = queueUploader;
    startFromSession_ = 0;
    uploadEngineManager_ = uploadEngineManager;
    scriptsManager_ = scriptsManager;
    uploadErrorHandler_ = uploadErrorHandler;
    //autoStart_ = true;
    networkClientFactory_ = networkClientFactory;
    runningThreadsCount_ = 0;
    start();
}

FileQueueUploaderPrivate::~FileQueueUploaderPrivate() {
    stopSignal_ = true;
    threadCount_ = 0;
    queueCondition_.notify_all();
    for (auto& thread : threads_) {
        thread.join();
    }

}

bool FileQueueUploaderPrivate::onNeedStopHandler() {
    return stopSignal_;
}

void FileQueueUploaderPrivate::onErrorMessage(CUploader*, ErrorInfo ei)
{
    uploadErrorHandler_->ErrorMessage(ei);
}

void FileQueueUploaderPrivate::onDebugMessage(CUploader*, const std::string& msg, bool isResponseBody)
{
    uploadErrorHandler_->DebugMessage(msg, isResponseBody);
}

void FileQueueUploaderPrivate::onTaskAdded(UploadSession*, UploadTask* task)
{
    sessionsMutex_.lock();
    startFromSession_ = 0;
    sessionsMutex_.unlock();
    taskAdded(task);
    //start();
}

int FileQueueUploaderPrivate::pendingTasksCount()
{
    std::lock_guard<std::recursive_mutex> lock2(sessionsMutex_);
    TaskAcceptorBase acceptor(false); // do not use mutex
    serverThreadsMutex_.lock();
    acceptor.serverThreads_ = this->serverThreads_;
    serverThreadsMutex_.unlock();
    for (size_t i = startFromSession_; i < sessions_.size(); i++)
    {
        sessions_[i]->pendingTasksCount(&acceptor);
    }
    return acceptor.fileCount;
}

void FileQueueUploaderPrivate::taskAdded(UploadTask* task)
{
    queueUploader_->taskAdded(task);
}

void FileQueueUploaderPrivate::OnConfigureNetworkClient(CUploader* uploader, INetworkClient* nm)
{
    if (  queueUploader_->OnConfigureNetworkClient )
    {
        queueUploader_->OnConfigureNetworkClient(queueUploader_, nm);
    }
}

std::shared_ptr<UploadTask> FileQueueUploaderPrivate::getNextJob() {
    std::unique_lock<std::mutex> lck(queueMutex_);
    queueCondition_.wait(lck, [&] {return stopSignal_ || !queue_.empty(); });

    if (stopSignal_ && runningThreadsCount_ > threadCount_) {
        return std::shared_ptr<UploadTask>();
    }

    for (auto it = queue_.begin(); it != queue_.end(); ++it) {
        if (canAcceptUploadTask(it->get())) {
            std::shared_ptr<UploadTask> res = *it;
            queue_.erase(it);
            //res->setStatus(UploadTask::StatusPostponed);
            return res;
        }
    }
    //std::lock_guard<std::recursive_mutex> lock(sessionsMutex_);

    /*if (!sessions_.empty() && !stopSignal_)
    {
        for (size_t i = startFromSession_; i < sessions_.size(); i++)
        {
            std::shared_ptr<UploadTask> task;
            if (!sessions_[i]->getNextTask(this, task))
            {
                startFromSession_ = i + 1;
            }
            if (task) {
                task->setStatus(UploadTask::StatusPostponed);

                return task;
            }
            
        }
    }*/
    return std::shared_ptr<UploadTask>();
}

void FileQueueUploaderPrivate::AddTaskToQueue(std::shared_ptr<UploadTask>  task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        task->setUploadManager(queueUploader_);
        queue_.push_back(task);
    }
    taskAdded(task.get());
    queueCondition_.notify_all();
}

void FileQueueUploaderPrivate::insertTaskAfter(UploadTask* after, std::shared_ptr<UploadTask> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        auto it = std::find_if(queue_.begin(), queue_.end(), [](std::shared_ptr<UploadTask> t) { return t->parentTask() == nullptr; });
        if (it == queue_.end()) {
            queue_.push_back(task);
        } else {
            queue_.insert(it, task);
        }  
        //queue_.push_front(task);
    }
    taskAdded(task.get());
    queueCondition_.notify_one();
}

bool FileQueueUploaderPrivate::removeTaskFromQueue(UploadTask* task) {
    std::unique_lock<std::mutex> lock(queueMutex_);

    auto it = std::find_if(queue_.begin(), queue_.end(), [task](const std::shared_ptr<UploadTask>& t)
    {
        return t.get() == task;
    });
    if (it != queue_.end()) {
        queue_.erase(it);
        return true;
    }
    
    return false;
}

void FileQueueUploaderPrivate::AddSingleTask(std::shared_ptr<UploadTask> task) {
    std::shared_ptr<UploadSession> uploadSession(new UploadSession());
    uploadSession->addTask(task);
    AddSession(uploadSession);
}

void FileQueueUploaderPrivate::AddSession(std::shared_ptr<UploadSession> uploadSession)
{
    addSessionToQueue(uploadSession);

    sessionsMutex_.lock();
    sessions_.push_back(uploadSession);
    sessionsMutex_.unlock();
    queueUploader_->sessionAdded(uploadSession.get());
}

void FileQueueUploaderPrivate::addSessionToQueue(std::shared_ptr<UploadSession> uploadSession) {
    //uploadSession->addTaskAddedCallback(UploadSession::TaskAddedCallback(this, &FileQueueUploaderPrivate::onTaskAdded));
    int count = uploadSession->taskCount();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        for (int i = 0; i < count; i++) {
            auto task = uploadSession->getTask(i);
            task->setUploadManager(queueUploader_);
            if (task->status() == UploadTask::StatusInQueue) {
                queue_.push_back(task);
                taskAdded(task.get());
            }
        }
    }
    queueCondition_.notify_all();
}

void FileQueueUploaderPrivate::removeSession(std::shared_ptr<UploadSession> uploadSession)
{
    std::lock_guard<std::recursive_mutex> lock(sessionsMutex_);
    auto it = std::find(sessions_.begin(), sessions_.end(), uploadSession);
    if (it != sessions_.end() ) {
        sessions_.erase(it);
    }
    startFromSession_ = 0;
}

void FileQueueUploaderPrivate::addUploadFilter(UploadFilter* filter)
{
    filters_.push_back(filter);
}

void FileQueueUploaderPrivate::removeUploadFilter(UploadFilter* filter)
{
    auto it = std::find(filters_.begin(), filters_.end(), filter);
    if (it != filters_.end()) {
        filters_.erase(it);
    }
}

void FileQueueUploaderPrivate::retrySession(std::shared_ptr<UploadSession> uploadSession) {
    startFromSession_ = 0;
    uploadSession->clearStopFlag();
    uploadSession->restartFailedTasks(queueUploader_);

    addSessionToQueue(uploadSession);
}

int FileQueueUploaderPrivate::sessionCount()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return sessions_.size();
}

std::shared_ptr<UploadSession> FileQueueUploaderPrivate::session(int index)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return sessions_[index];
}

void FileQueueUploaderPrivate::start() {
    //std::lock_guard<std::recursive_mutex> lock(mutex_);
    stopSignal_ = false;
   
    startThreads(threadCount_);
}

void FileQueueUploaderPrivate::startThreads(int count) {
    for (int i = 0; i < count; i++) {
        ++runningThreadsCount_;
        threads_.emplace_back(&FileQueueUploaderPrivate::run, this);
        //t1.detach();
    }
}

void FileQueueUploaderPrivate::setMaxThreadCount(int threadCount) {
    if (threadCount_ == threadCount) {
        return;
    }
    int oldThreadCount = threadCount_;

    threadCount_ = threadCount;

    if (threadCount > oldThreadCount) {
        startThreads(threadCount - oldThreadCount);
    } else {
        stopSignal_ = true;
        queueCondition_.notify_all();
    }
}
void FileQueueUploaderPrivate::run()
{
    for (;;)
    {
        auto it = getNextJob();

        if (!it) {
            --runningThreadsCount_;
            if (runningThreadsCount_ == threadCount_) {
                stopSignal_ = false;
            }
            break;
        }

        CUploader uploader(networkClientFactory_);
        uploader.onConfigureNetworkClient.bind(this, &FileQueueUploaderPrivate::OnConfigureNetworkClient);

        // TODO
        uploader.onErrorMessage.bind(this, &FileQueueUploaderPrivate::onErrorMessage);
        uploader.onDebugMessage.bind(this, &FileQueueUploaderPrivate::onDebugMessage);
        auto fut = dynamic_cast<FileUploadTask*>(it.get());

        UploadTask* topLevelTask = it->parentTask() ? it->parentTask() : it.get();
        auto topLevelFileTask = dynamic_cast<FileUploadTask*>(topLevelTask);
        it->setUploadManager(queueUploader_);
        it->setStatus(UploadTask::StatusRunning);
        mutex_.lock();
        serverThreads_[it->serverName()].waitingFileCount--;
        std::string initialServerName = it->serverName();
        UploadSession* session = it->session();
        //serverThreads_[serverName].runningThreads++;
        mutex_.unlock();

        bool res = true;
        for (size_t i = 0; i < filters_.size(); i++) {
            res = res && filters_[i]->PreUpload(it.get()); // ServerProfile can be changed in PreUpload filters
        }
        if (!res)
        {
            it->deletePostponedChilds(); // delete thumbnail
            it->finishTask(UploadTask::StatusFailure);
            continue;
        }

        /*if (it->schedulePostponedChilds()) {
            startFromSession_ = 0;
            //start();
        }*/

        std::string serverName = it->serverName();

        if (initialServerName != serverName) {
            if (fut) {
                fut->setFileName(fut->originalFileName());
            }
            it->setStatus(UploadTask::StatusInQueue);
            continue;
        }
        
        std::string  profileName = it->serverProfile().profileName();

        CAbstractUploadEngine *engine = uploadEngineManager_->getUploadEngine(it->serverProfile());

        if (!engine)
        {
            session->setFatalErrorForServer(serverName, profileName);
            ErrorInfo ei;
            ei.ServerName = serverName;
            ei.messageType = ErrorInfo::mtError;
            ei.sender = "Uploader";
            if (topLevelFileTask) {
                ei.TopLevelFileName = topLevelFileTask->getFileName();
            }
            ei.error = "Fatal error occured while uploading. Aborting upload to this server.";
            uploadErrorHandler_->ErrorMessage(ei);
            it->finishTask(UploadTask::StatusFailure);
            continue;
        }
        engine->serverSync()->incrementThreadCount();
        uploader.setUploadEngine(engine);
        uploader.onNeedStop.bind(this, &FileQueueUploaderPrivate::onNeedStopHandler);
        it->setStatusText(tr("Starting upload"));

        try {
            res = uploader.Upload(it);

            it->setUploadSuccess(res);
            if (!res && uploader.isFatalError()) {
                ErrorInfo ei;
                ei.ServerName = serverName;
                ei.messageType = ErrorInfo::mtError;
                ei.sender = "Uploader";
                if (topLevelFileTask) {
                    ei.TopLevelFileName = topLevelFileTask->getFileName();
                }
                ei.error = "Fatal error occured while uploading. Aborting upload to this server.";
                uploadErrorHandler_->ErrorMessage(ei);

                session->setFatalErrorForServer(serverName, profileName);
                //serverThreads_[serverName].fatalError = true;
            }

            serverThreadsMutex_.lock();

            serverThreads_[serverName].runningThreads--;

            serverThreadsMutex_.unlock();

            //callMutex_.lock();

            UploadResult* result = it->uploadResult();
            result->serverName = serverName;

            if (res) {
                for (size_t i = 0; i < filters_.size(); i++) {
                    filters_[i]->PostUpload(it.get());
                }
            } else {
            }
            UploadTask::Status st = res ? UploadTask::StatusFinished : UploadTask::StatusFailure;
            if (it->stopSignal()) {
                st = UploadTask::StatusStopped;
            }
            it->finishTask(st);

        } catch (NetworkClient::AbortedException &) {
            it->finishTask(UploadTask::StatusStopped);
        }
       
        engine->serverSync()->decrementThreadCount();
       
        //callMutex_.unlock();

    }
    mutex_.lock();
    //runningThreads_--;

    mutex_.unlock();
    uploadEngineManager_->clearThreadData();
    scriptsManager_->clearThreadData();
    
    /*if (!runningThreads_)
    {
        isRunning_ = false;
        if (queueUploader_->OnQueueFinished) {
            queueUploader_->OnQueueFinished(queueUploader_);
        }
        //LOG(ERROR) << "All threads terminated";
    }*/

}

