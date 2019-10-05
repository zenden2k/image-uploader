#ifndef IU_CORE_UPLOADMANAGER_H
#define IU_CORE_UPLOADMANAGER_H

#include "FileQueueUploader.h"

class CMyEngineList;
class ScriptsManager;
class BasicSettings;
class CUploadEngineList;
class UrlShorteningFilter;

class UploadManager : public CFileQueueUploader
{
public:
    UploadManager(std::shared_ptr<UploadEngineManager> uploadEngineManager, std::shared_ptr<CUploadEngineList> engineList, std::shared_ptr<ScriptsManager> scriptsManager, std::shared_ptr<IUploadErrorHandler> uploadErrorHandler,
        std::shared_ptr<INetworkClientFactory> networkClientFactory, int threadCount);
    ~UploadManager();
    bool shortenLinksInSession(std::shared_ptr<UploadSession> session, UrlShorteningFilter* filter);
protected:
    std::shared_ptr<UploadEngineManager> uploadEngineManager_;

    void sessionAdded(UploadSession* session) override;
    void onSessionFinished(UploadSession* session);
    void onTaskFinished(UploadTask* task, bool ok);
    void taskAdded(UploadTask* task) override;
    void settingsChanged(BasicSettings* settings);
};
#endif
