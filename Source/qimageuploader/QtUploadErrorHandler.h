#ifndef IU_CORE_FUNC_DEFAULTUPLOADERRORHANDLER_H
#define IU_CORE_FUNC_DEFAULTUPLOADERRORHANDLER_H

#pragma once

#include "Core/Upload/UploadErrorHandler.h"
#include "Core/Logging/Logger.h"

class QtUploadErrorHandler : public IUploadErrorHandler {
public:
    QtUploadErrorHandler(ILogger* logger);
    virtual void ErrorMessage(const ErrorInfo& errorInfo) override;
    virtual void DebugMessage(const std::string& msg, bool isResponseBody) override;
protected:
    ILogger* logger_;
    int responseFileIndex_;

};
#endif