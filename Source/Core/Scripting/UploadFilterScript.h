#pragma once

#include <string>
#include "Script.h"

class UploadTask;

class UploadFilterScript : public Script {
    public:
        UploadFilterScript(const std::string& fileName, ThreadSync* serverSync);
        bool preUpload(UploadTask* task);
        bool postUpload(UploadTask* task);
};
