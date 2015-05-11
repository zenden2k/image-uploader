#ifndef IU_CORE_UPLOADMANAGER_H
#define IU_CORE_UPLOADMANAGER_H

#include "FileQueueUploader.h"
#ifdef IU_WTL
    #include "Filters/ImageConverterFilter.h"
#endif
#include "Filters/UrlShorteningFilter.h"
#include "Filters/UserFilter.h"

class ScriptsManager;
class BasicSettings;
class UploadManager : public CFileQueueUploader
{
public:
    UploadManager(UploadEngineManager* uploadEngineManager, ScriptsManager* scriptsManager, IUploadErrorHandler* uploadErrorHandler);
    bool shortenLinksInSession(std::shared_ptr<UploadSession> session);
protected:
    #ifdef IU_WTL
    ImageConverterFilter imageConverterFilter;
    #endif
    UrlShorteningFilter urlShorteningFilter;
    UserFilter userFilter;
    UploadEngineManager* uploadEngineManager_;

    void configureNetwork(CFileQueueUploader* uploader, NetworkClient* networkClient);
    void sessionAdded(UploadSession* session) override;
    void onSessionFinished(UploadSession* session);
    void onTaskFinished(UploadTask* task, bool ok);
    void taskAdded(UploadTask* task) override;
    void settingsChanged(BasicSettings* settings);

};
#endif
