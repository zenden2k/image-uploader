#include "UploadManager.h"
#include <Func/Common.h>

UploadManager::UploadManager(UploadEngineManager* uploadEngineManager) : CFileQueueUploader(uploadEngineManager)
{
	addUploadFilter(&imageConverterFilter);
	addUploadFilter(&urlShorteningFilter);
	OnConfigureNetworkClient.bind(this, &UploadManager::configureNetwork);
}

void UploadManager::configureNetwork(CFileQueueUploader* uploader, NetworkClient* networkClient)
{
	IU_ConfigureProxy(*networkClient);
}