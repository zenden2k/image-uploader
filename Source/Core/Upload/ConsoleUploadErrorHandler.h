#ifndef IU_CORE_UPLOAD_CONSOLEUPLOADERRORHANDLER_H
#define IU_CORE_UPLOAD_CONSOLEUPLOADERRORHANDLER_H

#pragma once

#include "Core/Upload/UploadErrorHandler.h"

class ConsoleUploadErrorHandler : public IUploadErrorHandler {
public:
    ConsoleUploadErrorHandler();
    void ErrorMessage(const ErrorInfo& errorInfo) override;
    void DebugMessage(const std::string& msg, bool isResponseBody) override;
};

#endif
