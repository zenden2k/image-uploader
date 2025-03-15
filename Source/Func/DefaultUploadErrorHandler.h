#ifndef IU_CORE_FUNC_DEFAULTUPLOADERRORHANDLER_H
#define IU_CORE_FUNC_DEFAULTUPLOADERRORHANDLER_H

#pragma once

#include "Core/Upload/UploadErrorHandler.h"
#include "Core/Logging/Logger.h"

class CUploadEngineListBase;

class DefaultUploadErrorHandler : public IUploadErrorHandler {
public:
    DefaultUploadErrorHandler(std::shared_ptr<ILogger> logger, CUploadEngineListBase* engineList);
    void ErrorMessage(const ErrorInfo& errorInfo) override;
    void DebugMessage(const std::string& msg, bool isResponseBody) override;
protected:
    std::shared_ptr<ILogger> logger_;
    int responseFileIndex_;
    CUploadEngineListBase* engineList_;
};
#endif
