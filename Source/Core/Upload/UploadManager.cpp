#include "UploadManager.h"

#include "Core/HistoryManager.h"
#include "Core/ServiceLocator.h"
#include "Core/LocalFileCache.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/Filters/UrlShorteningFilter.h"
#include "Core/Scripting/ScriptsManager.h"
#include "Core/Settings/BasicSettings.h"
#include "UploadEngineManager.h"

UploadManager::UploadManager(std::shared_ptr<UploadEngineManager> uploadEngineManager, 
    std::shared_ptr<CUploadEngineList> engineList, std::shared_ptr<ScriptsManager> scriptsManager,
    std::shared_ptr<IUploadErrorHandler> uploadErrorHandler, std::shared_ptr<INetworkClientFactory> networkClientFactory, int threadCount) :
CFileQueueUploader(uploadEngineManager, scriptsManager, uploadErrorHandler, networkClientFactory, threadCount)
{
    uploadEngineManager_ = uploadEngineManager;
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
    Settings.addChangeCallback(BasicSettings::ChangeCallback(this, &UploadManager::settingsChanged));
}

UploadManager::~UploadManager() {
    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    Settings->removeChangeCallback(BasicSettings::ChangeCallback(this, &UploadManager::settingsChanged));
}

bool UploadManager::shortenLinksInSession(std::shared_ptr<UploadSession> session, UrlShorteningFilter* filter)
{
    int taskCount = session->taskCount();

    bool res = false;
    for (int i = 0; i < taskCount; i++)
    {
        std::shared_ptr<UploadTask> task = session->getTask(i);
        if (task->shorteningStarted())
        {
            continue;
        }
        task->restartTask(false);
        task->serverProfile().setShortenLinks(true);
        res = filter->PostUpload(task.get()) || res;
    }
    session->recalcFinishedCount();
    return res;
}

void UploadManager::sessionAdded(UploadSession* session)
{
    CFileQueueUploader::sessionAdded(session);
    session->addSessionFinishedCallback(UploadSession::SessionFinishedCallback(this, &UploadManager::onSessionFinished));
}

void UploadManager::onSessionFinished(UploadSession* uploadSession)
{
    uploadEngineManager_->resetFailedAuthorization();
}

void UploadManager::onTaskFinished(UploadTask* task, bool ok)
{
    UploadSession* uploadSession = task->session();
    std::shared_ptr<CHistorySession> session;
    if (uploadSession->isHistoryEnabled()) {
        CHistoryManager * mgr = ServiceLocator::instance()->historyManager();
        std::lock_guard<std::mutex> lock(uploadSession->historySessionMutex_);
        session = uploadSession->historySession_;
        if (!session) {
            session = mgr->newSession();
            uploadSession->historySession_ = session;
        }
    }

    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (!fileTask)
    {
        return ;
    }

    TempFileDeleter* tempFileDeleter = fileTask->tempFileDeleter(false);
    if (tempFileDeleter)
    {
        tempFileDeleter->cleanup();
    }
    if (!fileTask->uploadSuccess() || fileTask->status() != UploadTask::StatusFinished)
    {
        return ;
    }
    
    if (uploadSession->isHistoryEnabled()) {
        HistoryItem hi(session.get());
        hi.localFilePath = fileTask->originalFileName();
        hi.serverName = fileTask->serverProfile().serverName();
        UploadResult* uploadResult = fileTask->uploadResult();
        hi.directUrl = uploadResult->directUrl;
        hi.directUrlShortened = uploadResult->directUrlShortened;
        hi.thumbUrl = uploadResult->thumbUrl;
        hi.viewUrl = uploadResult->downloadUrl;
        hi.viewUrlShortened = uploadResult->downloadUrlShortened;
        hi.editUrl = uploadResult->editUrl;
        hi.deleteUrl = uploadResult->deleteUrl;
        hi.displayName = fileTask->getDisplayName();
        hi.sortIndex = fileTask->index();
        hi.uploadFileSize = fileTask->getDataLength();
        if (!hi.directUrl.empty()) {
            LocalFileCache::instance()->addFile(hi.directUrl, hi.localFilePath);
        }

        CHistoryManager * mgr = ServiceLocator::instance()->historyManager();
        mgr->saveHistoryItem(&hi);
        //session->addItem(hi);
    }
}

void UploadManager::taskAdded(UploadTask* task)
{
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (!fileTask)
    {
        return;
    }

    if (task->parentTask())
    {
        return ;
    }
    task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &UploadManager::onTaskFinished));
}

void UploadManager::settingsChanged(BasicSettings* settings)
{
    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    setMaxThreadCount(Settings->MaxThreads);
}
