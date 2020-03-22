#ifndef IU_CORE_UPLOAD_SIZEEXCEEDFILTER_H
#define IU_CORE_UPLOAD_SIZEEXCEEDFILTER_H

#pragma once

#include <mutex>
#include <unordered_map>

#include "Core/Upload/UploadFilter.h"
#include "Core/Upload/UploadTask.h"

class UploadEngineManager;
class CUploadEngineList;

class SizeExceedFilter : public UploadFilter {
public:
    explicit SizeExceedFilter(CUploadEngineList* engineList, UploadEngineManager* uploadEngineManager);
    bool PreUpload(UploadTask* task) override;
    bool PostUpload(UploadTask* task) override;
protected:
    CUploadEngineList* engineList_;
    UploadEngineManager* uploadEngineManager_;
    struct UploadSessionData {
        ServerProfile newImageServer, newFileServer;
        bool callbackAdded = false;
    };
    std::mutex uploadSessionDataMapMutex_;
    std::unordered_map<UploadSession*, UploadSessionData> uploadSessionDataMap_;
    void onSessionFinished(UploadSession* session);
};

#endif