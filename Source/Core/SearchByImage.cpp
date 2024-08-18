#include "SearchByImage.h"

#include "Core/Upload/SearchByImageFileTask.h"
#include "Core/Upload/TempImageUploadTask.h"
#include "Core/Upload/UploadSession.h"
#include "Core/Upload/UploadManager.h"
#include "Core/i18n/Translator.h"

std::shared_ptr<UploadSession> SearchByImage::search(const std::string& fileName, const ServerProfile& imageSearchServer, const ServerProfile& temporaryServer, UploadManager* uploadManager, std::function<void(const std::string&)> statusCallback)
{
    CUploadEngineData* ued = imageSearchServer.uploadEngineData();
    if (!ued->hasType(CUploadEngineData::TypeSearchByImageServer)) {
        return {};
    }
    auto session = std::make_shared<UploadSession>(false);
    std::string displayName = IuCoreUtils::ExtractFileName(fileName);
    if (ued->UploadToTempServer) { 
        auto uploadTask = std::make_shared<TempImageUploadTask>(fileName, displayName);
        uploadTask->setServerProfile(temporaryServer);
        uploadTask->setSearchServerProfile(imageSearchServer);
        session->addTask(uploadTask);
    } else {
        auto uploadTask = std::make_shared<SearchByImageFileTask>(fileName, displayName);
        uploadTask->setServerProfile(imageSearchServer);
        session->addTask(uploadTask);
    }

    return session;
}
