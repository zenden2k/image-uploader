#ifndef CORE_UPLOAD_AUTHTASK_H
#define CORE_UPLOAD_AUTHTASK_H

#include "UploadTask.h"
#include <string>

enum class AuthActionType { Login, Logout};

class AuthTask: public UploadTask {
    public:
        AuthTask(AuthActionType type = AuthActionType::Login);
        ~AuthTask();
        virtual Type type() const override;
        std::string getMimeType() const override;
        int64_t getDataLength() const override;
        std::string toString() override;
        std::string title() const override;
        AuthActionType authActionType() const;
        int retryLimit() override;

protected:
    AuthActionType authActionType_;
};    

#endif