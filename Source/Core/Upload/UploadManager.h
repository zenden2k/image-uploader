#ifndef IU_CORE_UPLOADMANAGER_H
#define IU_CORE_UPLOADMANAGER_H

#include "FileQueueUploader.h"
#ifdef IU_WTL
    #include "Filters/ImageConverterFilter.h"
    #include "Filters/SizeExceedFilter.h"
#endif
#include "Filters/UrlShorteningFilter.h"
#include "Filters/UserFilter.h"

class CMyEngineList;
class ScriptsManager;
class BasicSettings;
class CUploadEngineList;
class UploadManager : public CFileQueueUploader
{
public:
    UploadManager(UploadEngineManager* uploadEngineManager, CUploadEngineList* engineList, ScriptsManager* scriptsManager, IUploadErrorHandler* uploadErrorHandler);
    ~UploadManager();
    bool shortenLinksInSession(std::shared_ptr<UploadSession> session);
    void setEnableHistory(bool enable);
protected:
#ifdef IU_WTL_APP
    ImageConverterFilter imageConverterFilter;
    SizeExceedFilter sizeExceedFilter_;
#endif
    UrlShorteningFilter urlShorteningFilter;
    UserFilter userFilter;
    UploadEngineManager* uploadEngineManager_;
    bool enableHistory_;

    void configureNetwork(CFileQueueUploader* uploader, NetworkClient* networkClient);
    void sessionAdded(UploadSession* session) override;
    void onSessionFinished(UploadSession* session);
    void onTaskFinished(UploadTask* task, bool ok);
    void taskAdded(UploadTask* task) override;
    void settingsChanged(BasicSettings* settings);
};
#endif
