#ifndef IU_CORE_UPLOADMANAGER_H
#define IU_CORE_UPLOADMANAGER_H

#include "FileQueueUploader.h"
#include "Filters/ImageConverterFilter.h"
#include "Filters/UrlShorteningFilter.h"
#include "Filters/UserFilter.h"

class ScriptsManager;
class UploadManager : public CFileQueueUploader
{
public:
    UploadManager(UploadEngineManager* uploadEngineManager, ScriptsManager* scriptsManager);
    bool shortenLinksInSession(std::shared_ptr<UploadSession> session);
protected:
    ImageConverterFilter imageConverterFilter;
    UrlShorteningFilter urlShorteningFilter;
    UserFilter userFilter;

    void configureNetwork(CFileQueueUploader* uploader, NetworkClient* networkClient);
    void sessionAdded(UploadSession* session) override;
    void onSessionFinished(UploadSession* session);
    void onTaskFinished(UploadTask* task, bool ok);
    void taskAdded(UploadTask* task) override;
    void settingsChanged(CSettings* settings);

};
#endif