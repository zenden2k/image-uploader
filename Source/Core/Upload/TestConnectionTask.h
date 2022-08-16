#pragma once

#include <string>

#include "UploadTask.h"

class TestConnectionTask: public UploadTask {
    public:
        TestConnectionTask();
        Type type() const override;
        std::string getMimeType() const override;
        int64_t getDataLength() const override;
        std::string toString() override;
        std::string title() const override;
        int retryLimit() override;
};    
