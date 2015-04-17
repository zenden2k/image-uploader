#ifndef IU_CORE_UPLOADMANAGER_H
#define IU_CORE_UPLOADMANAGER_H

#include "FileQueueUploader.h"
#include "Filters/ImageConverterFilter.h"
#include "Filters/UrlShorteningFilter.h"

class UploadManager : public CFileQueueUploader
{
public:
	UploadManager(UploadEngineManager* uploadEngineManager);
protected:
	ImageConverterFilter imageConverterFilter;
	UrlShorteningFilter urlShorteningFilter;
};
#endif