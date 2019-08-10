#ifndef IU_CORE_UPLOAD_UPLOADERRORHANDLER_H
#define IU_CORE_UPLOAD_UPLOADERRORHANDLER_H

#pragma once

#include "CommonTypes.h"

class IUploadErrorHandler {
public:
    virtual ~IUploadErrorHandler() = default;
    virtual void ErrorMessage(const ErrorInfo& errorInfo) = 0;
    virtual void DebugMessage(const std::string& msg, bool isResponseBody)=0;
};

#endif