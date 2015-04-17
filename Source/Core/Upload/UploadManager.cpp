#include "UploadManager.h"

UploadManager::UploadManager(UploadEngineManager* uploadEngineManager) : CFileQueueUploader(uploadEngineManager)
{
	addUploadFilter(&imageConverterFilter);
	addUploadFilter(&urlShorteningFilter);
}