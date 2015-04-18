#ifndef IU_CORE_UPLOADMANAGER_H
#define IU_CORE_UPLOADMANAGER_H

#include "FileQueueUploader.h"
#include "Filters/ImageConverterFilter.h"
#include "Filters/UrlShorteningFilter.h"

class UploadManager : public CFileQueueUploader
{
public:
	UploadManager(UploadEngineManager* uploadEngineManager);
	bool shortenLinksInSession(std::shared_ptr<UploadSession> session);
protected:
	ImageConverterFilter imageConverterFilter;
	UrlShorteningFilter urlShorteningFilter;
	void configureNetwork(CFileQueueUploader* uploader, NetworkClient* networkClient);
	void sessionAdded(UploadSession* session) override;
	void onSessionFinished(UploadSession* session);
	void onTaskFinished(UploadTask* task, bool ok);
	void taskAdded(UploadTask* task) override;

};
#endif