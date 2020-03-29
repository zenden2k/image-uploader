#include "AuthTask.h"

AuthTask::AuthTask(AuthActionType type) {
    authActionType_ = type;
}

UploadTask::Type AuthTask::type() const {
    return TypeAuth;
}

std::string AuthTask::getMimeType() const {
    return {};
}

int64_t AuthTask::getDataLength() const {
    return 0;
}

std::string AuthTask::toString() {
    return "Auth";
}

std::string AuthTask::title() const {
    return "Auth";
}

AuthActionType AuthTask::authActionType() const {
    return authActionType_;
}

int AuthTask::retryLimit() {
    return 1;
}
